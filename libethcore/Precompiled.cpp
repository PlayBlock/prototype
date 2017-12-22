/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
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
/** @file Precompiled.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Precompiled.h"
#include <libdevcore/Log.h>
#include <libdevcore/SHA3.h>
#include <libdevcrypto/Hash.h>
#include <libdevcrypto/Common.h>
#include <libdevcrypto/LibSnark.h>
#include <libethcore/Common.h>

#include <libevm/Vote.h>
//#include <libethereum/chainbase.hpp>
//#include <libethereum/State.h>

using namespace std;
using namespace dev;
using namespace dev::eth;



PrecompiledRegistrar* PrecompiledRegistrar::s_this = nullptr;

PrecompiledExecutor const& PrecompiledRegistrar::executor(std::string const& _name)
{
	if (!get()->m_execs.count(_name))
		BOOST_THROW_EXCEPTION(ExecutorNotFound());
	return get()->m_execs[_name];
}

PrecompiledPricer const& PrecompiledRegistrar::pricer(std::string const& _name)
{
	if (!get()->m_pricers.count(_name))
		BOOST_THROW_EXCEPTION(PricerNotFound());
	return get()->m_pricers[_name];
}

namespace
{

ETH_REGISTER_PRECOMPILED(ecrecover)(bytesConstRef _in, Address const& _address, State& _state)
{
	struct
	{
		h256 hash;
		h256 v;
		h256 r;
		h256 s;
	} in;

	memcpy(&in, _in.data(), min(_in.size(), sizeof(in)));

	h256 ret;
	u256 v = (u256)in.v;
	if (v >= 27 && v <= 28)
	{
		SignatureStruct sig(in.r, in.s, (byte)((int)v - 27));
		if (sig.isValid())
		{
			try
			{
				if (Public rec = recover(sig, in.hash))
				{
					ret = dev::sha3(rec);
					memset(ret.data(), 0, 12);
					return {true, ret.asBytes()};
				}
			}
			catch (...) {}
		}
	}
	return {true, {}};
}

ETH_REGISTER_PRECOMPILED(sha256)(bytesConstRef _in, Address const& _address, State& _state)
{
	return {true, dev::sha256(_in).asBytes()};
}

ETH_REGISTER_PRECOMPILED(ripemd160)(bytesConstRef _in, Address const& _address, State& _state)
{
	return {true, h256(dev::ripemd160(_in), h256::AlignRight).asBytes()};
}

ETH_REGISTER_PRECOMPILED(identity)(bytesConstRef _in, Address const& _address, State& _state)
{
	return {true, _in.toBytes()};
}

// Parse _count bytes of _in starting with _begin offset as big endian int.
// If there's not enough bytes in _in, consider it infinitely right-padded with zeroes.
bigint parseBigEndianRightPadded(bytesConstRef _in, bigint const& _begin, bigint const& _count)
{
	if (_begin > _in.count())
		return 0;
	assert(_count <= numeric_limits<size_t>::max() / 8); // Otherwise, the return value would not fit in the memory.

	size_t const begin{_begin};
	size_t const count{_count};

	// crop _in, not going beyond its size
	bytesConstRef cropped = _in.cropped(begin, min(count, _in.count() - begin));

	bigint ret = fromBigEndian<bigint>(cropped);
	// shift as if we had right-padding zeroes
	assert(count - cropped.count() <= numeric_limits<size_t>::max() / 8);
	ret <<= 8 * (count - cropped.count());

	return ret;
}

ETH_REGISTER_PRECOMPILED(modexp)(bytesConstRef _in, Address const& _address, State& _state)
{
	bigint const baseLength(parseBigEndianRightPadded(_in, 0, 32));
	bigint const expLength(parseBigEndianRightPadded(_in, 32, 32));
	bigint const modLength(parseBigEndianRightPadded(_in, 64, 32));
	assert(modLength <= numeric_limits<size_t>::max() / 8); // Otherwise gas should be too expensive.
	assert(baseLength <= numeric_limits<size_t>::max() / 8); // Otherwise, gas should be too expensive.
	if (modLength == 0 && baseLength == 0)
		return {true, bytes{}}; // This is a special case where expLength can be very big.
	assert(expLength <= numeric_limits<size_t>::max() / 8);

	bigint const base(parseBigEndianRightPadded(_in, 96, baseLength));
	bigint const exp(parseBigEndianRightPadded(_in, 96 + baseLength, expLength));
	bigint const mod(parseBigEndianRightPadded(_in, 96 + baseLength + expLength, modLength));

	bigint const result = mod != 0 ? boost::multiprecision::powm(base, exp, mod) : bigint{0};

	size_t const retLength(modLength);
	bytes ret(retLength);
	toBigEndian(result, ret);

	return {true, ret};
}

namespace
{
	bigint expLengthAdjust(bigint const& _expOffset, bigint const& _expLength, bytesConstRef _in)
	{
		if (_expLength <= 32)
		{
			bigint const exp(parseBigEndianRightPadded(_in, _expOffset, _expLength));
			return exp ? msb(exp) : 0;
		}
		else
		{
			bigint const expFirstWord(parseBigEndianRightPadded(_in, _expOffset, 32));
			size_t const highestBit(expFirstWord ? msb(expFirstWord) : 0);
			return 8 * (_expLength - 32) + highestBit;
		}
	}

	bigint multComplexity(bigint const& _x)
	{
		if (_x <= 64)
			return _x * _x;
		if (_x <= 1024)
			return (_x * _x) / 4 + 96 * _x - 3072;
		else
			return (_x * _x) / 16 + 480 * _x - 199680;
	}
}

ETH_REGISTER_PRECOMPILED_PRICER(modexp)(bytesConstRef _in)
{
	bigint const baseLength(parseBigEndianRightPadded(_in, 0, 32));
	bigint const expLength(parseBigEndianRightPadded(_in, 32, 32));
	bigint const modLength(parseBigEndianRightPadded(_in, 64, 32));

	bigint const maxLength(max(modLength, baseLength));
	bigint const adjustedExpLength(expLengthAdjust(baseLength + 96, expLength, _in));

	return multComplexity(maxLength) * max<bigint>(adjustedExpLength, 1) / 20;
}

ETH_REGISTER_PRECOMPILED(alt_bn128_G1_add)(bytesConstRef _in, Address const& _address, State& _state)
{
	return dev::crypto::alt_bn128_G1_add(_in);
}

ETH_REGISTER_PRECOMPILED(alt_bn128_G1_mul)(bytesConstRef _in, Address const& _address, State& _state)
{
	return dev::crypto::alt_bn128_G1_mul(_in);
}

ETH_REGISTER_PRECOMPILED(alt_bn128_pairing_product)(bytesConstRef _in, Address const& _address, State& _state)
{
	return dev::crypto::alt_bn128_pairing_product(_in);
}

ETH_REGISTER_PRECOMPILED_PRICER(alt_bn128_pairing_product)(bytesConstRef _in)
{
	return 100000 + (_in.size() / 192) * 80000;
}
class dev::eth::State;
ETH_REGISTER_PRECOMPILED(mortgage)(bytesConstRef _in, Address const& _address, State& _state)
{
	//uint64_t balance = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	//if (_state.balance(_address) >= balance)
	//{
	//	unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//	unordered_map<u256, u256> mapChange;
	//	Vote vote(voteMap, mapChange, _address);
	//	vote.load();
	//	uint64_t validBalance = balance / VOTES_PRE_ETH * VOTES_PRE_ETH;
	//	_state.subBalance(_address, validBalance);
	//	vote.mortgage(validBalance);
	//	vote.save();
	//	for (auto i : mapChange)
	//	{
	//		_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//	}
	//}

	uint64_t amount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	VoteDelegate::mortgage(amount, _address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(redeem)(bytesConstRef _in, Address const& _address, State& _state)
{
	//uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	//unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//unordered_map<u256, u256> mapChange;
	//Vote vote(voteMap, mapChange, _address);
	//vote.load();
	//if (vote.redeem(voteCount))
	//{
	//	_state.addBalance(_address, voteCount * VOTES_PRE_ETH);
	//}
	//vote.save();

	//for (auto i : mapChange)
	//{
	//	_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//}


	uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	VoteDelegate::redeem(voteCount, _address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(candidateRegister)(bytesConstRef _in, Address const& _address, State& _state)
{
	//unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//unordered_map<u256, u256> mapChange;
	//Vote vote(voteMap, mapChange, _address);
	//vote.load();
	//vote.candidateRegister();
	//vote.save();

	//for (auto i : mapChange)
	//{
	//	_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//}

	VoteDelegate::candidateRegister(_address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(candidateDeregister)(bytesConstRef _in, Address const& _address, State& _state)
{
	//unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//unordered_map<u256, u256> mapChange;
	//Vote vote(voteMap, mapChange, _address);
	//vote.load();
	//vote.candidateDeregister();
	//vote.save();

	//for (auto i : mapChange)
	//{
	//	_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//}


	VoteDelegate::candidateDeregister(_address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(vote)(bytesConstRef _in, Address const& _address, State& _state)
{
	//Address address(bytes(_in.begin(), _in.end()));
	//unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//unordered_map<u256, u256> mapChange;
	//Vote vote(voteMap, mapChange, _address);
	//vote.load();
	//vote.vote(address);
	//vote.save();

	//for (auto i : mapChange)
	//{
	//	_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//}


	Address toaddress(bytes(_in.begin(), _in.end()));
	VoteDelegate::vote(toaddress,_address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(removeVote)(bytesConstRef _in, Address const& _address, State& _state)
{
	//unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//unordered_map<u256, u256> mapChange;
	//Vote vote(voteMap, mapChange, _address);
	//vote.load();
	//vote.removeVote();
	//vote.save();

	//for (auto i : mapChange)
	//{
	//	_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//}

	Address toaddress(bytes(_in.begin(), _in.end()));
	VoteDelegate::removeVote(_address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(send)(bytesConstRef _in, Address const& _address, State& _state)
{
	//Address address(bytes(_in.begin(), _in.begin() + sizeof(Address)));
	//uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin() + sizeof(Address), _in.end()));
	//unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//unordered_map<u256, u256> mapChange;
	//Vote vote(voteMap, mapChange, _address);
	//vote.load();
	//vote.send(address, voteCount);
	//vote.save();

	//for (auto i : mapChange)
	//{
	//	_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//}

	Address toAddress(bytes(_in.begin(), _in.begin() + sizeof(Address)));
	uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin() + sizeof(Address), _in.end()));
	VoteDelegate::send(toAddress, voteCount, _address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(assign)(bytesConstRef _in, Address const& _address, State& _state)
{
	//uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	//unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//unordered_map<u256, u256> mapChange;
	//Vote vote(voteMap, mapChange, _address);
	//vote.load();
	//vote.assign(voteCount);
	//vote.save();

	//for (auto i : mapChange)
	//{
	//	_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//}

	uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	VoteDelegate::assign(voteCount,_address, _state);
	return make_pair(true, bytes());
}

ETH_REGISTER_PRECOMPILED(deAssign)(bytesConstRef _in, Address const& _address, State& _state)
{
	//uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	//unordered_map<u256, u256> voteMap = _state.storage(Address("0000000000000000000000000000000000000005"));
	//unordered_map<u256, u256> mapChange;
	//Vote vote(voteMap, mapChange, _address);
	//vote.load();
	//vote.deAssign(voteCount);
	//vote.save();

	//for (auto i : mapChange)
	//{
	//	_state.setStorage(Address("0000000000000000000000000000000000000005"), i.first, i.second);
	//

	uint64_t voteCount = boost::lexical_cast<uint64_t>(string(_in.begin(), _in.end()));
	VoteDelegate::deAssign(voteCount, _address, _state);
	return make_pair(true, bytes());
}


}
