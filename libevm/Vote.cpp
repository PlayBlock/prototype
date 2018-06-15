
#include <libdevcore/SHA3.h>
#include <libethereum/State.h>
#include "Vote.h"
#include "libdevcore/Address.h"
#include <libjumphash/jumphash.h>

using namespace dev;
using namespace eth;

Vote::Vote(
	std::map<dev::h256, std::pair<dev::u256,dev::u256>>& map, 
	std::unordered_map<dev::u256, dev::u256>& mapChange, 
	const dev::Address& address, bool isCandidate, uint64_t votedNumber,
	uint64_t unAssignNumber, uint64_t assignNumbe, bool isVoted, const dev::Address& voteTo, uint64_t receivedVoteNumber)
	: StateMap(map,mapChange,address), m_isCandidate(isCandidate), m_votedNumber(votedNumber), m_unAssignNumber(unAssignNumber),
	m_assignNumber(assignNumbe), m_isVoted(isVoted), m_voteTo(voteTo), m_receivedVoteNumber(receivedVoteNumber)
{
}

Vote::~Vote()
{
}

 

void Vote::_loadImpl(const dev::bytes& loadedBytes)
{ 
	dev::bytes tempBytes; 
	auto iterator = loadedBytes.begin();

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

dev::bytes Vote::_saveImpl()
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

	return saveInBytes;
}
 

