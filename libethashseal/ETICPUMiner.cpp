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
	//auto target = w.target;

	m_jumpHash.prepare_calc_pow(w.blockId, tryNonce, w.privateKey, w.target);


	std::map<dev::h256, std::pair<dev::u256, dev::u256>> map;
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Address address = w.workerAccount;
	POW_Operation op(map, mapChange, address);

	op.block_id = w.blockId;
	op.nonce = tryNonce;
	op.worker_pubkey = toPublic(w.privateKey);

	unsigned checkCount = 1;

	for (; !shouldStop(); checkCount++) {
		if (m_jumpHash.is_calc_finished()) {

			m_jumpHash.query_pow_result(op.nonce, op.input, op.signature, op.work);
			submitProof(ETIProofOfWork::Solution{ op });
			m_jumpHash.validate_pow_result(op.worker_pubkey, op.block_id, op.nonce, op.input, op.work, op.signature, w.target);
			break;
		}

		if (!(checkCount % 100))
			accumulateHashes(100);
	}
}

std::string ETICPUMiner::platformInfo()
{
	string baseline = toString(std::thread::hardware_concurrency()) + "-thread CPU";
	return baseline;
}
