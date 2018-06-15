/*
This file is part of cpp-etiam.

cpp-etiam is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

cpp-ethereum is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Vote.h
* @author dongzhe <dongzhe@fancytech.com>
* @date 2017
*/

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieDB.h>
#include <libdevcore/SHA3.h>
#include <libethcore/Common.h>
#include "State.h"
#include "Vote.h"
#include <array>
#include <assert.h>
using namespace std;
using namespace dev;
using namespace dev::eth;

const int U256Length = 256 / 8;
const int AddressLength = 160 / 8;

u256 UserStorage::addressToU256(Address const& _address, int _postffix)
{
	u256 _r((u160)_address);
	return (_r << (256 - 160)) + _postffix;
}


void UserStorage::LoadFixedSizeBtyes(bytes& _data, State const& _state, Address const& _storageaddress, Address const& _keyaddress, int _size ,int _startPage)
{
	int pageNum = (_size + 31) / 32;
	_data.reserve(_size);
	for (int i = _startPage; i < pageNum; i++)
	{
		u256 const key = addressToU256(_keyaddress, i);
		h256 value = (h256)_state.storage(_storageaddress, key);
		if (i == pageNum - 1)
		{
			_data.insert(_data.end(), value.data(), value.data() + (_size - i * 32));
		}
		else
		{
			_data.insert(_data.end(), value.data(), value.data() + 32);
		}
	}

}

void UserStorage::LoadFixedSizeBtyes(bytes& _data, map<h256, pair<u256, u256>> const& _storageMap, Address const& _storageaddress, Address const& _keyaddress, int _size, int _startPage)
{
	int pageNum = (_size + 31) / 32;
	_data.reserve(_size);
	for (int i = 0; i < pageNum; i++)
	{
		u256 const key = addressToU256(_keyaddress, i);
		h256 const h256key = key;	
		h256 value = _storageMap.find(sha3(h256key))->second.second;
		if (i == pageNum - 1)
		{
			_data.insert(_data.end(), value.data(), value.data() + (_size - i * 32));
		}
		else
		{
			_data.insert(_data.end(), value.data(), value.data() + 32);
		}
	}
}

//void UserStorage::LoadFixedSizeBtyes(bytes& _data, map<h256, pair<u256, u256>> _storageMap, Address const& _storageaddress, Address const& _keyaddress, int _size, int _startPage)
//{
//	map<Address, bytes> resMap;
//	map<h256, pair<u256, u256>>  storageMap = _state.storage(_storageaddress);
//
//	for (const auto &p : storageMap)
//	{
//		h256 keyh256 = p.second.first;
//
//		FixedHash<12> ret;
//		memcpy(ret.data(), keyh256.data() + 20, 12);
//		if (!ret)
//		{
//			Address address;
//			memcpy(address.data(), keyh256.data(), 20);
//			int pageNum = (_size + 31) / 32;
//			dev::bytes tmp;
//			tmp.reserve(_size);
//			for (int i = 0; i < pageNum; i++)
//			{
//				if (i != pageNum - 1)
//				{
//					dev::u256 const key = addressToU256(address, i);
//					dev::h256 value = (dev::h256)_state.storage(_storageaddress, key);
//					bytes bytesTemp = value.asBytes();
//					tmp.insert(tmp.end(), bytesTemp.begin(), bytesTemp.end());
//				}
//				else
//				{
//					dev::u256 const key = addressToU256(address, i);
//					dev::h256 value = (dev::h256)_state.storage(_storageaddress, key);
//					bytes bytesTemp = value.asBytes();
//					tmp.insert(tmp.end(), bytesTemp.begin(), bytesTemp.end());
//				}
//			}
//			resMap.emplace(address, tmp);
//		}
//
//	}
//	return resMap;
//}

void UserStorage::SaveBytes(State& _state, Address const& _storageaddress, Address const& _keyaddress, bytes const& _data)
{
	int pageNum = (_data.size() + U256Length - 1) / U256Length;
	bytes::const_iterator iterator = _data.begin();

	for (int i = 0; i < pageNum; i++)
	{
		if (i != pageNum - 1)
		{
			bytesConstRef tmp(&*iterator, U256Length);
			h256 value(tmp);

			_state.setStorage(_storageaddress, addressToU256(_keyaddress, i), value);
			iterator += U256Length;
		}
		else {
			int lastsize = _data.end() - iterator;
			bytesConstRef tmp(&*iterator, _data.end() - iterator);
			h256 value(tmp, h256::AlignLeft);

			_state.setStorage(_storageaddress, addressToU256(_keyaddress, i), value);
		}
	}
}