void Vote::resetVotedTo(const dev::Address& address)
{
	dev::bytes bytes0(12, '\0');
	for (auto iterator = getMap().begin(); iterator != getMap().end(); iterator++)
	{
		dev::u256 iteratorFirst = iterator->first;
		dev::bytes iteratorBytes = ((dev::h256)iteratorFirst).asBytes();
		dev::bytes iteratorAddress(iteratorBytes.begin(), iteratorBytes.begin() + sizeof(dev::Address));
		dev::bytes iteratorExpand(iteratorBytes.begin() + sizeof(dev::Address), iteratorBytes.end());

		if (iteratorExpand == bytes0)
		{
			/// Reset myself vote info.
			if (dev::Address(iteratorAddress) == getAddress())
			{
				if (m_voteTo == getAddress())
				{
					m_isVoted = false;
					m_voteTo = dev::Address();
				}
			}
			/// Reset another vote info.
			else
			{
				Vote reset(getMap(), getMapChange(), dev::Address(iteratorAddress));
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
 
int Vote::mortgage(uint64_t balance)
{
	m_unAssignNumber += balance / VOTES_PRE_ETH; 
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
	if (address == getAddress())
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
				Vote before(getMap(), getMapChange(), m_voteTo);
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
		Vote to(getMap(), getMapChange(), address);
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
			if (m_voteTo == getAddress())
			{
				m_votedNumber -= m_assignNumber;
			}
			/// Vote to another before.
			else
			{
				Vote before(getMap(), getMapChange(), m_voteTo);
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
		if (m_voteTo == getAddress())
		{
			m_votedNumber -= m_assignNumber;
			m_isVoted = false;
			m_voteTo = dev::Address();
		}
		/// Remove vote to another.
		else
		{
			Vote before(getMap(), getMapChange(), m_voteTo);
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
		if (m_voteTo == getAddress())
		{
			m_votedNumber += voteCount;
		}
		/// Move vote to another.
		else
		{
			Vote before(getMap(), getMapChange(), m_voteTo);
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
		if (m_voteTo == getAddress())
		{
			m_votedNumber -= voteCount;
		}
		/// Remove vote from another.
		else
		{
			Vote before(getMap(), getMapChange(), m_voteTo);
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
	if (address == getAddress() || m_unAssignNumber < voteCount)
	{
		return 0;
	}
	m_unAssignNumber -= voteCount;
	
	Vote to(getMap(), getMapChange(), address);
	to.load();
	to.setReceivedVoteNumber(to.getReceivedVoteNumber() + voteCount);
	to.setAssignNumber(to.getAssignNumber() + voteCount);

	/// Vote immediately.
	if (to.getIsVoted() == true)
	{
		/// Vote to myself.
		if (to.getVoteTo() == getAddress())
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
			Vote before(getMap(), getMapChange(), to.getVoteTo());
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
	resetVotedTo(getAddress());
	m_votedNumber = 0;
	m_isCandidate = false;
	return 1;
}


void VoteDelegate::mortgage(uint64_t amount, dev::Address const& _address, dev::eth::State& _state)
{
	if (_state.balance(_address) >= amount)
	{
		std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
		std::unordered_map<dev::u256, dev::u256> mapChange;
		Vote vote(voteMap, mapChange, _address);
		vote.load();
		uint64_t validBalance = amount / VOTES_PRE_ETH * VOTES_PRE_ETH;
		_state.subBalance(_address, validBalance);
		vote.mortgage(validBalance);
		vote.save();
		for (auto i : mapChange)
		{
			_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
		}
	}


}


void VoteDelegate::redeem(uint64_t voteCount, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
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
		_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
	}
}
void VoteDelegate::candidateRegister(dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.candidateRegister();
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
	}
}
void VoteDelegate::candidateDeregister(dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.candidateDeregister();
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
	}
}

void VoteDelegate::vote(dev::Address _toAddress, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.vote(_toAddress);
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
	}
}

void VoteDelegate::removeVote(dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.removeVote();
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
	}

}

void VoteDelegate::send(dev::Address _toAddress, uint64_t _amount, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.send(_toAddress, _amount);
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
	}
}

void VoteDelegate::assign(uint64_t voteCount, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.assign(voteCount);
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
	}
}
void VoteDelegate::deAssign(uint64_t voteCount, dev::Address const& _address, dev::eth::State& _state)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> voteMap = _state.storage(Address(VoteInfoAddress));
	std::unordered_map<dev::u256, dev::u256> mapChange;
	Vote vote(voteMap, mapChange, _address);
	vote.load();
	vote.deAssign(voteCount);
	vote.save();

	for (auto i : mapChange)
	{
		_state.setStorage(Address(VoteInfoAddress), i.first, i.second);
	}
	
}

void VoteDelegate::pow(dev::Address const& _address, 
	dev::eth::State& _state,
	dev::h512 worker_pubkey,
	dev::h256 block_id, //¿éhash
	uint64_t nonce,
	dev::h256 input,
	dev::h256 work,
	dev::Signature signature)
{
	std::map<dev::h256, std::pair<dev::u256, dev::u256>> powMap = _state.storage(Address(POWInfoAddress)); 
	std::unordered_map<dev::u256, dev::u256> mapChange;
	POW_Operation powop(powMap, mapChange, _address);

	powop.load();  
	powop.worker_pubkey = worker_pubkey;
	powop.block_id = block_id;
	powop.nonce = nonce;
	powop.input = input;
	powop.work = work;
	powop.signature = signature;
	powop.save();  

	for (auto i : mapChange)
	{
		_state.setStorage(Address(POWInfoAddress), i.first, i.second);
	} 
}

StateMap::StateMap(
	std::map<dev::h256, std::pair<dev::u256, dev::u256>>& map, 
	std::unordered_map<dev::u256, dev::u256>& mapChange, 
	const dev::Address& address):
	m_map(map),m_mapChange(mapChange),m_address(address)
{ 
}

StateMap::~StateMap()
{

}

void StateMap::load()
{ 
	this->_loadImpl(loadFromMap(size())); 
}

void StateMap::save()
{
   saveInMaps(this->_saveImpl());
}

 

dev::bytes StateMap::uint64_tToBytes(uint64_t number)
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

uint64_t StateMap::bytesToUint64_t(const dev::bytes& bytes)
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

dev::bytes StateMap::u256ToBytes(dev::u256 number)
{
	std::string s = number.str();
	return dev::bytes(s.begin(), s.end());
}

dev::u256 StateMap::bytesToU256(const dev::bytes& bytes)
{
	std::string s(bytes.begin(), bytes.end());
	return dev::u256(s);
}

dev::bytes StateMap::h160ToBytes(dev::h160 number)
{
	return number.asBytes(); 
}

dev::h160 StateMap::bytesToH160(const dev::bytes& bytes)
{
	return dev::h160(bytes);
}

dev::bytes StateMap::h256ToBytes(dev::h256 number)
{
	return number.asBytes();
}

dev::h256 StateMap::bytesToH256(const dev::bytes& bytes)
{
	return dev::h256(bytes);
}

dev::u256 StateMap::expand(uint64_t number, uint32_t m /*= 0*/)
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

void StateMap::saveInMaps(const dev::bytes& data)
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
		dev::h256 hashkey = key;
		dev::h256 const hashedKey = dev::sha3(hashkey);
		m_map[hashedKey] = std::make_pair(key, (dev::u256)pageH256);
		m_mapChange[key] = (dev::u256)pageH256;
	}

	// Write last page.
	dev::bytes lastPage(data.begin() + (page * 32), data.end());
	lastPage.insert(lastPage.end(), (32 - segment), '\0');
	memcpy(pageH256.data(), lastPage.data(), 32); 

	u256 key = expand(page);
	h256 hashkey = key;
	h256 const hashedKey = dev::sha3(hashkey);
	m_map[hashedKey] = std::make_pair(key, (dev::u256)pageH256);
	m_mapChange[key] = (dev::u256)pageH256;


}

