
#include <libdevcore/SHA3.h>
#include <libethereum/State.h>
#include "Vote.h"

using namespace dev;
using namespace eth;

Vote::Vote(std::map<dev::h256, std::pair<dev::u256,dev::u256>>& map, std::unordered_map<dev::u256, dev::u256>& mapChange, const dev::Address& address, bool isCandidate, uint64_t votedNumber,
	uint64_t unAssignNumber, uint64_t assignNumbe, bool isVoted, const dev::Address& voteTo, uint64_t receivedVoteNumber)
	: m_map(map), m_mapChange(mapChange), m_address(address), m_isCandidate(isCandidate), m_votedNumber(votedNumber), m_unAssignNumber(unAssignNumber),
	m_assignNumber(assignNumbe), m_isVoted(isVoted), m_voteTo(voteTo), m_receivedVoteNumber(receivedVoteNumber)
{
}

Vote::~Vote()
{
}

void Vote::load()
{
	dev::bytes loadFromBytes;
	dev::bytes tempBytes;

	loadFromBytes = loadFromMap(size());
	auto iterator = loadFromBytes.begin();

	tempBytes = dev::bytes(iterator, iterator + sizeof(m_isCandidate));
	iterator += sizeof(m_isCandidate);
	m_isCandidate = tempBytes.at(0) == '\0' ? 0 : 1;

	tempBytes = dev::bytes(iterator, iterator + sizeof(m_votedNumber));
	iterator += sizeof(m_votedNumber);
	m_votedNumber = bytesToUint64_t(tempBytes);
	tempBytes = dev::bytes(iterator, iterator + sizeof(m_unAssignNumber));
	iterator += sizeof(m_unAssignNumber);
	m_unAssignNumber = bytesToUint64_t(tempBytes);
	tempBytes = dev::bytes(iterator, iterator + sizeof(m_assignNumber));
	iterator += sizeof(m_assignNumber);
	m_assignNumber = bytesToUint64_t(tempBytes);

	tempBytes = dev::bytes(iterator, iterator + sizeof(m_isVoted));
	iterator += sizeof(m_isVoted);
	m_isVoted = tempBytes.at(0) == '\0' ? 0 : 1;

	tempBytes = dev::bytes(iterator, iterator + sizeof(m_voteTo));
	iterator += sizeof(m_voteTo);
	m_voteTo = dev::Address(tempBytes);

	tempBytes = dev::bytes(iterator, iterator + sizeof(m_receivedVoteNumber));
	iterator += sizeof(m_receivedVoteNumber);
	m_receivedVoteNumber = bytesToUint64_t(tempBytes);
}

void Vote::save()
{
	dev::bytes saveInBytes;
	dev::bytes tempBytes;

	m_isCandidate == 0 ? saveInBytes.push_back('\0') : saveInBytes.push_back('\1');

	tempBytes = uint64_tToBytes(m_votedNumber);
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());
	tempBytes = uint64_tToBytes(m_unAssignNumber);
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());
	tempBytes = uint64_tToBytes(m_assignNumber);
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());

	m_isVoted == 0 ? saveInBytes.push_back('\0') : saveInBytes.push_back('\1');

	tempBytes = m_voteTo.asBytes();
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());

	tempBytes = uint64_tToBytes(m_receivedVoteNumber);
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());

	saveInMaps(saveInBytes);
}

dev::bytes Vote::uint64_tToBytes(uint64_t number)
{
	dev::bytes res;
	byte val;
	for (int i = 0; i < 8; i++)
	{
		val = number % 8;
		number = number / 8;
		res.push_back(val);
	}
	return res;
}

uint64_t Vote::bytesToUint64_t(const dev::bytes& bytes)
{
	if (bytes.size() != 8)
		BOOST_THROW_EXCEPTION(std::length_error("Bytes length is not 8."));
	uint64_t res = 0;
	for (int i = 7; i >= 0; i--)
	{
		res *= 8;
		res += bytes.at(i);
	}
	return res;
}

dev::bytes Vote::u256ToBytes(dev::u256 number)
{
	std::string s = number.str();
	return dev::bytes(s.begin(), s.end());
}

dev::u256 Vote::bytesToU256(const dev::bytes & bytes)
{
	std::string s(bytes.begin(), bytes.end());
	return dev::u256(s);
}