void VoteInfo::load(State& _state)
{
	u256 const key = UserStorage::addressToU256(m_address, 0);
	h256 const value = _state.storage(VoteInfoAddress, key);
	int size = *value.data() * (sizeof(Address) + sizeof(uint64_t)) + VoteInfo::BasicSize;
	bytes data;
	if (size > U256Length)
	{
		data.insert(data.end(), value.data(), value.data() + U256Length);
		UserStorage::LoadFixedSizeBtyes(data, _state, VoteInfoAddress, m_address, size, 1);
	}
	else {
		data.insert(data.end(), value.data(), value.data() + size);
	}
	initFromBytes(data);
}


void VoteInfo::save(State& _state)
{
	removeZeroVote();
	byte voteToNum = m_voteRecord.size();
	int size = voteToNum * (sizeof(Address) + sizeof(uint64_t)) + VoteInfo::BasicSize;
	bytes valueBytes(size, (byte)0);
	bytes::iterator iterator = valueBytes.begin();

	memcpy(&*iterator, &voteToNum, sizeof(voteToNum));
	iterator += sizeof(byte);

	byte boolValues = m_isCandidate;
	*iterator = boolValues;
	iterator += sizeof(byte);

	memcpy(&*iterator, &m_holdVotes, sizeof(m_holdVotes));
	iterator += sizeof(m_holdVotes);

	memcpy(&*iterator, &m_receivedVote, sizeof(m_receivedVote));
	iterator += sizeof(m_receivedVote);

	memcpy(&*iterator, m_name.data(), m_name.length());
	iterator += NameMaxSize;

	memcpy(&*iterator, m_url.data(), m_url.length());
	iterator += URLMaxSize;


	for (auto const& p : m_voteRecord)
	{
		memcpy(&*iterator, &p.first, sizeof(Address));
		iterator += sizeof(Address);
		memcpy(&*iterator, &p.second, sizeof(uint64_t));
		iterator += sizeof(uint64_t);
	}
	assert(iterator == valueBytes.end());

	UserStorage::SaveBytes(_state, VoteInfoAddress, m_address, valueBytes);

}

map<Address, VoteInfo> VoteInfo::getVoteInfoMap(State const& _state)
{
	map<h256, pair<u256, u256>>  storageMap = _state.storage(VoteInfoAddress);
	map<Address, VoteInfo> voteInfoMap;
	for (auto const &p : storageMap)
	{
		h256 keyh256 = p.second.first;
		FixedHash<U256Length - AddressLength> ret;
		memcpy(ret.data(), keyh256.data() + AddressLength, U256Length - AddressLength);
		if (!ret)
		{		
			Address address;
			memcpy(address.data(), keyh256.data(), AddressLength);
			h256 value = p.second.second;
			int size = *value.data() * (sizeof(Address) + sizeof(uint64_t)) + VoteInfo::BasicSize;

			bytes data;
			if (size > U256Length)
			{
				data.insert(data.end(), value.data(), value.data() + U256Length);
				UserStorage::LoadFixedSizeBtyes(data, _state, VoteInfoAddress, address, size, 1);
			}
			else {	
				data.insert(data.end(), value.data(), value.data() + size);
			}

			VoteInfo vote(address);
			voteInfoMap[address] = vote;
			voteInfoMap[address].initFromBytes(data);
		}
	}
	return voteInfoMap;


	//map<Address, bytes> resMap;

	//map<Address, VoteInfo> voteInfoMap;
	//map<Address, bytes> bytesMap = UserStorage::LoadFixedSizeBtyes(_state, VoteInfoAddress, Size);
	//for (auto const &p : bytesMap)
	//{
	//	VoteInfo info(p.first);
	//	info.initFromBytes(p.second);
	//	voteInfoMap.emplace(p.first, info);
	//}
	//return voteInfoMap;

}

h256 const mask = u256("0xffffffffffffffffffffffff");

map<Address, uint64_t> VoteInfo::getProducerMap(State const& _state)
{
	map<Address, uint64_t> res;
	map<h256, pair<u256, u256>>  storageMap = _state.storage(VoteInfoAddress);
	for (auto const &p : storageMap)
	{
		h256 keyh256 = p.second.first;
		h256 r = mask&keyh256;
		if (!r)
		{
			Address address;
			memcpy(address.data(), keyh256.data(), AddressLength);
			h256 value = p.second.second;

			if (*(value.data() + sizeof(byte)))
			{
				uint64_t votes;
				memcpy(&votes, value.data() + 2 * sizeof(byte) + sizeof(m_holdVotes), sizeof(votes));
				res[address] = votes;
			}
		}
	}
	return res;
}



