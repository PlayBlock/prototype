#include "chain_controller.hpp"
#include "rand.hpp"
//#include "BlockChain.h"
#include <iostream>
//#include "../libevm/Vote.h"
#include "producer_objects.hpp"
#include "version.hpp"
#include "boost/container/flat_map.hpp"
#include <tuple>

using namespace dev;
using namespace dev::eth;
using namespace dev::eth::chain;
using namespace eos::chain;
using native::eos::ProducerVotesMultiIndex;
using native::eos::ProducerScheduleMultiIndex;
using native::eos::ProducerVotesObject;

chain_controller::chain_controller(const dev::eth::BlockChain& bc, chainbase::database& db): _db(db), _bc(bc){

	initialize_indexes();
	initialize_chain(bc);

	//starter.register_types(*this, _db);

	//// Behave as though we are applying a block during chain initialization (it's the genesis block!)
	//with_applying_block([&] {
	//	initialize_chain(starter);
	//});

	//spinup_db();
	//spinup_fork_db();

	//if (_block_log.read_head() && head_block_num() < _block_log.read_head()->block_num())
	//	replay();
}

chain_controller::~chain_controller() {
	_db.flush();
}

void chain_controller::initialize_indexes() {
	//_db.add_index<account_index>();
	//_db.add_index<permission_index>();
	//_db.add_index<permission_link_index>();
	//_db.add_index<action_permission_index>();
	//_db.add_index<key_value_index>();
	//_db.add_index<key128x128_value_index>();
	//_db.add_index<key64x64x64_value_index>();

	_db.add_index<global_property_multi_index>();
	_db.add_index<dynamic_global_property_multi_index>();
	_db.add_index<ProducerVotesMultiIndex>();
	_db.add_index<ProducerScheduleMultiIndex>();
	_db.add_index<producer_multi_index>();
	_db.add_index<process_hardfork_multi_index>();

	//_db.add_index<block_summary_multi_index>();
	//_db.add_index<transaction_multi_index>();
	//_db.add_index<generated_transaction_multi_index>();
}

void chain_controller::initialize_chain(const dev::eth::BlockChain& bc)
{
	try {
		if (!_db.find<global_property_object>()) 
		{
			_db.with_write_lock([&] {

				//auto initial_timestamp = fc::time_point_sec(bc.genesis().timestamp().convert_to<uint32_t>());
				/// TODO: 使用创世块时间戳初始化
				auto initial_timestamp = fc::time_point_sec(1513580324);

				// Create global properties
				_db.create<global_property_object>([&](global_property_object& p) { 
					
					for (int i = 0; i < bc.chainParams().initialProducers.size(); i++)
					{
						p.active_producers[i] = bc.chainParams().initialProducers[i];
					} 
					 
					p.last_hardfork = 0; 
					p.current_hardfork_version = (eth::chain::hardfork_version(eth::chain::version(0, 0, 0)));
					p.next_hardfork = (eth::chain::hardfork_version(eth::chain::version(0, 0, 0)));
					p.next_hardfork_time = config::ETI_GenesisTime;   

				});

				_db.create<process_hardfork_object>([&](process_hardfork_object& pho) {
					pho.hardfork_timepoint = config::ETI_GenesisTime;
				});

				_db.create<dynamic_global_property_object>([&](dynamic_global_property_object& p) {
					p.time = initial_timestamp;
					p.recent_slots_filled = uint64_t(-1);
				});

				// Create the singleton object, ProducerScheduleObject
				_db.create<native::eos::ProducerScheduleObject>([](const auto&) {});

				for (const auto& a : _db.get<global_property_object>().active_producers)
				{
					if (a == AccountName())
						continue;

					ctrace << "genesis active producer:" << a;
					_db.create<producer_object>([&](producer_object& p) {
						p.owner = a;
					});

					_db.create<ProducerVotesObject>([&](ProducerVotesObject& p) {
						p.ownerName = a;
					});
				}

				//// Initialize block summary index
				//for (int i = 0; i < 0x10000; i++)
				//	_db.create<block_summary_object>([&](block_summary_object&) {});

				//auto messages = starter.prepare_database(*this, _db);
				//std::for_each(messages.begin(), messages.end(), [&](const Message& m) {
				//	MessageOutput output;
				//	ProcessedTransaction trx; /// dummy tranaction required for scope validation
				//	std::sort(trx.scope.begin(), trx.scope.end());
				//	with_skip_flags(skip_scope_check | skip_transaction_signatures | skip_authority_check, [&]() {
				//		process_message(trx, m.code, m, output);
				//	});
				//});
			});
		}

		//初始化hardforks相关全局变量
		init_hardforks();

	}
	catch (UnknownHardfork& e) {//若当前不在主分叉上
		ctrace << "YOUR CLIENT'S VERSION IS TOO OLD!!!!!!";
		while(1){}
	}
	catch (...) {
		cdebug << "initialize_chain error";
		throw;
	}
}

