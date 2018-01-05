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
#include <vector>
#include <map>
#include <string>

#define VOTES_PRE_ETH 10000000000000000
namespace dev
{
namespace eth
{

class UserStorage
{
public:
	static u256 addressToU256(Address const& _address, int _postffix);
	static bytes LoadFixedSizeBtyes(State const& _state, Address const& _storageaddress, Address const& _keyaddress, int _size);
	//static bytes LoadFixedSizeBtyes(State const& _state, map<h256, pair<u256, u256>> const& _storageMap, int _size);

	static void SaveBytes(State& _state, Address const& _storageaddress, Address const& _keyaddress, bytes const& _data);

	//static bytes LoadDPOSVoteBytes(State const& _state, Address const& _storageaddress, Address const& _keyaddress);
	//static void SaveDPOSVoteBytes(State& const _state, Address const& _storageaddress, Address const& _keyaddress, bytes const& _data);
};


class VoteInfo
{
public:
	friend class VoteManager;
	VoteInfo() = default;
	VoteInfo(Address const& _address) :m_address(_address) {}
	void load(State& _state);
	void save(State& _state);
	static std::map<Address, VoteInfo> getVoteInfoMap(State const& _state);

	bool getIsCandidate() const{ return m_isCandidate; }
	void removeZeroVote();
	uint64_t getReceivedVotedNumber() const{ return m_receivedVote; } 
	uint64_t getHoldVoteNumber() const { return m_holdVotes; }
	std::string getName() { return m_name; }
	std::string getURL() { return m_url; }
	
	//Address getVoteTo() const { return ZeroAddress; };

	static const int NameMaxSize = 40;
	static const int URLMaxSize = 40;
	//voteToCount:byte  m_isCandidate:byte 
	//m_receivedVote m_holdVotes :u64
	static const int BasicSize = 2 * sizeof(byte) + 2 * sizeof(uint64_t) + NameMaxSize + URLMaxSize;

protected:
	void initFromBytes(bytes const &_bytes);
	void recordVoteTo(Address const&, uint64_t num);
	Address m_address;                                     ///地址信息

	//byte m_voteToCount;   //投给了几个人票
	bool m_isCandidate;   //是否是生产者

	uint64_t m_receivedVote;//生产者得票数 已经收到的票数
	std::string m_name;	  //昵称
	std::string m_url;	  //URL

	uint64_t m_holdVotes;  //持有的票
	std::map<Address, uint64_t>  m_voteRecord; ///将已分配投票权投给的地址                                

};

class VoteManager
{
public:
	static bool candidateRegister(std::string const& _name, std::string const& _url,Address const& _address, State& _state);
	static bool candidateDeregister(Address const& _address, State& _state);

	static bool mortgage(uint64_t _money, Address const& _address, State& _state);
	static bool redeem(uint64_t _votes, Address const& _address, State& _state);
	static bool send(Address const& _toAddress, uint64_t _votes, Address const& _senderAddress, State& _state);

	static bool vote(Address const& _toAddress,uint64_t _votes, Address const& _senderAddress, State& _state);
	static bool removeVote(Address const& _toAddress, uint64_t _votes, Address const& _senderAddress, State& _state);
};

















}
}
