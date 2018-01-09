#include <libethcore/Precompiled.h>
#include <libethereum/State.h>

#include <libdevcore/Log.h>
#include <libdevcore/SHA3.h>
#include <libdevcrypto/Hash.h>
#include <libdevcrypto/Common.h>
#include <libdevcrypto/LibSnark.h>
#include <libethcore/Common.h>

#include <libevm/Vote.h>
using namespace std;
using namespace dev;
using namespace dev::eth;
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
	VoteDelegate::vote(toaddress, _address, _state);
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
	VoteDelegate::assign(voteCount, _address, _state);
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