fc::time_point_sec chain_controller::get_slot_time(uint32_t slot_num)const
{
	if (slot_num == 0)
		return fc::time_point_sec();

	auto interval = block_interval();
	const dynamic_global_property_object& dpo = get_dynamic_global_properties();

	if (head_block_num() == 0)
	{
		// n.b. first block is at genesis_time plus one block interval
		fc::time_point_sec genesis_time = dpo.time;
		return genesis_time + slot_num * interval;
	}

	int64_t head_block_abs_slot = head_block_time().sec_since_epoch() / interval;
	fc::time_point_sec head_slot_time(head_block_abs_slot * interval);

	return head_slot_time + (slot_num * interval);
}

const global_property_object& chain_controller::get_global_properties()const {
	return _db.get<global_property_object>();
}

const dynamic_global_property_object&chain_controller::get_dynamic_global_properties() const {
	return _db.get<dynamic_global_property_object>();
}

fc::time_point_sec chain_controller::head_block_time()const {
	return get_dynamic_global_properties().time;
}

uint32_t chain_controller::head_block_num()const {
	return get_dynamic_global_properties().head_block_number;
}

uint32_t chain_controller::get_slot_at_time(fc::time_point_sec when)const
{
	fc::time_point_sec first_slot_time = get_slot_time(1);
	if (when < first_slot_time)
		return 0;
	return (when - first_slot_time).to_seconds() / block_interval() + 1;
}

types::AccountName chain_controller::get_scheduled_producer(uint32_t slot_num)const
{
	const dynamic_global_property_object& dpo = get_dynamic_global_properties();
	uint64_t current_aslot = dpo.current_absolute_slot + slot_num;
	const auto& gpo = _db.get<global_property_object>();
	return gpo.active_producers[current_aslot % gpo.active_producers.size()];
}

const producer_object& chain_controller::get_producer(const types::AccountName& ownerName) const {
	return _db.get<producer_object, by_owner>(ownerName);
}

ProducerRound chain_controller::calculate_next_round(const BlockHeader& next_block) {
	auto schedule = native::eos::ProducerScheduleObject::get(_db).calculateNextRound(_db);

	utilities::rand::random rng(next_block.timestamp().convert_to<uint64_t>());
	rng.shuffle(schedule);
	return schedule;
}


void chain_controller::update_global_properties(const BlockHeader& b) {
	// If we're at the end of a round, update the BlockchainConfiguration, producer schedule
	// and "producers" special account authority
	ctrace << "============>BlockNum = "<<b.number().convert_to<uint32_t>();
	if (b.number().convert_to<uint32_t>() % config::TotalProducersPerRound == 0) {
		try {
			auto schedule = calculate_next_round(b);
			//auto config = _admin->get_blockchain_configuration(_db, schedule);

			const auto& gpo = get_global_properties();

			//更新硬分叉选票
			update_hardfork_votes(gpo.active_producers);

			_db.modify(gpo, [schedule](global_property_object& gpo) {
				for (int i = 0; i < schedule.size(); i++)
				{
					gpo.active_producers[i] = schedule[i];
				}
				
				//gpo.configuration = std::move(config);
			});
		}
		catch (NoEnoughProducers const& e) {
			cdebug << e.what();
			return;
		}
		catch (...) {
			cdebug << "calculate_next_round error";
			return;
		}

		//auto active_producers_authority = types::Authority(config::ProducersAuthorityThreshold, {}, {});
		//for (auto& name : gpo.active_producers) {
		//	active_producers_authority.accounts.push_back({ { name, config::ActiveName }, 1 });
		//}

		//auto& po = _db.get<permission_object, by_owner>(boost::make_tuple(config::ProducersAccountName, config::ActiveName));
		//_db.modify(po, [active_producers_authority](permission_object& po) {
		//	po.auth = active_producers_authority;
		//});
	}
}

