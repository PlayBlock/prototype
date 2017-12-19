#pragma once

#include "../libdevcore/Common.h"
#include "../libevm/ExtVMFace.h"


/// 1 Eth = 100 Votes
#define VOTES_PRE_ETH 10000000000000000

class Vote
{
public:
	Vote(std::map<dev::h256, std::pair<dev::u256, dev::u256>>& map, std::unordered_map<dev::u256, dev::u256>& mapChange, const dev::Address& address, bool isCandidate = false, uint64_t votedNumber = 0,
		uint64_t unAssignNumber = 0, uint64_t assignNumbe = 0, bool isVoted = false,
		const dev::Address& voteTo = dev::Address(), uint64_t receivedVoteNumber = 0);
	~Vote();

	void load();
	void save();
	uint64_t size();

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
	const std::unordered_map<dev::u256, dev::u256>& getMapChange();

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

	std::map<dev::h256, std::pair<dev::u256, dev::u256>>& m_map;            ///存储投票信息的map
	dev::Address m_address;                                     ///投票信息对应的地址
	std::unordered_map<dev::u256, dev::u256>& m_mapChange;      ///存储投票信息改变的map

	dev::bytes uint64_tToBytes(uint64_t number);
	uint64_t bytesToUint64_t(const dev::bytes& bytes);
	dev::bytes u256ToBytes(dev::u256 number);
	dev::u256 bytesToU256(const dev::bytes& bytes);

	dev::u256 expand(uint64_t number, uint32_t m = 0);
	void saveInMaps(const dev::bytes& data);
	dev::bytes loadFromMap(uint64_t size);

	void resetVotedTo(const dev::Address& address);
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