void VoteInfo::initFromBytes(bytes const &_bytes)
{
	bytes::const_iterator iterator = _bytes.begin();
	byte voteToNum;
	memcpy(&voteToNum, &*iterator, sizeof(voteToNum));
	iterator += sizeof(byte);

	byte boolValues = *iterator;
	m_isCandidate = boolValues;
	iterator += sizeof(byte);

	memcpy(&m_holdVotes, &*iterator, sizeof(m_holdVotes));
	iterator += sizeof(m_holdVotes);

	memcpy(&m_receivedVote, &*iterator, sizeof(m_receivedVote));
	iterator += sizeof(m_receivedVote);

	size_t len = strnlen((const char*)&*iterator, NameMaxSize);
	m_name = std::string((const char*)&*iterator, len);
	iterator += NameMaxSize;

	len = strnlen((const char*)&*iterator, URLMaxSize);
	m_url = std::string((const char*)&*iterator, len);
	iterator += URLMaxSize;

	m_voteRecord.clear();
	for (int i = 0; i < voteToNum; i++)
	{
		Address to;
		uint64_t number;

		memcpy(&to, &*iterator, sizeof(Address));
		iterator += sizeof(Address);
		memcpy(&number, &*iterator, sizeof(uint64_t));
		iterator += sizeof(uint64_t);
		m_voteRecord.emplace(to, number);
	}

	assert(iterator == _bytes.end());

}

void VoteInfo::recordVoteTo(Address const& _address, uint64_t num)
{
	if (m_voteRecord.find(_address) != m_voteRecord.end())
	{
		m_voteRecord[_address] += num;
	}
	else
	{
		m_voteRecord[_address] = num;
	}
}

void VoteInfo::removeZeroVote()
{
	for (auto i = m_voteRecord.begin(), last = m_voteRecord.end(); i != last; ) 
	{
		if (i->second == 0) 
		{
			i = m_voteRecord.erase(i);
		}
		else {
			++i;
		}
	}
}



bool VoteManager::candidateRegister(std::string const& _name, std::string const& _url, Address const& _address, State& _state)
{
	VoteInfo sender(_address);
	sender.load(_state);
	if (sender.m_isCandidate)
	{
		return false;
	}
	if (!sender.m_isCandidate)
	{
		sender.m_name = _name;
		sender.m_url = _url;
		sender.m_isCandidate = true;
		sender.save(_state);
	}
	return true;
}


bool VoteManager::candidateDeregister(Address const& _address, State& _state)
{
	VoteInfo sender(_address);
	sender.load(_state);
	if (!sender.m_isCandidate)
	{
		return false;
	}
	map<Address, VoteInfo> infoMap = VoteInfo::getVoteInfoMap(_state);
	for (auto & p : infoMap)
	{
		//auto x = p.second.m_voteRecord;
		p.second.m_holdVotes += p.second.m_voteRecord[_address];
		p.second.m_voteRecord[_address] = 0;
		p.second.save(_state);
	}
	//in case of changed in the loop(vote to self)
	sender.load(_state);
	sender.m_isCandidate = false;
	sender.m_receivedVote = 0;
	sender.m_name = "";
	sender.m_url = "";
	sender.save(_state);
	return true;
}


bool VoteManager::mortgage(uint64_t _money, Address const& _address, State& _state)
{
	if (_state.balance(_address) >= _money)
	{
		VoteInfo sender(_address);
		sender.load(_state);
		uint64_t voteNumber = _money / VOTES_PRE_ETH;
		uint64_t validMoney = voteNumber * VOTES_PRE_ETH;
		_state.subBalance(_address, validMoney);
		sender.m_holdVotes += voteNumber;
		sender.save(_state);
	}
	return true;
}

bool VoteManager::redeem(uint64_t _votes, Address const& _address, State& _state)
{
	VoteInfo sender(_address);
	sender.load(_state);
	if (sender.m_holdVotes >= _votes)
	{
		sender.m_holdVotes -= _votes;
		_state.addBalance(_address, _votes * VOTES_PRE_ETH);
		sender.save(_state);
	}
	return true;
}

bool VoteManager::send(Address const& _toAddress, uint64_t _votes, Address const& _senderAddress, State& _state)
{
	if (_toAddress == _senderAddress)
	{
		return true;
	}
	VoteInfo sender(_senderAddress);
	sender.load(_state);

	if (sender.m_holdVotes < _votes)
	{
		return false;
	}

	VoteInfo to(_toAddress);
	to.load(_state);

	sender.m_holdVotes -= _votes;
	to.m_holdVotes += _votes;
	sender.save(_state);
	to.save(_state);
	return true;
}


