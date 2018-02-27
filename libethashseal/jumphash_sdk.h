#pragma once

#include <libdevcore/FixedHash.h>
#include "ETIProofOfWork.h"
#include "libdevcore/Worker.h"

using namespace dev;


class jump_hash : public Worker
{

	struct work_package {
		h256 blockId;
		Secret privateKey;
		types::AccountName workerAccount;
		dev::h256 target;
		uint64_t nonce;
	};

	struct solution {
		uint64_t nonce;
		h256 workinput;
		h520 signature;
		h256 work;
	};

public:
	jump_hash();
	~jump_hash();

	/*
	*参数：空
	*返回值：空
	*行为：初始化SDK状态，及硬件相关初始化动作
	*调用时机：矿工全节点在初始化时调用，在全节点生命周期中只调用一次
	*/
	void init_jumphash_sdk();

	/*
	参数：空
	*返回值：空
	*行为：销毁SDK申请的内存及清理硬件相关销毁动作
	*调用时机：矿工全节点在退出时调用，在全节点生命周期中只调用一次
	*/
	void destroy_jumphash_sdk();

	/*
	*参数： 
	*block_id 当前块ID，字长256位
	*start_nonce 起始nonce，每次枚举hash时在此基础上进行叠加，字长64位
	*worker_priv_key 矿工私钥，需要参与到解题算法中，字长256位
	*target 目标难度，算出的题解必须小于此值，字长256位
	*返回值：空
	*行为：传入算题必要参数，对后续解题做准备
	*调用时机：在矿工全节点每次接到新块时调用
	*/
	void prepare_calc_pow(const h256& block_id, const uint64_t start_nonce, const Secret& worker_priv_key, const h256& target);

	/*
	*参数：空
	*返回值：空
	*行为：启动硬件开始全力解题，具体算法参照上面的“解题算法”小节
	*调用时机：需要在prepare_calc_pow之后调用
	*/
	void start_calc_pow();

	/*
	*参数：空
	*返回值：bool 通知全节点客户端此时是否已算出满足难度要求的题解，true表示已算出题目，false表示未算出题目
	*行为：由sdk向客户端汇报是否已解出难题
	*调用时机：在begin_calc_pow后调用，用来轮询查看是否已解除难题
	*/
	bool is_calc_finished();

	/*
	*参数：
	*nonce: 算出题解所用的nonce
	*workinput: 算出题解的workinput
	*signature: 使用矿工私钥签名的workinput
	*work: 最终的题解
	*返回值：bool
	*true 获取成功，说明调用时刻已经算出满足难度的题解
	*false 获取失败，尚未解除满足难度的题解
	*行为：SDK需要传出题解及算出题解过程中的一些中间结果
	*调用时机：在客户端发现SDK已算完题时
	*/
	bool query_pow_result(uint64_t& nonce, h256& workinput, h520& signature, h256& work);

	/*
	*参数：空
	*返回值：空
	*行为：终止计算题解过程
	*调用时机：会在出现新题目时（新块出现尚未解出上一道题）调用
	*/
	void stop_calc_pow();

	/*
	*参数：
	*worker_pub_key： 矿工公钥
	*block_id: 算题基于的块ID
	*nonce: 算出题解所用的nonce
	*workinput: 算出题解所用的workinput
	*work: 最终的题解
	*signature: 用矿工私钥签名的workinput
	*target: 题目要满足的难度
	*返回值：bool
	*true 通过验证
	*false 未通过验证
	*行为：算法参见上面“验证算法”小节
	*调用时机：会在全节点客户端接到别人的pow题解时调用，对题解进行验证
	*/
	bool validate_pow_result(const h512& worker_pub_key, const h256& block_id, const uint64_t nonce, const h256& workinput, const h256& work, const h520& signature, const h256& target);

private:

	void workLoop() override;

	work_package const& work() const { Guard l(x_work); return m_work; }

	work_package m_work;
	mutable Mutex x_work;

	solution m_solution;
	bool m_solution_is_ready;
	mutable Mutex x_solution;

};