//std::map<Address, VoteBace> chain_controller::get_votes(h256 const& _hash) const
//{
//	if (_stateDB == nullptr)
//		return std::map<Address, VoteBace>();
//
//	Block block(_bc, *_stateDB);
//	block.populateFromChain(_bc, _hash == h256() ? _bc.currentHash() : _hash);
//
//
//	std::unordered_map<u256, u256> voteMap = block.storage(Address("0000000000000000000000000000000000000005"));
//	std::unordered_map<u256, u256> voteMapChange;
//	std::map<Address, VoteBace> returnMap;
//	dev::bytes bytes0(12, '\0');
//	for (auto iterator = voteMap.begin(); iterator != voteMap.end(); iterator++)
//	{
//		dev::u256 iteratorFirst = iterator->first;
//		dev::bytes iteratorBytes = ((dev::h256)iteratorFirst).asBytes();
//		dev::bytes iteratorAddress(iteratorBytes.begin(), iteratorBytes.begin() + sizeof(dev::Address));
//		dev::bytes iteratorExpand(iteratorBytes.begin() + sizeof(dev::Address), iteratorBytes.end());
//
//		if (iteratorExpand == bytes0)
//		{
//			Vote reset(voteMap, voteMapChange, dev::Address(iteratorAddress));
//			reset.load();
//			VoteBace vb;
//			vb.m_address = dev::Address(iteratorAddress);
//			vb.m_assignNumber = reset.getAssignNumber();
//			vb.m_isCandidate = reset.getIsCandidate();
//			vb.m_isVoted = reset.getIsVoted();
//			vb.m_receivedVoteNumber = reset.getReceivedVoteNumber();
//			vb.m_unAssignNumber = reset.getUnAssignNumber();
//			vb.m_votedNumber = reset.getVotedNumber();
//			vb.m_voteTo = reset.getVoteTo();
//			returnMap[dev::Address(iteratorAddress)] = vb;
//		}
//	}
//	return returnMap;
//}

using native::eos::ProducerScheduleObject;
using native::eos::ProducerVotesObject;

void chain_controller::update_pvomi_perblock()
{
//	//std::map<Address, VoteBace> AllProducers = get_votes();
//	std::map<Address, uint32_t> AllProducers;
//	std::unordered_set<Address> InactiveProducers;
//
//	// check every producer in AllProducers
//	InactiveProducers.clear();
//	for (const auto& p : AllProducers)
//	{
//		auto it = _all_votes.find(p.first);
//		if (it != _all_votes.end())
//		{
//			dev::types::Int64 deltaVotes = p.second.m_votedNumber - it->second;
//			if (deltaVotes == 0)
//				continue;
//
//			auto raceTime = ProducerScheduleObject::get(_db).currentRaceTime;
//			_db.modify<ProducerVotesObject>(_db.get<ProducerVotesObject, native::eos::byOwnerName>(p.first), [&](ProducerVotesObject& pvo) {
//				pvo.updateVotes(deltaVotes, raceTime);
//			});
//			it->second = p.second.m_votedNumber;
//		}
//		else
//		{
//			auto raceTime = ProducerScheduleObject::get(_db).currentRaceTime;
//			_db.create<ProducerVotesObject>([&](ProducerVotesObject& pvo) {
//				pvo.ownerName = p.first;
//				pvo.startNewRaceLap(raceTime);
//			});
//
//			_db.create<producer_object>([&](producer_object& po) {
//				po.owner = p.first;
//				//po.signing_key = update.key;
//				//po.configuration = update.configuration;
//			});
//			
//			_all_votes.insert(std::make_pair(p.first, p.second.m_votedNumber));
//		}
//	}
//
//	// remove all inactive producers
//	for (const auto& p : _all_votes)
//	{
//		if (AllProducers.find(p.first) == AllProducers.end())
//		{
//			InactiveProducers.insert(p.first);
//		}
//	}
//
//	for (auto& p : InactiveProducers)
//	{
//		_db.remove<ProducerVotesObject>(_db.get<ProducerVotesObject, native::eos::byOwnerName>(p));
//		_db.remove<producer_object>(_db.get<producer_object, eos::chain::by_owner>(p));
//		_all_votes.erase(p);
//	}
//
//	ctrace << "ProducerVotesMultiIndex: ";
//	for ( auto& a : _db.get_index<ProducerVotesMultiIndex, native::eos::byProjectedRaceFinishTime>())
//	{
//		ctrace << "name: " << a.ownerName;
//		ctrace << "votes: " << a.getVotes();
//		ctrace << "finish time: " << a.projectedRaceFinishTime();
//	}
}

 

