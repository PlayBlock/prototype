#include <jsonrpccpp/common/exception.h>
#include <libethcore/KeyManager.h>
#include <libweb3jsonrpc/AccountHolder.h>
#include <libethcore/CommonJS.h>
#include <libweb3jsonrpc/JsonHelper.h>
#include <libethereum/Client.h>

#include "Personal.h"
#include <utils/json_spirit/json_spirit_value.h>
#include <utils/json_spirit/json_spirit_reader_template.h>
#include <utils/json_spirit/json_spirit_writer_template.h>

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
		State _state = m_eth.getState(jsToBlockNumber(_blockNumber));
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
		State _state = m_eth.getState(jsToBlockNumber(_blockNumber));
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

			res[to_string(i)] = subRes;
		}

		return res;
	}
	catch (...)
	{
		BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INVALID_PARAMS));
	}
}

enum Type
{
	Dpos,
	Pow,
};

string Personal::personal_setConfigFile(Json::Value const& _config)
{
	Address address;
	string password;
	Type type;
	Secret secret;
	try
	{
		if (_config["address"].empty())
			BOOST_THROW_EXCEPTION(JsonRpcException("Lack address field!"));
		if (_config["password"].empty())
			BOOST_THROW_EXCEPTION(JsonRpcException("Lack password field!"));
		if (_config["type"].empty())
			BOOST_THROW_EXCEPTION(JsonRpcException("Lack type field!"));

		string _address = _config["address"].asString();
		address = Address(_address);

		password = _config["password"].asString();

		string _type = _config["type"].asString();
		if (_type.compare(string("dpos")) == 0)
		{
			type = Type::Dpos;
		}
		else if (_type.compare(string("pow")) == 0)
		{
			type = Type::Pow;
		}
		else
		{
			BOOST_THROW_EXCEPTION(JsonRpcException("Type error!(""dpos"", ""pow"" only)"));
		}
	}
	catch (JsonRpcException&)
	{
		throw;
	}
	catch (...)
	{
		BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INVALID_PARAMS));
	}

	try
	{
		secret = m_keyManager.secret(address, [&]() { return password; });
	}
	catch (...)
	{
		BOOST_THROW_EXCEPTION(JsonRpcException("Invalid password or account."));
	}

	try
	{
		string filePath(boost::filesystem::current_path().string());
		string s = contentsString(filePath + "/config.json");
		json_spirit::mValue v;
		json_spirit::read_string(s, v);
		json_spirit::mObject& json_config = v.get_obj();

		/// privateKeys
		if (!json_config.count("privateKeys"))
		{
			json_spirit::mObject key;
			key["0x" + address.hex()] = toHex(secret.ref());
			json_config["privateKeys"] = key;
		}
		else
		{
			json_spirit::mObject& privateKeys = json_config["privateKeys"].get_obj();
			privateKeys["0x" + address.hex()] = toHex(secret.ref());
		}

		/// params
		if (!json_config.count("params"))
		{
			BOOST_THROW_EXCEPTION(JsonRpcException("Config file no ""params"" field!"));
		}
		else
		{
			json_spirit::mObject& params = json_config["params"].get_obj();
			/// producerAccounts
			if (!params.count("producerAccounts"))
			{
				json_spirit::mArray producerAccounts;
				producerAccounts.push_back("0x" + address.hex());
				params["producerAccounts"] = producerAccounts;
			}
			else
			{
				json_spirit::mArray& producerAccounts = params["producerAccounts"].get_array();
				auto res = find(producerAccounts.begin(), producerAccounts.end(), "0x" + address.hex());
				if (res == producerAccounts.end())
				{
					producerAccounts.push_back("0x" + address.hex());
				}
			}

			///powWorker
			if (type == Type::Pow)
			{
				params["powWorker"] = "0x" + address.hex();
			}
		}

		writeFile(filePath + "/config.json", asBytes(json_spirit::write_string(v, true)));

		return "Success!";
	}
	catch (const std::exception&)
	{
		BOOST_THROW_EXCEPTION(JsonRpcException("File IO error."));
	}
}
