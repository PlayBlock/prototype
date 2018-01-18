#include <jsonrpccpp/common/exception.h>
#include <libethcore/KeyManager.h>
#include <libweb3jsonrpc/AccountHolder.h>
#include <libethcore/CommonJS.h>
#include <libweb3jsonrpc/JsonHelper.h>
#include <libethereum/Client.h>

#include "Personal.h"

using namespace std;
using namespace dev;
using namespace dev::rpc;
using namespace dev::eth;
using namespace jsonrpc;

Personal::Personal(KeyManager& _keyManager, AccountHolder& _accountHolder, eth::Interface& _eth):
	m_keyManager(_keyManager),
	m_accountHolder(_accountHolder),
	m_eth(_eth)
{
}

std::string Personal::personal_newAccount(std::string const& _password)
{
	KeyPair p = KeyManager::newKeyPair(KeyManager::NewKeyType::NoVanity);
	m_keyManager.import(p.secret(), std::string(), _password, std::string());
	return toJS(p.address());
}

string Personal::personal_sendTransaction(Json::Value const& _transaction, string const& _password)
{
	TransactionSkeleton t;
	try
	{
		t = toTransactionSkeleton(_transaction);
	}
	catch (...)
	{
		BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INVALID_PARAMS));
	}

	if (Secret s = m_keyManager.secret(t.from, [&](){ return _password; }, false))
	{
		// return the tx hash
		return toJS(m_eth.submitTransaction(t, s).first);
	}
	BOOST_THROW_EXCEPTION(JsonRpcException("Invalid password or account."));
}

string Personal::personal_signAndSendTransaction(Json::Value const& _transaction, string const& _password)
{
	return personal_sendTransaction(_transaction, _password);
}

bool Personal::personal_unlockAccount(std::string const& _address, std::string const& _password, int _duration)
{
	return m_accountHolder.unlockAccount(Address(fromHex(_address, WhenError::Throw)), _password, _duration);
}

Json::Value Personal::personal_listAccounts()
{
	return toJson(m_keyManager.accounts());
}

Json::Value Personal::personal_getVote(std::string const& _address)
{
	try
	{
		string _blockNumber = "latest";
		State _state = client()->getState(jsToBlockNumber(_blockNumber));
		map<Address, VoteInfo> voteMap = VoteInfo::getVoteInfoMap(_state);

		Address voteAddress = Address(_address);
		auto it = voteMap.find(voteAddress);
		if (it == voteMap.end())
			return Json::Value(Json::nullValue);

		Json::Value res(Json::objectValue);
		Json::Value subRes(Json::objectValue);

		for (auto i : it->second.getHoldVoteRecord())
		{
			subRes["0x" + i.first.hex()] = to_string(i.second);
		}
		res["address"] = "0x" + voteAddress.hex();
		res["name"] = it->second.getName();
		res["url"] = it->second.getURL();
		res["isCandidate"] = it->second.getIsCandidate();
		res["receivedVote"] = to_string(it->second.getReceivedVotedNumber());
		res["holdVotes"] = to_string(it->second.getHoldVoteNumber());
		res["voteRecord"] = subRes;

		return res;
	}
	catch (...)
	{
		BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INVALID_PARAMS));
	}
}

struct CmpByValue {
	bool operator()(const pair<Address, uint64_t>& lhs, const pair<Address, uint64_t>& rhs) {
		if (lhs.second == rhs.second)
			return lhs.first > rhs.first;
		return lhs.second > rhs.second;
	}
};

Json::Value Personal::personal_getProducer()
{
	try
	{
		string _blockNumber = "latest";
		State _state = client()->getState(jsToBlockNumber(_blockNumber));
		map<Address, uint64_t> producerMap = VoteInfo::getProducerMap(_state);
		map<Address, VoteInfo> voteMap = VoteInfo::getVoteInfoMap(_state);

		vector<pair<Address, uint64_t>> returnVec(producerMap.begin(), producerMap.end());
		sort(returnVec.begin(), returnVec.end(), CmpByValue());

		Json::Value res(Json::objectValue);
		for (int i = 0; i < 15 && i != returnVec.size(); i++)
		{
			Address voteAddress = std::get<0>(returnVec[i]);
			VoteInfo voteInfo= voteMap[voteAddress];

			Json::Value subRes(Json::objectValue);
			subRes["address"] = "0x" + voteAddress.hex();
			subRes["name"] = voteInfo.getName();
			subRes["url"] = voteInfo.getURL();
			subRes["receivedVote"] = to_string(voteInfo.getReceivedVotedNumber());

			res[to_string(i + 1)] = subRes;
		}

		return res;
	}
	catch (...)
	{
		BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INVALID_PARAMS));
	}
}
