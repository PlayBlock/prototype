#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/Address.h>
#include "libproducer/types.hpp"


/// 1 Eth = 100 Votes
#define VOTES_PRE_ETH 10000000000000000

class dev::eth::State;

class StateMap
{
public:
	StateMap(
		std::map<dev::h256, std::pair<dev::u256, dev::u256>>& map, 
		std::unordered_map<dev::u256, dev::u256>& mapChange, 
		const dev::Address& address);
	~StateMap();

	void load();
	void save();
	virtual uint64_t size() { return 0; } 
	std::unordered_map<dev::u256, dev::u256>& getMapChange() { return m_mapChange; }
	std::map<dev::h256, std::pair<dev::u256, dev::u256>>& getMap() { return m_map; }
	dev::Address getAddress() const { return m_address; }

	virtual void _loadImpl(const dev::bytes& loadedBytes) = 0;
	virtual dev::bytes _saveImpl() = 0; 

protected:
	dev::bytes uint64_tToBytes(uint64_t number);
	uint64_t bytesToUint64_t(const dev::bytes& bytes);
	dev::bytes u256ToBytes(dev::u256 number);
	dev::u256 bytesToU256(const dev::bytes& bytes);
	dev::bytes h160ToBytes(dev::h160 number);
	dev::h160 bytesToH160(const dev::bytes& bytes);

	dev::bytes h256ToBytes(dev::h256 number);
	dev::h256 bytesToH256(const dev::bytes& bytes);

	

	dev::u256 expand(uint64_t number, uint32_t m = 0);
	void saveInMaps(const dev::bytes& data);
	dev::bytes loadFromMap(uint64_t size);

private:
	std::map<dev::h256, std::pair<dev::u256, dev::u256>>& m_map; ///存储投票信息的map
	dev::Address m_address;                                      ///投票信息对应的地址
	std::unordered_map<dev::u256, dev::u256>& m_mapChange;       ///存储投票信息改变的map

};


class Vote : public StateMap
{
public:
	Vote(
		std::map<dev::h256, std::pair<dev::u256, dev::u256>>& map, 
		std::unordered_map<dev::u256, dev::u256>& mapChange, 
		const dev::Address& address, 
		bool isCandidate = false, uint64_t votedNumber = 0,
		uint64_t unAssignNumber = 0, uint64_t assignNumbe = 0, bool isVoted = false,
		const dev::Address& voteTo = dev::Address(), uint64_t receivedVoteNumber = 0);
	~Vote();
	 

	virtual void _loadImpl(const dev::bytes& loadedBytes);
	virtual dev::bytes _saveImpl(); 
	virtual uint64_t size();

	void setIsCandidate(bool number);
	void setVotedNumber(uint64_t number);
	void setUnAssignNumber(uint64_t number);
	void setAssignNumber(uint64_t number);
	void setIsVoted(bool number);
	void setVoteTo(const dev::Address& address);
	void setReceivedVoteNumber(uint64_t number);

	bool getIsCandidate();
	uint64_t getVotedNumber();
	uint64_t getUnAssignNumber();
	uint64_t getAssignNumber();
	bool getIsVoted();
	dev::Address getVoteTo();
	uint64_t getReceivedVoteNumber(); 

	int mortgage(uint64_t balance);                             ///抵押
	int redeem(uint64_t voteCount);                             ///赎回
	int vote(const dev::Address& address);                      ///投票
	int removeVote();                                           ///取消投票
	int assign(uint64_t voteCount);                             ///未分配转已分配
	int deAssign(uint64_t voteCount);                           ///已分配转未分配
	int send(const dev::Address& address, uint64_t voteCount);  ///未分配投票权发送
	int candidateRegister();                                    ///注册成为块生产者
	int candidateDeregister();                                  ///取消注册成为块生产者

protected:
	bool m_isCandidate;                                         ///是否是生产者
	uint64_t m_votedNumber;                                     ///生产者得票数
	uint64_t m_unAssignNumber;                                  ///尚未分配投票权
	uint64_t m_assignNumber;                                    ///已分配投票权
	bool m_isVoted;                                             ///是否已经投票
	dev::Address m_voteTo;                                      ///将已分配投票权投给的地址
	uint64_t m_receivedVoteNumber;                              ///收到他人的投票权，这部分投票权只能作为已分配投票全，不能转为尚未分配投票权

 

	void resetVotedTo(const dev::Address& address);
};

class POW_Operation : public StateMap
{
public:
	POW_Operation(
		std::map<dev::h256, std::pair<dev::u256, dev::u256>>& map,
		std::unordered_map<dev::u256, dev::u256>& mapChange,
		const dev::Address& address);
	~POW_Operation();

	virtual void _loadImpl(const dev::bytes& loadedBytes);
	virtual dev::bytes _saveImpl();
	virtual uint64_t size();

	//根据block_id与nonce生成input
	dev::h256 work_input();

	//验证结果是否正确
	bool validate();

	//算hash
	void create(const fc::ecc::private_key& w, const dev::h256& i);


//protected:

	dev::h512			 worker_pubkey;	//pow worker公钥
	dev::h256		 block_id;		//当前块hash
	uint64_t		 nonce = 0;  
	dev::h256        input; 
	dev::h256        work; 
	dev::Signature		 signature; 
	
};

struct VoteBace
{
	dev::Address m_address;
	bool m_isCandidate;
	uint64_t m_votedNumber;
	uint64_t m_unAssignNumber;
	uint64_t m_assignNumber;
	bool m_isVoted;
	dev::Address m_voteTo;
	uint64_t m_receivedVoteNumber;
};

class VoteDelegate
{
public:
	static void mortgage(uint64_t amount, dev::Address const& _address, dev::eth::State& _state);
	static void redeem(uint64_t amount, dev::Address const& _address, dev::eth::State& _state);
	
	static void candidateRegister(dev::Address const& _address, dev::eth::State& _state);
	static void candidateDeregister(dev::Address const& _address, dev::eth::State& _state);

	static void vote(dev::Address _toAddress, dev::Address const& _address, dev::eth::State& _state);
	static void removeVote(dev::Address const& _address, dev::eth::State& _state);
	static void send(dev::Address _toAddress, uint64_t amount, dev::Address const& _address, dev::eth::State& _state);

	static void assign(uint64_t amount, dev::Address const& _address, dev::eth::State& _state);
	static void deAssign(uint64_t amount, dev::Address const& _address, dev::eth::State& _state); 

	//POW相关 
	static void pow(
		dev::Address const& _address, dev::eth::State& _state, 
		dev::h256 block_id, //块hash
		uint64_t nonce, 
		dev::h256 input, 
		dev::h256 work);
};

