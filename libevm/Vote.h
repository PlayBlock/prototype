#pragma once

#include "../libdevcore/Common.h"
#include "../libevm/ExtVMFace.h"


/// 1 Eth = 100 Votes
#define VOTES_PRE_ETH 10000000000000000

class Vote
{
public:
	Vote(std::unordered_map<dev::u256, dev::u256>& map, std::unordered_map<dev::u256, dev::u256>& mapChange, const dev::Address& address, bool isCandidate = false, uint64_t votedNumber = 0,
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

	int mortgage(uint64_t balance);                             ///��Ѻ
	int redeem(uint64_t voteCount);                             ///���
	int vote(const dev::Address& address);                      ///ͶƱ
	int removeVote();                                           ///ȡ��ͶƱ
	int assign(uint64_t voteCount);                             ///δ����ת�ѷ���
	int deAssign(uint64_t voteCount);                           ///�ѷ���תδ����
	int send(const dev::Address& address, uint64_t voteCount);  ///δ����ͶƱȨ����
	int candidateRegister();                                    ///ע���Ϊ��������
	int candidateDeregister();                                  ///ȡ��ע���Ϊ��������

protected:
	bool m_isCandidate;                                         ///�Ƿ���������
	uint64_t m_votedNumber;                                     ///�����ߵ�Ʊ��
	uint64_t m_unAssignNumber;                                  ///��δ����ͶƱȨ
	uint64_t m_assignNumber;                                    ///�ѷ���ͶƱȨ
	bool m_isVoted;                                             ///�Ƿ��Ѿ�ͶƱ
	dev::Address m_voteTo;                                      ///���ѷ���ͶƱȨͶ���ĵ�ַ
	uint64_t m_receivedVoteNumber;                              ///�յ����˵�ͶƱȨ���ⲿ��ͶƱȨֻ����Ϊ�ѷ���ͶƱȫ������תΪ��δ����ͶƱȨ

	std::unordered_map<dev::u256, dev::u256>& m_map;            ///�洢ͶƱ��Ϣ��map
	dev::Address m_address;                                     ///ͶƱ��Ϣ��Ӧ�ĵ�ַ
	std::unordered_map<dev::u256, dev::u256>& m_mapChange;      ///�洢ͶƱ��Ϣ�ı��map

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