void dev::eth::chain::chain_controller::update_pow()
{
	auto last_block_id = _bc.currentHash();

	dev::h256 target = get_pow_target();

	auto& dgp = _db.get<dynamic_global_property_object>();

	for (auto i : _temp_pow_ops)
	{
		POW_Operation& opref = i.second;

		if (opref.block_id != last_block_id) { //若pow结果不是上一个块的略过
			continue;
		}

		if(opref.work >= target ) continue;


		const producer_object* pow_producer = _db.find<producer_object, by_owner>(opref.getAddress());

		if (pow_producer != nullptr)
		{
			if (pow_producer->pow_worker != 0)
			{//尚在队列中，略过
				continue;
			}
		}

		_db.modify(dgp, [&](dynamic_global_property_object& p) {
			p.total_pow++;
			p.num_pow_witnesses++;
		});



		if (pow_producer == nullptr)
		{//此Produer尚未存在，创建
			_db.create<producer_object>([&](producer_object& po) {
				po.owner = opref.getAddress(); 
				po.pow_worker = dgp.total_pow;
			});
		}
		else {//修改pow次序
			_db.modify(*pow_producer, [&](producer_object& po) {
				po.pow_worker = dgp.total_pow; 
			});
		}


		//const auto& witnesses_by_name = _db.get_index_type< producer_object >().indices().get<by_owner>();
	}
}

dev::h256 dev::eth::chain::chain_controller::get_pow_target()
{  
	const auto& dgp = _db.get<dynamic_global_property_object>(); 
	dev::u256 target;
	target = -1;
	target >>= ((dgp.num_pow_witnesses / 4) + 4); 
	return dev::h256(target);
}

void dev::eth::chain::chain_controller::process_block_header(const BlockHeader& b)
{
	const types::AccountName& producer = b.producer();
	const producer_object& producer_obj = get_producer(producer);

	if (b.runningVersion() != producer_obj.running_version)
	{
		_db.modify(producer_obj, [&](producer_object& po)
		{
			po.running_version = b.runningVersion();
		});
	} 

	if (b.hardforkVote().hf_version != producer_obj.hardfork_ver_vote ||
		b.hardforkVote().hf_time != producer_obj.hardfork_time_vote)
	{//更新producer的hardfork选票

		_db.modify(producer_obj, [&](producer_object& po) {
			po.hardfork_ver_vote = b.hardforkVote().hf_version;
			po.hardfork_time_vote = b.hardforkVote().hf_time;
		});
	} 
	const global_property_object& gpo = get_global_properties();

	if (producer_obj.running_version < gpo.current_hardfork_version)
	{
		ctrace << "Block produced by witness that is not running current hardfork";
		BOOST_THROW_EXCEPTION(InvlidRuningVersion());
	}
}



