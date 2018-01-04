
#include "producer_objects.hpp" 
#include "producer_object.hpp"
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>

#include <libdevcore/Exceptions.h>
#include <libdevcore/Assertions.h>
#include "global_property_object.hpp"


namespace native {
namespace eos {
using namespace chain;
using namespace types;

void ProducerVotesObject::updateVotes(ShareType deltaVotes, UInt128 currentRaceTime) {
   auto timeSinceLastUpdate = currentRaceTime - race.positionUpdateTime;
   auto newPosition = race.position + race.speed * timeSinceLastUpdate;
   auto newSpeed = race.speed + deltaVotes;

   race.update(newSpeed, newPosition, currentRaceTime);
}


ProducerRound ProducerScheduleObject::calculateNextRound(chainbase::database& db) const {

   ProducerRound round; 

   round.reserve(config::TotalProducersPerRound);

   std::map<AccountName,AccountName> processedProducers;

   int iActive = 0; 

   const auto& allProducersByVotes = db.get_index<ProducerVotesMultiIndex, byVotes>();

   if (allProducersByVotes.size() == 0 )
   {//需要至少一个Producer
	   BOOST_THROW_EXCEPTION(dev::NoEnoughProducers()
		   << dev::errinfo_comment("Active Producers Size = 0 !!!!"));
   }

    
   //挑选出票数最高的19人
   for (
	   auto iProducer = allProducersByVotes.begin();
	   iProducer != allProducersByVotes.end() && iActive < config::DPOSVotedProducersPerRound ; 
	   iProducer++ , iActive++ )
   { 
	   round.push_back(iProducer->ownerName);
	   processedProducers.insert(std::make_pair(iProducer->ownerName,iProducer->ownerName));
   }

   ctrace << "Top Voted Producers Count = " << iActive;

   //挑选出1个虚拟赛跑选手
   const auto& allProducersByFinishTime = db.get_index<ProducerVotesMultiIndex, byProjectedRaceFinishTime>(); 
   int runerCount = 0;
   for (
	   auto iProducer = allProducersByFinishTime.begin();
	   iProducer != allProducersByFinishTime.end() && runerCount < config::DPOSRunnerupProducersPerRound;
	   iProducer++ )
   {
	   if(processedProducers.count(iProducer->ownerName))
		   continue;

	   round.push_back(iProducer->ownerName); 
	   processedProducers.insert(std::make_pair(iProducer->ownerName, iProducer->ownerName));
	   runerCount++;
	   iActive++;
   }  

   AccountName lastRunnerUpName;
   if (runerCount > 0)
   {
	   lastRunnerUpName = round[iActive - 1];
   }

   ctrace << "Runerup Producers Count = " << runerCount;
    

   //选出1个POW见证人  
   const auto& allProducerObjsByPOW = db.get_index<producer_multi_index, by_pow>();

   const auto& gprops = db.get<dynamic_global_property_object>();

   int powCount = 0;
   for (
	   auto iProducer = allProducerObjsByPOW.upper_bound(0);
	   iProducer != allProducerObjsByPOW.end() && powCount <  config::POWProducersPerRound;
	   iProducer++ )
   {
	   if (processedProducers.count(iProducer->owner))
		   continue; 

	   //剔除本轮被选中的POW见证人
	   db.modify(*iProducer, [&](producer_object& prod)
	   {
		   prod.pow_worker = 0;
	   });

	   //当前pow见证人-1
	   db.modify(gprops, [&](dynamic_global_property_object& obj)
	   {
		   obj.num_pow_witnesses--;
	   });

	   //只有非DPOS注册见证人才有权POW出块
	   if (db.find<ProducerVotesObject, byOwnerName>(iProducer->owner) == NULL)
	   {
		   round.push_back(iProducer->owner);
		   processedProducers.insert(std::make_pair(iProducer->owner, iProducer->owner));
		   powCount++;
		   iActive++;
	   } 
   }

   //其它空位填空直到21位
   while (iActive < config::TotalProducersPerRound)
   {
	   round.push_back(AccountName());
	   iActive++;
   }

   ctrace << "POW Producers Count = " << powCount;
   ctrace << "Total Producers Count = " << processedProducers.size();

   //没有runer up Producer
   if (runerCount == 0)
	   return round;

   //更新虚拟赛跑

   // Machinery to update the virtual race tracking for the producers that completed their lap
   auto lastRunnerUp = allProducersByFinishTime.iterator_to(db.get<ProducerVotesObject,byOwnerName>(lastRunnerUpName));
   auto newRaceTime = lastRunnerUp->projectedRaceFinishTime();
   auto StartNewLap = [&db, newRaceTime](const ProducerVotesObject& pvo) {
      db.modify(pvo, [newRaceTime](ProducerVotesObject& pvo) {
         pvo.startNewRaceLap(newRaceTime);
      });
   };
   auto LapCompleters = boost::make_iterator_range(allProducersByFinishTime.begin(), ++lastRunnerUp);

   // Start each producer that finished his lap on the next one, and update the global race time.
   try {
	   if ((unsigned)boost::distance(LapCompleters) < allProducersByFinishTime.size()
		   && newRaceTime < std::numeric_limits<UInt128>::max()) { 

		   // 下面这样写会在访问LapCompleters的过程中修改container，导致使用LapCompleters访问下一个元素时发生错误
		   //boost::for_each(LapCompleters, StartNewLap);

		   std::vector<ProducerVotesObject> AllLapCompleters;
		   for (auto &a : LapCompleters)
		   {
			   AllLapCompleters.push_back(a);
		   }
		   boost::for_each(AllLapCompleters, StartNewLap);

		   db.modify(*this, [newRaceTime](ProducerScheduleObject& pso) {
			   pso.currentRaceTime = newRaceTime;
		   });

	   }
	   else {
		   //wlog("Producer race finished; restarting race.");
		   resetProducerRace(db);
	   } 
	 } catch (...) {
      // Virtual race time has overflown. Reset race for everyone. 
      resetProducerRace(db);
   }

   return round;
}

void ProducerScheduleObject::resetProducerRace(chainbase::database& db) const {
   auto ResetRace = [&db](const ProducerVotesObject& pvo) {
      db.modify(pvo, [](ProducerVotesObject& pvo) {
         pvo.startNewRaceLap(0);
      });
   };
   const auto& AllProducers = db.get_index<ProducerVotesMultiIndex, byVotes>();

   boost::for_each(AllProducers, ResetRace);
   db.modify(*this, [](ProducerScheduleObject& pso) {
      pso.currentRaceTime = 0;
   });
}

} } // namespace native::eos
