#include "ETICPUMiner.h"

#include <thread>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <random>
//#include "ETIProofOfWork.h"
#include <libevm/Vote.h>


using namespace std;
using namespace dev;
using namespace eth;

unsigned ETICPUMiner::s_numInstances = 0;


ETICPUMiner::ETICPUMiner(GenericMiner<ETIProofOfWork>::ConstructionInfo const& _ci) :
	GenericMiner<ETIProofOfWork>(_ci), Worker("miner" + toString(index()))
{
}

ETICPUMiner::~ETICPUMiner()
{
	terminate();
}

void ETICPUMiner::kickOff()
{
	stopWorking();
	startWorking();
}

void ETICPUMiner::pause()
{
	stopWorking();
}

void ETICPUMiner::workLoop()
{
	auto tid = std::this_thread::get_id();
	static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

	uint64_t tryNonce = s_eng();

	WorkPackage w = work();
	auto target = w.target;

	std::map<dev::h256, std::pair<dev::u256, dev::u256>> map;
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Address address = w.workerAccount;
	POW_Operation op(map, mapChange, address);

	op.block_id = w.blockId;
	op.nonce = tryNonce;


	//h256 boundary = w.boundary;
	unsigned hashCount = 1;
	for (; !shouldStop(); tryNonce++, hashCount++)
	{
		op.nonce = tryNonce;
		op.create(w.privateKey, op.work_input());
		// 计算结果小于等于target的时候退出，报告找到的这个解
		if (op.work <= target && submitProof(ETIProofOfWork::Solution{ op }))
			break;

		if (!(hashCount % 100))
			accumulateHashes(100);
	}
}

std::string ETICPUMiner::platformInfo()
{
	string baseline = toString(std::thread::hardware_concurrency()) + "-thread CPU";
	return baseline;
}