void dev::eth::chain::chain_controller::process_hardforks()
{
	const eos::chain::global_property_object& gpo = get_global_properties();

	while (
		_hardfork_versions[gpo.last_hardfork] < gpo.next_hardfork && 
		gpo.next_hardfork_time <= head_block_time())
	{
		if (gpo.last_hardfork < config::ETI_HardforkNum) {
			apply_hardfork(gpo.last_hardfork + 1);
		}else{
			ctrace<<"Unknown Hardfork!!!!!";
			BOOST_THROW_EXCEPTION(UnknownHardfork());
		}
	}
}

void dev::eth::chain::chain_controller::apply_hardfork(uint32_t hardfork)
{
	const eos::chain::global_property_object& gpo = get_global_properties();

	if (hardfork != gpo.last_hardfork + 1)
	{
		ctrace << "Hardfork being applied out of order" << " hardfork = " << hardfork << " gpo.last_hardfork = " << gpo.last_hardfork;
		BOOST_THROW_EXCEPTION(HardforkApplyOutOfOrder());
	}

	_db.create<eos::chain::process_hardfork_object>([&](eos::chain::process_hardfork_object& pho) {
		pho.hardfork_timepoint = _hardfork_times[hardfork];
	});
	

	_db.modify(gpo, [&](global_property_object& gpo)
	{ 
		gpo.last_hardfork = hardfork;
		gpo.current_hardfork_version = _hardfork_versions[hardfork]; 
	});

	//切换硬分叉成功！！
	ctrace << "Switch Hardfork " << _hardfork_versions[hardfork].v_num << " Succeed!!!!! ";
}

void dev::eth::chain::chain_controller::update_hardfork_votes(const std::array<AccountName, TotalProducersPerRound>& active_producers)
{
	const eos::chain::global_property_object& gpo = get_global_properties();
	
	boost::container::flat_map< version, uint32_t, std::greater< version > > producer_versions;
	boost::container::flat_map< std::tuple< hardfork_version, time_point_sec >, uint32_t ,
		std::greater<std::tuple< hardfork_version, time_point_sec >>> hardfork_version_votes;

	//统计硬分叉投票
	for (uint32_t i = 0; i < active_producers.size(); i++)
	{ 

		//略过空位
		if (active_producers[i] == AccountName())
			continue;

		auto producer_obj = get_producer(active_producers[i]);


		if (producer_versions.find(producer_obj.running_version) == producer_versions.end())
			producer_versions[producer_obj.running_version] = 1;
		else
			producer_versions[producer_obj.running_version] += 1;

		auto version_vote = std::make_tuple(producer_obj.hardfork_ver_vote, producer_obj.hardfork_time_vote);
		if (hardfork_version_votes.find(version_vote) == hardfork_version_votes.end())
			hardfork_version_votes[version_vote] = 1;
		else
			hardfork_version_votes[version_vote] += 1;
	}

	{//打印hardfork_version 选票结果
		auto hf_itr = hardfork_version_votes.begin(); 
		while (hf_itr != hardfork_version_votes.end())
		{
			auto hf_ver = std::get<0>(hf_itr->first);
			auto hf_time = std::get<1>(hf_itr->first);
			std::cout << "hardfork " << hf_ver.v_num << ": votes = " << hf_itr->second << " time = "<< hf_time.to_iso_string() <<std::endl;
			hf_itr++;
		}
	}
 
	auto hf_itr = hardfork_version_votes.begin();
	 
	while (hf_itr != hardfork_version_votes.end())
	{
		if (hf_itr->second >= config::ETI_HardforkRequiredProducers)
		{
			auto hf_ver = std::get<0>(hf_itr->first);
			auto hf_time = std::get<1>(hf_itr->first);
			_db.modify(gpo, [&](global_property_object& gpo)
			{
				gpo.next_hardfork.v_num = hf_ver.v_num;
				gpo.next_hardfork_time = hf_time;
			});

			break;
		}

		hf_itr++;
	}

	// We no longer have a majority
	if (hf_itr == hardfork_version_votes.end())
	{
		_db.modify(gpo, [&](global_property_object& gpo)
		{
			gpo.next_hardfork = gpo.current_hardfork_version;
		});
	}
}

