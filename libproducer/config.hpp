#pragma once
//#include <boost/multiprecision/cpp_int.hpp>
#include <libdevcore/Common.h>
#include <fc/time.hpp>
#include <libproducer/version.hpp>

namespace dev {

namespace types {
	using UInt128 = u128;
	using Uint64 = uint64_t;
	using Int64 = int64_t;
}


namespace eth {

namespace config {

	using types::UInt128;

	const static int BlockIntervalSeconds = 3;
	 
	const static int BlocksPerYear = (365 * 24 * 60 * 60 / BlockIntervalSeconds);
	const static int BlocksPerDay = (24 * 60 * 60 / BlockIntervalSeconds);
	//const static int StartMinerVotingBlock = BlocksPerDay;
	//用于测试暂时改为一轮
	const static int StartMinerVotingBlock = 21;

	//每轮DPOS Worker总数
	const static int DPOSProducersPerRound = 17;
	//每轮靠选票稳定出的DPOS见证人总数
	const static int DPOSVotedProducersPerRound = 16;
	//每轮虚拟赛跑的见证人总数
	const static int DPOSRunnerupProducersPerRound = 1;
	//每轮POW 见证人总数
	const static int POWProducersPerRound = 4;
	//每轮见证人总数
	const static int TotalProducersPerRound = 21;


	//ETI创世时间
	const static fc::time_point_sec ETI_GenesisTime = (fc::time_point_sec(0));

	//ETI当前版本
	const static eth::chain::version ETI_BlockchainVersion = (eth::chain::version(0, 0, 0));

	//ETI当前硬分叉版本
	const static eth::chain::hardfork_version ETI_BlockchainHardforkVersion = (eth::chain::hardfork_version(ETI_BlockchainVersion));

	//ETI目前硬分叉数
	const static int ETI_HardforkNum = 0;
	
	//硬分叉投票最小人数
	const static int ETI_HardforkRequiredProducers = 1;

	const static int Percent100 = 10000;
	const static int Percent1 = 100;
	const static int IrreversibleThresholdPercent = 70 * Percent1;

	const static int TransactionOfBlockLimit = 10240;

	const static UInt128 ProducerRaceLapLength = std::numeric_limits<UInt128>::max();
}

template<typename Number>
Number ETH_PERCENT(Number value, int percentage) {
	return value * percentage / config::Percent100;
}
} // namespace eth
} // namespace dev