dev::u256 Vote::expand(uint64_t number, uint32_t m)
{
	dev::bytes expandKey = m_address.asBytes();
	byte val;

	// Expand key with num.
	for (int i = 0; i < 8; i++)
	{
		val = number % 8;
		number = number / 8;
		expandKey.push_back(val);
	}

	// Expand key with m.
	expandKey.insert(expandKey.end(), 4, '\0');

	dev::h256 temp;
	memcpy(temp.data(), expandKey.data(), 32);

	return (dev::u256)temp;
}

void Vote::saveInMaps(const dev::bytes& data)
{
	uint64_t size = data.size();
	uint64_t page = size / 32;
	uint64_t segment = size % 32;
	dev::h256 pageH256;

	// Write complete pages.
	for (uint64_t i = 0; i < page; i++)
	{
		memcpy(pageH256.data(), data.data() + (i * 32), 32);
		//m_map[expand(i)] = (dev::u256)pageH256;
		u256 key = expand(i);
		h256 const hashedKey(key);
		m_map[hashedKey] = std::make_pair(key, (dev::u256)pageH256);
		m_mapChange[key] = (dev::u256)pageH256;
	}

	// Write last page.
	dev::bytes lastPage(data.begin() + (page * 32), data.end());
	lastPage.insert(lastPage.end(), (32 - segment), '\0');
	memcpy(pageH256.data(), lastPage.data(), 32);
	//std::cout << "expand(page): " << expand(page).str() << std::endl;
	//for (auto i : m_map)
	//{
	//	std::cout << "i.first: " << i.first.str() << std::endl;
	//}
	//m_map[expand(page)] = (dev::u256)pageH256;
	//m_mapChange[expand(page)] = (dev::u256)pageH256;

	u256 key = expand(page);
	h256 const hashedKey(key);
	m_map[hashedKey] = std::make_pair(key, (dev::u256)pageH256);
	m_mapChange[key] = (dev::u256)pageH256;

}

dev::bytes Vote::loadFromMap(uint64_t size)
{
	uint64_t page = size / 32;
	uint64_t segment = size % 32;
	dev::h256 pageH256;
	dev::bytes res;
	dev::bytes bytesTemp;
	//std::unordered_map<dev::u256, dev::u256>::iterator it;

	// Write complete pages.
	for (uint64_t i = 0; i < page; i++)
	{
		dev::u256 const key = expand(i);
		dev::h256 const hashedKey(key);
		auto it = m_map.find(hashedKey);
		if (it == m_map.end())
		{
			pageH256 = dev::h256();
		}
		else
		{
			//pageH256 = (dev::h256)m_map.at(expand(i));
			pageH256 = (dev::h256)(it->second.second);
		}
		bytesTemp = pageH256.asBytes();
		res.insert(res.end(), bytesTemp.begin(), bytesTemp.end());
	}

	// Write last page.
	dev::u256 key = expand(page);
	dev::h256 const hashedKey(key);
	//it = m_map.find(key);
	auto it = m_map.find(hashedKey);

	if (it == m_map.end())
	{
		pageH256 = dev::h256();
	}
	else
	{
		//pageH256 = (dev::h256)m_map.at(expand(page));
		pageH256 = (dev::h256)(it->second.second);
	}
	bytesTemp = pageH256.asBytes();
	res.insert(res.end(), bytesTemp.begin(), bytesTemp.begin() + segment);
	return res;
}

void Vote::resetVotedTo(const dev::Address& address)
{
	dev::bytes bytes0(12, '\0');
	for (auto iterator = m_map.begin(); iterator != m_map.end(); iterator++)
	{
		dev::u256 iteratorFirst = iterator->first;
		dev::bytes iteratorBytes = ((dev::h256)iteratorFirst).asBytes();
		dev::bytes iteratorAddress(iteratorBytes.begin(), iteratorBytes.begin() + sizeof(dev::Address));
		dev::bytes iteratorExpand(iteratorBytes.begin() + sizeof(dev::Address), iteratorBytes.end());

		if (iteratorExpand == bytes0)
		{
			/// Reset myself vote info.
			if (dev::Address(iteratorAddress) == m_address)
			{
				if (m_voteTo == m_address)
				{
					m_isVoted = false;
					m_voteTo = dev::Address();
				}
			}
			/// Reset another vote info.
			else
			{
				Vote reset(m_map, m_mapChange, dev::Address(iteratorAddress));
				reset.load();
				if (reset.getVoteTo() == address)
				{
					reset.setIsVoted(false);
					reset.setVoteTo(dev::Address());
					reset.save();
				}

			}
		}
	}
}