dev::bytes StateMap::loadFromMap(uint64_t size)
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
		dev::h256 const hashkey = key;
		dev::h256 const hashedKey = dev::sha3(hashkey);
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
	dev::h256 const hashkey = key;
	dev::h256 const hashedKey = dev::sha3(hashkey);
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

POW_Operation::POW_Operation(
	std::map<dev::h256, std::pair<dev::u256, dev::u256>>& map, 
	std::unordered_map<dev::u256, dev::u256>& mapChange, 
	const dev::Address& address):StateMap(map,mapChange,address)
{

}

POW_Operation::~POW_Operation()
{

}

void POW_Operation::_loadImpl(const dev::bytes& loadedBytes)
{
	dev::bytes tempBytes;
	auto iterator = loadedBytes.begin();

	tempBytes = dev::bytes(iterator, iterator + sizeof(worker_pubkey));
	iterator += sizeof(worker_pubkey);
	worker_pubkey = dev::h512(tempBytes);

	tempBytes = dev::bytes(iterator, iterator + sizeof(block_id));
	iterator += sizeof(block_id);
	block_id = bytesToH256(tempBytes);


	tempBytes = dev::bytes(iterator, iterator + sizeof(nonce));
	iterator += sizeof(nonce);
	nonce = bytesToUint64_t(tempBytes);


	tempBytes = dev::bytes(iterator, iterator + sizeof(input));
	iterator += sizeof(input);
	input = bytesToH256(tempBytes);

	tempBytes = dev::bytes(iterator, iterator + sizeof(work));
	iterator += sizeof(work);
	work = bytesToH256(tempBytes); 

	tempBytes = dev::bytes(iterator, iterator + sizeof(signature));
	iterator += sizeof(signature);
	signature = Signature(tempBytes);
}

dev::bytes POW_Operation::_saveImpl()
{
	dev::bytes saveInBytes;
	dev::bytes tempBytes; 

	tempBytes = worker_pubkey.asBytes();
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());

	tempBytes = h256ToBytes(block_id);
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());

	tempBytes = uint64_tToBytes(nonce);
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());

	tempBytes = h256ToBytes(input);
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());
	 
	tempBytes = h256ToBytes(work);
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());

	tempBytes = signature.asBytes();
	saveInBytes.insert(saveInBytes.end(), tempBytes.begin(), tempBytes.end());

	return saveInBytes;
}

uint64_t POW_Operation::size()
{
	return 
		sizeof(worker_pubkey) + 
		sizeof(block_id) + 
		sizeof(nonce) + 
		sizeof(input) + 
		sizeof(work)+
		sizeof(signature); 
}

dev::h256 POW_Operation::work_input()
{
	auto hash = dev::sha3(block_id);
	dev::bytes nonceBytes = uint64_tToBytes(nonce);
	for (int i = 0; i < nonceBytes.size(); i++)
	{
		hash[i] = nonceBytes[i];
	}
	return dev::sha3(hash);
}

bool POW_Operation::validate()
{

	if (work_input() != input) return false;
	
	if (work == dev::h256()) return false;
	 
	if (Public(dev::recover(signature, input)) != worker_pubkey) return false;

	auto sig_hash = dev::sha3(signature);
	Public recover = dev::recover(signature, sig_hash);

	uint8_t output0[64], output1[64];
	Hex2Str((uint8_t*)recover.data(), output0, 32);

	int id = (((uint16_t *)block_id.data())[0]) % 13;
	jump[id](output0, output1);	bytesConstRef output1Bytes(output1, sizeof(output1));
	if (work != dev::sha3(output1Bytes)) return false;

	return true;
}


void POW_Operation::create(const fc::ecc::private_key& w, const dev::h256& i) {
	input = i;
	signature = dev::sign(w, input);
	auto sig_hash = dev::sha3(signature);
	Public recover = dev::recover(signature, sig_hash);
	//work = dev::sha3(recover);

	uint8_t output0[64], output1[64];
	Hex2Str((uint8_t*)recover.data(), output0, 32);
	int id = (((uint16_t *)block_id.data())[0]) % 13;
	jump[id](output0, output1);
	bytesConstRef output1Bytes(output1, sizeof(output1));

	work = dev::sha3(output1Bytes);
}