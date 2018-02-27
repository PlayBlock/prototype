#include "jumphash_sdk.h"


jump_hash::jump_hash() 
{
	m_work = work_package();
	m_solution = solution();
	m_solution_is_ready = false;

	init_jumphash_sdk();
}

jump_hash::~jump_hash()
{
	destroy_jumphash_sdk();
	terminate();
}

void jump_hash::init_jumphash_sdk()
{
}

void jump_hash::destroy_jumphash_sdk()
{
}

void jump_hash::prepare_calc_pow(const h256& block_id, const uint64_t start_nonce, const Secret& worker_priv_key, const h256& target)
{
	if (m_work.blockId == block_id)
		return;

	Guard l(x_work);
	m_work = work_package();
	m_work.blockId = block_id;
	m_work.nonce = start_nonce;
	m_work.privateKey = worker_priv_key;
	m_work.target = target;

	m_solution_is_ready = false;

	DEV_TIMED_ABOVE("stop", 250)
		stop_calc_pow();
	DEV_TIMED_ABOVE("start", 250)
		start_calc_pow();

}

void jump_hash::start_calc_pow()
{
	stopWorking();
	startWorking();
}

bool jump_hash::is_calc_finished() 
{
	return m_solution_is_ready;
}

bool jump_hash::query_pow_result(uint64_t& nonce, h256& workinput, h520& signature, h256& work) 
{
	Guard l(x_solution);
	nonce = m_solution.nonce;
	workinput = m_solution.workinput;
	signature = m_solution.signature;
	work = m_solution.work;

	return true;
}

void jump_hash::stop_calc_pow() 
{
	stopWorking();
}

bool jump_hash::validate_pow_result(const h512& worker_pub_key, const h256& block_id, const uint64_t nonce, const h256& workinput, const h256& work, const h520& signature, const h256& target) 
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> map;
	std::unordered_map<dev::u256, dev::u256> mapChange;
	POW_Operation op(map, mapChange, toAddress(worker_pub_key));

	op.block_id = block_id;
	op.nonce = nonce;
	op.worker_pubkey = worker_pub_key;
	op.work = work;
	op.signature = signature;
	op.input = workinput;

	if (!op.validate()) 
		return false;

	return (op.work < target);
}

void jump_hash::workLoop() 
{
	work_package w = work();
	auto target = w.target;

	std::map<dev::h256, std::pair<dev::u256, dev::u256>> map;
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Address address = w.workerAccount;
	POW_Operation op(map, mapChange, address);

	op.block_id = w.blockId;
	op.nonce = w.nonce;
	op.worker_pubkey = toPublic(w.privateKey);

	uint64_t tryNonce = op.nonce;

	//h256 boundary = w.boundary;
	unsigned hashCount = 1;
	for (; !shouldStop(); tryNonce++, hashCount++)
	{
		op.nonce = tryNonce;
		op.create(w.privateKey, op.work_input());
		// 计算结果小于等于target的时候退出，报告找到的这个解
		if (op.work <= target)
		{
			Guard l(x_solution);
			m_solution.nonce = op.nonce;
			m_solution.signature = op.signature;
			m_solution.work = op.work;
			m_solution.workinput = op.work_input();

			m_solution_is_ready = true;
			break;
		}
	}
}