void dev::eth::chain::chain_controller::update_pow_perblock(const BlockHeader& b)
{  
	//按照当前header构造块
	Block newBlock(_bc, *_stateDB);
	newBlock.populateFromChain(_bc, b.hash());

	_temp_pow_ops.clear();

	std::map<h256, std::pair<u256, u256>> powMap = newBlock.storage(POWInfoAddress); 
	std::map<dev::h160,dev::h160> addressTab;

	//收集所有的POW Address
	for (auto itpow : powMap)
	{ 
		dev::bytes buff = dev::h256(itpow.second.first).asBytes();
		dev::bytes addrBuff; 
		for (int i = 0; i < 20; i++)
		{
			addrBuff.push_back(buff[i]);
		}

		dev::h160 powAddr(addrBuff);
		if (!addressTab.count(powAddr)) {
			addressTab.insert(std::make_pair(powAddr, powAddr)); 
		}
	}

	//将所有OP放到临时列表
	for (auto itaddr : addressTab)
	{
		std::unordered_map<dev::u256, dev::u256> mapChange; 
		POW_Operation powop(powMap, mapChange, itaddr.first);
		powop.load(); 
		_temp_pow_ops.insert(std::make_pair(itaddr.first, powop));
	} 
	 
}


void chain_controller::update_global_dynamic_data(const BlockHeader& b) {
	const dynamic_global_property_object& _dgp = _db.get<dynamic_global_property_object>();

	uint32_t missed_blocks = head_block_num() == 0 ? 1 : get_slot_at_time(fc::time_point_sec(b.timestamp().convert_to<uint32_t>()));
	assert(missed_blocks != 0);
	missed_blocks--;

	//   if (missed_blocks)
	//      wlog("Blockchain continuing after gap of ${b} missed blocks", ("b", missed_blocks));

	for (uint32_t i = 0; i < missed_blocks; ++i) {
		AccountName missed_producer_name = get_scheduled_producer(i + 1);

		if (missed_producer_name != AccountName())
		{//略过空Producer
			const auto& producer_missed = get_producer(missed_producer_name);
			if (producer_missed.owner != b.producer()) {
				/*
				const auto& producer_account = producer_missed.producer_account(*this);
				if( (fc::time_point::now() - b.timestamp) < fc::seconds(30) )
				wlog( "Producer ${name} missed block ${n} around ${t}", ("name",producer_account.name)("n",b.block_num())("t",b.timestamp) );
				*/

				_db.modify(producer_missed, [&](producer_object& w) {
					w.total_missed++;
				});
			}
		}
	}

	// dynamic global properties updating
	_db.modify(_dgp, [&](dynamic_global_property_object& dgp) {
		dgp.head_block_number = b.number().convert_to<uint32_t>();
		dgp.head_block_id = b.hash();
		dgp.time = fc::time_point_sec(b.timestamp().convert_to<uint32_t>());
		//dgp.current_producer = b.producer();
		dgp.current_absolute_slot += missed_blocks + 1;

		// If we've missed more blocks than the bitmap stores, skip calculations and simply reset the bitmap
		if (missed_blocks < sizeof(dgp.recent_slots_filled) * 8) {
			dgp.recent_slots_filled <<= 1;
			dgp.recent_slots_filled += 1;
			dgp.recent_slots_filled <<= missed_blocks;
		}
		else
			dgp.recent_slots_filled = 0;
	});

}

void chain_controller::update_signing_producer(const producer_object& signing_producer, const BlockHeader& new_block)
{
	const dynamic_global_property_object& dpo = get_dynamic_global_properties();
	uint64_t new_block_aslot = dpo.current_absolute_slot + get_slot_at_time(fc::time_point_sec(new_block.timestamp().convert_to<uint32_t>()));

	_db.modify(signing_producer, [&](producer_object& _wit)
	{
		_wit.last_aslot = new_block_aslot;
		_wit.last_confirmed_block_num = new_block.number().convert_to<uint32_t>();
	});
}