uint64_t Vote::size()
{
	return sizeof(m_isCandidate) + sizeof(m_votedNumber) + sizeof(m_unAssignNumber) + sizeof(m_assignNumber) +
		sizeof(m_isVoted) + sizeof(m_voteTo) + sizeof(m_receivedVoteNumber);
}

void Vote::setIsCandidate(bool number)
{
	m_isCandidate = number;
}

void Vote::setVotedNumber(uint64_t number)
{
	m_votedNumber = number;
}

void Vote::setUnAssignNumber(uint64_t number)
{
	m_unAssignNumber = number;
}

void Vote::setAssignNumber(uint64_t number)
{
	m_assignNumber = number;
}

void Vote::setIsVoted(bool number)
{
	m_isVoted = number;
}

void Vote::setVoteTo(const dev::Address& address)
{
	m_voteTo = address;
}

void Vote::setReceivedVoteNumber(uint64_t number)
{
	m_receivedVoteNumber = number;
}

bool Vote::getIsCandidate()
{
	return m_isCandidate;
}

uint64_t Vote::getVotedNumber()
{
	return m_votedNumber;
}

uint64_t Vote::getUnAssignNumber()
{
	return m_unAssignNumber;
}

uint64_t Vote::getAssignNumber()
{
	return m_assignNumber;
}

bool Vote::getIsVoted()
{
	return m_isVoted;
}

dev::Address Vote::getVoteTo()
{
	return m_voteTo;
}

uint64_t Vote::getReceivedVoteNumber()
{
	return m_receivedVoteNumber;
}

const std::unordered_map<dev::u256, dev::u256>& Vote::getMapChange()
{
	return m_mapChange;
}

int Vote::mortgage(uint64_t balance)
{
	m_unAssignNumber += balance / VOTES_PRE_ETH;
	//std::cout << "mortgage:" << balance << "eth." << "m_unAssignNumber:" << balance / VOTES_PRE_ETH << std::endl;
	return 1;
}

int Vote::redeem(uint64_t voteCount)
{
	if (m_unAssignNumber < voteCount)
	{
		return 0;
	}
	m_unAssignNumber -= voteCount;

	return 1;
}

int Vote::vote(const dev::Address& address)
{
	/// Vote to myself.
	if (address == m_address)
	{
		if (m_isCandidate == 0)
		{
			///Illegal candidate.
			return 0;
		}
		/// Didn't vote before.
		if (m_isVoted == false)
		{
			m_voteTo = address;
			m_isVoted = true;
			m_votedNumber += m_assignNumber;
		}
		/// Voted before.
		else
		{
			if (m_voteTo == address)
			{
				return 2;
			}
			else
			{
				Vote before(m_map, m_mapChange, m_voteTo);
				before.load();
				before.setVotedNumber(before.getVotedNumber() - m_assignNumber);
				before.save();

				m_voteTo = address;
				m_isVoted = true;
				m_votedNumber += m_assignNumber;
			}
		}
	}
	/// Vote to another.
	else
	{
		Vote to(m_map, m_mapChange, address);
		to.load();
		if (to.getIsCandidate() == false)
		{
			///Illegal candidate.
			return 0;
		}
		/// Didn't vote before.
		if (m_isVoted == false)
		{
			m_voteTo = address;
			m_isVoted = true;
			to.setVotedNumber(to.getVotedNumber() + m_assignNumber);
			to.save();
		}
		/// Voted before.
		else
		{
			/// Vote to the account again.
			if (m_voteTo == address)
			{
				return 2;
			}

			/// Vote to myself before.
			if (m_voteTo == m_address)
			{
				m_votedNumber -= m_assignNumber;
			}
			/// Vote to another before.
			else
			{
				Vote before(m_map, m_mapChange, m_voteTo);
				before.load();
				before.setVotedNumber(before.getVotedNumber() - m_assignNumber);
				before.save();
			}

			m_voteTo = address;
			m_isVoted = true;
			to.setVotedNumber(to.getVotedNumber() + m_assignNumber);
			to.save();
		}
	}

	
	return 1;
}

int Vote::removeVote()
{
	if (m_isVoted == false)
	{
		return 2;
	}
	else
	{
		/// Remove vote to myself.
		if (m_voteTo == m_address)
		{
			m_votedNumber -= m_assignNumber;
			m_isVoted = false;
			m_voteTo = dev::Address();
		}
		/// Remove vote to another.
		else
		{
			Vote before(m_map, m_mapChange, m_voteTo);
			before.load();
			before.setVotedNumber(before.getVotedNumber() - m_assignNumber);
			before.save();

			m_isVoted = false;
			m_voteTo = dev::Address();
		}
	}
	return 1;
}