bool VoteManager::vote(Address const& _toAddress, uint64_t _votes, Address const& _senderAddress, State& _state)
{
	VoteInfo sender(_senderAddress);
	sender.load(_state);

	if (sender.m_holdVotes < _votes)
	{
		//not enough votes
		return false;
	}
	if (sender.m_voteRecord.size() >= 15 && sender.m_voteRecord.count(_toAddress)==0)
	{
		//Can't vote to 16 address
		return false;
	}

	// Vote to myself.
	if (_toAddress == _senderAddress)
	{
		if (!sender.m_isCandidate)
		{
			//Illegal candidate.
			return false;
		}

		sender.m_receivedVote += _votes;
	}
	/// Vote to another.
	else
	{
		VoteInfo to(_toAddress);
		to.load(_state);
		if (!to.m_isCandidate)
		{
			return false;
		}
		to.m_receivedVote += _votes;
		to.save(_state);
	}
	sender.m_holdVotes -= _votes;
	sender.recordVoteTo(_toAddress, _votes);
	sender.save(_state);
	return true;
}


bool VoteManager::removeVote(Address const& _toAddress, uint64_t _votes, Address const& _senderAddress, State& _state)
{
	VoteInfo sender(_senderAddress);
	sender.load(_state);
	if (sender.m_voteRecord.find(_toAddress) == sender.m_voteRecord.end())
	{
		return false;
	}
	if (sender.m_voteRecord[_toAddress] < _votes)
	{
		return false;
	}
	// Vote to myself.
	if (_toAddress == _senderAddress)
	{
		if (!sender.m_isCandidate)
		{
			//Illegal candidate.
			return false;
		}

		sender.m_receivedVote -= _votes;
	}
	/// Vote to another.
	else
	{
		VoteInfo to(_toAddress);
		to.load(_state);
		if (!to.m_isCandidate)
		{
			return false;
		}
		to.m_receivedVote -= _votes;
		to.save(_state);
	}
	sender.m_holdVotes += _votes;
	sender.m_voteRecord[_toAddress] -= _votes;
	sender.save(_state);
	return true;

}




ETH_REGISTER_PRECOMPILED(mortgage)(bytesConstRef _in, Address const& _address, State& _state)
{
	uint64_t mortgageAmount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	VoteManager::mortgage(mortgageAmount, _address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(redeem)(bytesConstRef _in, Address const& _address, State& _state)
{
	uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	VoteManager::redeem(voteCount, _address, _state);
	return make_pair(true, bytes());
}



ETH_REGISTER_PRECOMPILED(candidateRegister)(bytesConstRef _in, Address const& _address, State& _state)
{
	//char const* namePtr = (char const*)_in.data();
	//size_t nameLen = strnlen_s(namePtr, VoteInfo::NameMaxSize);
	//
	//char const* urlPtr = namePtr + nameLen + 1;
	//size_t urlLen = strnlen_s(urlPtr, VoteInfo::NameMaxSize);
	if (_in.size() < VoteInfo::NameMinSize + VoteInfo::URLMinSize + 1|| _in.size() > (VoteInfo::NameMaxSize + VoteInfo::URLMaxSize + 1))
	{
		return make_pair(true, bytes());
	}
	string str = _in.toString();
	size_t pos = str.find_first_of('\0');
	if (pos == string::npos)
	{
		return make_pair(true, bytes());
	}
	int nameLen = pos;
	int urlLen = _in.size() - pos - 1;

	if (nameLen > VoteInfo::NameMaxSize || nameLen < VoteInfo::NameMinSize || urlLen > VoteInfo::URLMaxSize || urlLen < VoteInfo::URLMinSize)
	{
		return make_pair(true, bytes());
	}
	VoteManager::candidateRegister(string(str.data(), nameLen), string(str.data() + pos + 1, urlLen), _address, _state);

	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(candidateDeregister)(bytesConstRef _in, Address const& _address, State& _state)
{
	VoteManager::candidateDeregister(_address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(vote)(bytesConstRef _in, Address const& _address, State& _state)
{
	Address toAddress(bytes(_in.begin(), _in.begin() + sizeof(Address)));
	uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin() + sizeof(Address), _in.end()));
	VoteManager::vote(toAddress, voteCount, _address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(removeVote)(bytesConstRef _in, Address const& _address, State& _state)
{
	Address toAddress(bytes(_in.begin(), _in.begin() + sizeof(Address)));
	uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin() + sizeof(Address), _in.end()));
	VoteManager::removeVote(toAddress, voteCount, _address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(send)(bytesConstRef _in, Address const& _address, State& _state)
{
	Address toAddress(bytes(_in.begin(), _in.begin() + sizeof(Address)));
	uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin() + sizeof(Address), _in.end()));
	VoteManager::send(toAddress, voteCount, _address, _state);
	return make_pair(true, bytes());
}