void chain_controller::update_last_irreversible_block()
{
	const global_property_object& gpo = get_global_properties();
	const dynamic_global_property_object& dpo = get_dynamic_global_properties();

	std::vector<const producer_object*> producer_objs; 

	auto it_prod = gpo.active_producers.begin();
	while (it_prod != gpo.active_producers.end())
	{
		if (*it_prod != AccountName())
		{//若不为空账户，则将其加入producer列表
			producer_objs.push_back(&get_producer(*it_prod));
		}
		it_prod++;
	}

	static_assert(config::IrreversibleThresholdPercent > 0, "irreversible threshold must be nonzero");

	size_t offset = dev::eth::ETH_PERCENT(producer_objs.size(), config::Percent100 - config::IrreversibleThresholdPercent);
	std::nth_element(producer_objs.begin(), producer_objs.begin() + offset, producer_objs.end(),
		[](const producer_object* a, const producer_object* b) {
		return a->last_confirmed_block_num < b->last_confirmed_block_num;
	});

	uint32_t new_last_irreversible_block_num = producer_objs[offset]->last_confirmed_block_num;

	if (new_last_irreversible_block_num > dpo.last_irreversible_block_num) {
		_db.modify(dpo, [&](dynamic_global_property_object& _dpo) {
			_dpo.last_irreversible_block_num = new_last_irreversible_block_num;
		});
	}

	// Trim fork_database and undo histories
	//_fork_db.set_max_size(head_block_num() - new_last_irreversible_block_num + 1);
	_db.commit(new_last_irreversible_block_num);
}

void chain_controller::databaseReversion(uint32_t _firstvalid)
{
	while (head_block_num() != _firstvalid)
	{
		_db.undo();
	}
}

void chain_controller::push_block(const BlockHeader& b)
{
	try {
		auto session = _db.start_undo_session(true);
		apply_block(b);
		session.push();
	}
	catch (...)
	{
		cwarn << "apply block error: " << b.number();
		throw;
	}

}

void chain_controller::init_allvotes(const BlockHeader& bh)
{
	_all_votes.clear();

	if (bh.number() <= 0)
		return;

	std::map<Address, VoteInfo> AllProducers = get_votes(bh.parentHash());
	for (const auto& p : AllProducers)
	{
		_all_votes.insert(std::make_pair(p.first, p.second.getReceivedVotedNumber()));
	}
}

void dev::eth::chain::chain_controller::init_hardforks()
{
	_hardfork_times[0] = fc::time_point_sec(config::ETI_GenesisTime);
	_hardfork_versions[0] = hardfork_version(0, 0); 
	//_hardfork_times[1] = fc::time_point_sec(1);
	//_hardfork_versions[1] = hardfork_version(0, 1);

	const auto& gpo = get_global_properties();

	if (gpo.last_hardfork > config::ETI_HardforkNum)
	{
		ctrace << "Chain knows of more hardforks than configuration";
		BOOST_THROW_EXCEPTION(UnknownHardfork());
	}

	if (_hardfork_versions[gpo.last_hardfork] > config::ETI_BlockchainVersion)
	{
		ctrace << "Blockchain version is older than last applied hardfork";
		BOOST_THROW_EXCEPTION(UnknownHardfork());
	}
}



void chain_controller::apply_block(const BlockHeader& b)
{
	const producer_object& signing_producer = validate_block_header(b);

	process_block_header(b);

	update_pow_perblock(b);

	update_pow();
	update_pvomi_perblock(); 
	update_global_dynamic_data(b);
	update_global_properties(b);
	update_signing_producer(signing_producer, b);
	update_last_irreversible_block();

	process_hardforks();

	std::cout <<"currentHash: "<< _bc.currentHash().hex() << std::endl;
}

const producer_object& chain_controller::validate_block_header(const BlockHeader& bh)const
{
	if (!bh.verifySign())
		BOOST_THROW_EXCEPTION(InvalidSignature());

	// verify sender is at this slot
	uint32_t slot = get_slot_at_time(fc::time_point_sec(bh.timestamp().convert_to<uint32_t>()));
	types::AccountName scheduled_producer = get_scheduled_producer(slot);

	const types::AccountName& producer = bh.producer();

	ctrace << "scheduled_producer: " << scheduled_producer;
	ctrace << "block producer: " << producer;
	if (scheduled_producer != producer)
		BOOST_THROW_EXCEPTION(InvalidProducer());

	return get_producer(scheduled_producer);
}