int Vote::assign(uint64_t voteCount)
{
	if (m_unAssignNumber < voteCount)
	{
		return 0;
	}
	/// Vote immediately.
	if (m_isVoted == true)
	{
		/// Move vote to myself.
		if (m_voteTo == m_address)
		{
			m_votedNumber += voteCount;
		}
		/// Move vote to another.
		else
		{
			Vote before(m_map, m_mapChange, m_voteTo);
			before.load();
			before.setVotedNumber(before.getVotedNumber() + voteCount);
			before.save();
		}
	}
	m_unAssignNumber -= voteCount;
	m_assignNumber += voteCount;
	return 1;
}

int Vote::deAssign(uint64_t voteCount)
{
	if ((m_assignNumber - m_receivedVoteNumber) < voteCount)
	{
		return 0;
	}
	/// Remote immediately.
	if (m_isVoted == true)
	{
		/// Remove vote from myself.
		if (m_voteTo == m_address)
		{
			m_votedNumber -= voteCount;
		}
		/// Remove vote from another.
		else
		{
			Vote before(m_map, m_mapChange, m_voteTo);
			before.load();
			before.setVotedNumber(before.getVotedNumber() - voteCount);
			before.save();
		}
	}
	m_assignNumber -= voteCount;
	m_unAssignNumber += voteCount;
	return 1;
}

int Vote::send(const dev::Address& address, uint64_t voteCount)
{
	if (address == m_address || m_unAssignNumber < voteCount)
	{
		return 0;
	}
	m_unAssignNumber -= voteCount;
	
	Vote to(m_map, m_mapChange, address);
	to.load();
	to.setReceivedVoteNumber(to.getReceivedVoteNumber() + voteCount);
	to.setAssignNumber(to.getAssignNumber() + voteCount);

	/// Vote immediately.
	if (to.getIsVoted() == true)
	{
		/// Vote to myself.
		if (to.getVoteTo() == m_address)
		{
			m_votedNumber += m_votedNumber;
		}
		/// Vote to himself.
		else if (to.getVoteTo() == address)
		{
			to.setVotedNumber(to.getVotedNumber() + voteCount);
		}
		/// Vote to another.
		else
		{
			Vote before(m_map, m_mapChange, to.getVoteTo());
			before.load();
			before.setVotedNumber(before.getVotedNumber() + voteCount);
			before.save();
		}
	}
	to.save();

	return 1;
}

int Vote::candidateRegister()
{
	if (m_isCandidate == false)
	{
		m_votedNumber = 0;
	}
	m_isCandidate = true;
	return 1;
}

int Vote::candidateDeregister()
{
	resetVotedTo(m_address);
	m_votedNumber = 0;
	m_isCandidate = false;
	return 1;
}


void VoteDelegate::mortgage(uint64_t amount, dev::Address const& _address, dev::eth::State& _state)
{
	if (_state.balance(_address) >= amount)
	{
		std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
		std::unordered_map<dev::u256, dev::u256> mapChange;
		Vote vote(voteMap, mapChange, _address);
		vote.load();
		uint64_t validBalance = amount / VOTES_PRE_ETH * VOTES_PRE_ETH;
		_state.subBalance(_address, validBalance);
		vote.mortgage(validBalance);
		vote.save();
		for (auto i : mapChange)
		{
			_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
		}
	}


}


void VoteDelegate::redeem(uint64_t voteCount, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	if (vote.redeem(voteCount))
	{
		_state.addBalance(_address, voteCount * VOTES_PRE_ETH);
	}
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	}
}
void VoteDelegate::candidateRegister(dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.candidateRegister();
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	}
}
void VoteDelegate::candidateDeregister(dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.candidateDeregister();
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	}
}

void VoteDelegate::vote(dev::Address _toAddress, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.vote(_toAddress);
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	}
}

void VoteDelegate::removeVote(dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.removeVote();
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	}

}

void VoteDelegate::send(dev::Address _toAddress, uint64_t _amount, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.send(_toAddress, _amount);
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	}
}

void VoteDelegate::assign(uint64_t voteCount, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.assign(voteCount);
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	}
}
void VoteDelegate::deAssign(uint64_t voteCount, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.deAssign(voteCount);
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	}
	
}

