#include "DposTests.h"
#include <boost/range/algorithm/find.hpp>
using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

namespace dev {
namespace test {



DposTestClient::DposTestClient() :
	m_bc(TestBlockChain::defaultDposGenesisBlock()),
	_producer_plugin(make_shared<class producer_plugin>(m_bc.getInterface())),
	_chain(_producer_plugin->get_chain_controller())
{

	_producer_plugin->get_chain_controller().setStateDB(m_bc.testGenesis().state().db());
	//_producer_plugin->get_chain_controller().init_global_property();
	m_bc.interfaceUnsafe().setProducer(_producer_plugin);

	init_accounts();
}

void DposTestClient::init_accounts()
{
	// init genesis account
	genesisAccount.nonce = 0;

	// read all prikey-address pair
	// transfer 1 eth from genesis account to all these addresses
	json_spirit::mValue v;
	string keys_file_path = dev::test::getFolder(__FILE__) + "/DposTests/";
	string s = contentsString(keys_file_path + "address-keys.json");
	json_spirit::read_string(s, v);
	json_spirit::mObject keys = v.get_obj();
	int i = 0;
	for (const auto& key : keys)
	{
		if (i++ > config::BlocksPerRound + 2)
			break;

		Account account{ key.first , key.second.get_str() , 0 };

		//add key into accounts
		m_accounts.push_back(std::move(account));

		//add address-privatekey pair into _private_keys
		_private_keys[AccountName(key.first)] = private_key(key.second.get_str());
	}

	auto& params = m_bc.getInterface().chainParams();
	for (auto& key : params.privateKeys)
	{
		_private_keys.insert(key);
	}

	//give 1 eth to accounts
	for (const auto& i : m_accounts)
	{
		transfer_eth(genesisAccount, i, u256(1000000000000000000));
	}

	produce_blocks(config::BlocksPerRound);

	//test balance
	//Block block(m_bc.interface(), m_bc.testGenesis().state().db());
	//block.populateFromChain(m_bc.interface(),m_bc.interface().currentHash());
	//for (auto i : m_accounts)
	//{
	//	cout<< block.balance(Address(i.second.address)) <<endl;
	//}
}

const private_key& DposTestClient::get_private_key(const AccountName& address) const
{
	return _private_keys.at(address);
}

void DposTestClient::produce_blocks(uint32_t count)
{
	for (int i = 0; i < count; i++)
	{
		auto slot = 1;
		auto producer = _chain.get_scheduled_producer(slot);
		while (producer == AccountName())
			producer = _chain.get_scheduled_producer(++slot);

		//auto producer =  _chain.get_scheduled_producer(slot);
		auto& private_key = get_private_key(producer);
		m_working.dposMine(m_bc, _chain.get_slot_time(slot), producer, private_key);
		m_bc.addBlock(m_working);
		m_working = TestBlock();
	}
}

string DposTestClient::getWAVMData(string function, Address address)
{
	if (address == Address())
	{
		bytes func = bytes(function.begin(), function.end());
		return toHex(func);
	}
	bytes func = bytes(function.begin(), function.end());
	bytes addr = address.asBytes();
	bytes total = func;
	total.push_back('\0');
	total += addr;
	return toHex(total);
}

void DposTestClient::mortgage_eth(Account& _from, uint64_t balance)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "0000000000000000000000000000000000000022";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = toHex(boost::lexical_cast<string>(balance));
	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}

void DposTestClient::redeem_eth(Account& _from, uint64_t voteCount)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "0000000000000000000000000000000000000023";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = toHex(boost::lexical_cast<string>(voteCount));
	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}

void DposTestClient::assign(Account& _from, uint64_t voteCount)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "0000000000000000000000000000000000000028";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = toHex(boost::lexical_cast<string>(voteCount));
	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}

void DposTestClient::deAssign(Account& _from, uint64_t voteCount)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "0000000000000000000000000000000000000029";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = toHex(boost::lexical_cast<string>(voteCount));
	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}

void DposTestClient::make_producer(Account& _from)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "0000000000000000000000000000000000000024";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = "0x610062";

	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}

void DposTestClient::unmake_producer(Account& _from)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "0000000000000000000000000000000000000025";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = "";
	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}
void DposTestClient::make_pow_transaction(Account& _from, ETIProofOfWork::Solution& _sol)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "000000000000000000000000000000000000002c";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = toString(_sol.op._saveImpl());
	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}
void DposTestClient::make_pow_producer(Account& _from)
{
	BlockHeader bh = m_bc.getInterface().info();

	static Secret priviteKey = Secret(_from.secret);
	static AccountName workerAccount(_from.address);
	// 注册回调函数，等待miners找到解后调用
	Notified<bool> sealed(false);

	std::map<dev::h256, std::pair<dev::u256, dev::u256>> map;
	std::unordered_map<dev::u256, dev::u256> mapChange;
	POW_Operation op(map, mapChange, Address());
	ETIProofOfWork::Solution sol = { op };

	sealEngine()->onETISealGenerated([&](const ETIProofOfWork::Solution& _sol) {
		sol.op = _sol.op;
		sealed = true;
	});
	
	auto tid = std::this_thread::get_id();
	static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

	uint64_t tryNonce = s_eng();
	uint64_t start = tryNonce;
	uint64_t nonce = start;// +thread_num;
	auto target = _producer_plugin->get_chain_controller().get_pow_target();

	ETIProofOfWork::WorkPackage newWork{ bh.hash(), priviteKey, workerAccount, nonce, target };

	// 给miners发送新的任务
	sealEngine()->newETIWork(newWork);
	sealed.waitNot(false);
	sealEngine()->onETISealGenerated([](const ETIProofOfWork::Solution&) {});
	make_pow_transaction(_from,sol);

}

void DposTestClient::send(Account& _from, const Account& on, uint64_t voteCount)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "000000000000000000000000000000000000002a";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = on.address + toHex(boost::lexical_cast<string>(voteCount));
	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}

void DposTestClient::approve_producer(Account& voter, const Account& on, uint64_t voteCount)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "0000000000000000000000000000000000000026";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(voter.nonce);
	string data = on.address + toHex(boost::lexical_cast<string>(voteCount));
	string secretKey = voter.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	voter.nonce++;
}

void DposTestClient::unapprove_producer(Account& voter, const Account& on, uint64_t voteCount)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = "0000000000000000000000000000000000000027";
	string value = "0x0";
	string nonce = boost::lexical_cast<string>(voter.nonce);
	string data = on.address + toHex(boost::lexical_cast<string>(voteCount));
	string secretKey = voter.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	voter.nonce++;
}

void DposTestClient::transfer_eth(Account& _from, const Account& _to, const u256& _value)
{
	string gasLimit = "0xc350";
	string gasPrice = "0x04a817c800";
	string to = _to.address;
	string value = _value.str();
	string nonce = boost::lexical_cast<string>(_from.nonce);
	string data = "0x";
	string secretKey = _from.secret;

	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", nonce));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", secretKey));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}

const chain::ProducerRound& DposTestClient::get_active_producers() {
	if (_producer_plugin == nullptr)
	{
		throw std::runtime_error("uninit producer plugin");
	}
	
	return _producer_plugin->get_chain_controller().get_global_properties().active_producers;
	       
}

Accounts& DposTestClient::get_accounts()
{
	return m_accounts;
}

const std::map<Address, VoteInfo> DposTestClient::get_votes()
{
	return _producer_plugin->get_chain_controller().get_votes();
}

const std::map<Address, uint64_t> DposTestClient::get_all_producers()
{
	std::map<Address, VoteInfo> voteInfo = _producer_plugin->get_chain_controller().get_votes();
	std::map<Address, uint64_t> ret;
	for (auto i : voteInfo)
	{
		ret[i.first] = i.second.getReceivedVotedNumber();
	}
	return ret;
}

u256 DposTestClient::balance(const Address& _address) const
{
	Block block(m_bc.getInterface(), m_bc.testGenesis().state().db());
	block.populateFromChain(m_bc.getInterface(),m_bc.getInterface().currentHash());
	return block.balance(_address);
}

bytes DposTestClient::code(const Address& _address) const
{
	Block block(m_bc.getInterface(), m_bc.testGenesis().state().db());
	block.populateFromChain(m_bc.getInterface(), m_bc.getInterface().currentHash());
	return block.code(_address);
}

u256 DposTestClient::storage(Address const & _contract, u256 const & _memory) const
{
	Block block(m_bc.getInterface(), m_bc.testGenesis().state().db());
	block.populateFromChain(m_bc.getInterface(), m_bc.getInterface().currentHash());
	return block.storage(_contract, _memory);
}

void DposTestClient::sendTransaction(const string & gasLimit, const string & gasPrice, const string & to, const string & value, const string & data, Account& _from)
{
	json_spirit::mObject obj;
	obj.emplace(make_pair("gasLimit", gasLimit));
	obj.emplace(make_pair("gasPrice", gasPrice));
	obj.emplace(make_pair("to", to));
	obj.emplace(make_pair("value", value));
	obj.emplace(make_pair("nonce", boost::lexical_cast<string>(_from.nonce)));
	obj.emplace(make_pair("data", data));
	obj.emplace(make_pair("secretKey", _from.secret));
	TestTransaction tx(obj);
	m_working.addTransaction(tx);

	_from.nonce++;
}

void DposTestClient::sendTransaction(TransactionSkeleton const & _ts, Secret const & _s)
{
	Transaction t(_ts, _s);

	TestTransaction tx(t);
	m_working.addTransaction(tx);
}

void DposTestClient::setGasLimit(u256 const & _v)
{
	cout << "_v.str(): " << _v.str() << endl;
	BlockHeader bh(m_working.blockHeader());
	bh.setGasLimit(_v);
	m_working.setBlockHeader(bh);
}


fs::path DposBlockTestSuite::suiteFolder() const
{
	return "DposBlockTests";
}
fs::path DposBlockTestSuite::suiteFillerFolder() const
{
	return "DposBlockTestsFiller";
}

class DpTestFixture {
public:
	DpTestFixture()
	{
		DposBlockTestSuite suite;
		string const& casename = boost::unit_test::framework::current_test_case().p_name;

		suite.runAllTestsInFolder(casename);
	}
};



BOOST_FIXTURE_TEST_SUITE(DposTestsSuite, TestOutputHelperFixture)
BOOST_AUTO_TEST_CASE(dtMakeProducer)
{
	g_logVerbosity = 13;

	// make blockchain
	DposTestClient client;

	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& account = client.get_accounts()[0];

	// get all producers that have registered	
	auto all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account.address)) == all_producers.end());

	// make producer
	client.make_producer(account);

	client.produce_blocks();
	all_producers = client.get_all_producers();

	// check this account has become a producer
	BOOST_REQUIRE(all_producers.find(types::AccountName(account.address)) != all_producers.end());

	// check every producer's vote is 0
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account.address)], 0);

	// unmake producer
	client.unmake_producer(account);

	// check this account isn't producer anymore
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account.address)) == all_producers.end());
}

BOOST_AUTO_TEST_CASE(dtMortgage)
{
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();
	auto balance = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance < u256(500000000000000000));// Account 0's balnce should reduce according to mortgage.
	BOOST_REQUIRE(balance >= u256(500000000000000000) - u256(1000000000000000));
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 50); // Mortage eth to have 50 votes.

	client.redeem_eth(account[0], 5);// Redeem 5 votes with 0.05 eth.
	client.produce_blocks();
	balance = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance < u256(550000000000000000));// Account 0's balnce should increase according to mortgage.
	BOOST_REQUIRE(balance >= u256(550000000000000000) - u256(1000000000000000) * 2);
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 45); // Redeem eth reduce to 45 votes.

	client.redeem_eth(account[0], 50);// Redeem 50 votes with 0.5 eth.
	client.produce_blocks();
	balance = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance < u256(550000000000000000));// Account 0's balnce shouldn't increase without enough votes.
	BOOST_REQUIRE(balance >= u256(550000000000000000) - u256(1000000000000000) * 3);
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 45); // Redeem eth reduce to 45 votes.

	client.redeem_eth(account[1], 5);// Redeem 5 votes with 0.05 eth.
	client.produce_blocks();
	balance = client.balance(Address(account[1].address));
	BOOST_REQUIRE(balance < u256(1000000000000000000));// Account 1's balnce shouldn't increase without votes.
	BOOST_REQUIRE(balance >= u256(1000000000000000000) - u256(1000000000000000) * 1);

	client.mortgage_eth(account[2], 1100000000000000000);// Account 2 mortage 1.1 eth with 110 votes, but have no enough eth.
	client.mortgage_eth(account[3], 1000000000000000000);// Account 3 mortage 1.0 eth with 100 votes,but have no enough eth.
	client.mortgage_eth(account[4], 159000000000000000);// Account 3 mortage 0.159 eth with 15 votes.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getHoldVoteNumber(), 0); // Mortage false, have no votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[3].address)].getHoldVoteNumber(), 0); // Mortage false, have no votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[4].address)].getHoldVoteNumber(), 15); // Mortage false, have no votes.
	balance = client.balance(Address(account[2].address));
	BOOST_REQUIRE(balance < u256(1000000000000000000));// Account 0's balnce should't change.
	BOOST_REQUIRE(balance >= u256(1000000000000000000) - u256(1000000000000000) * 1);
	balance = client.balance(Address(account[3].address));
	BOOST_REQUIRE(balance < u256(1000000000000000000));// Account 0's balnce should't change.
	BOOST_REQUIRE(balance >= u256(1000000000000000000) - u256(1000000000000000) * 1);
	balance = client.balance(Address(account[4].address));
	BOOST_REQUIRE(balance < u256(850000000000000000));// Account 0's balnce should reduce 0.15 eth rather than 0.159 eth.
	BOOST_REQUIRE(balance >= u256(850000000000000000) - u256(1000000000000000) * 1);


	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks();
	balance = client.balance(Address(account[5].address));
	BOOST_REQUIRE(balance < u256(100000000000000000));// Account 0's balnce should reduce according to mortgage.
	BOOST_REQUIRE(balance >= u256(100000000000000000) - u256(1000000000000000) * 9);
}
/*
BOOST_AUTO_TEST_CASE(dtAssign)
{
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	client.mortgage_eth(account[0], 500000000000000000);
	client.assign(account[0], 40); // Account 0 assign 40 votes.
	client.produce_blocks();
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getUnAssignNumber(), 10); // 10 unassign votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 40); // 40 assign votes.

	client.deAssign(account[0], 5); // Account 0 deassign 5 votes.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getUnAssignNumber(), 15); // 15 unassign votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 35); // 35 assign votes.

	client.assign(account[0], 50); // Account 0 assign 50 votes. Error happened.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getUnAssignNumber(), 15); // 15 unassign votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 35); // 35 assign votes.

	client.deAssign(account[0], 50); // Account 0 assign 50 votes. Error happened.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getUnAssignNumber(), 15); // 15 unassign votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 35); // 35 assign votes.
}
*/
BOOST_AUTO_TEST_CASE(dtApproveProducer)
{
	//g_logVerbosity = 13;

	// make blockchain
	DposTestClient client;

	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	// make producer
	client.make_producer(account[0]);//nonce = 0

	// get all producers that have registered	
	client.produce_blocks();
	auto all_producers = client.get_all_producers();


	// check this producer's vote is 0
	BOOST_REQUIRE_EQUAL(all_producers.size(), 1);
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 0);

	auto balance = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance < u256(1000000000000000000));
	BOOST_REQUIRE(balance >= u256(1000000000000000000) - u256(1000000000000000));

	// Mortage 0.5 eth eth with 50 votes.
	client.mortgage_eth(account[0], 500000000000000000);//nonce = 1
	client.produce_blocks();
	balance = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance < u256(500000000000000000));// Account 0's balnce should reduce according to mortgage.
	BOOST_REQUIRE(balance >= u256(500000000000000000) - u256(1000000000000000) * 2);

	// Redeem 5 votes with 0.05 eth.
	client.redeem_eth(account[0], 5);//nonce = 2
	client.produce_blocks();
	balance = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance < u256(550000000000000000));// Account 0's balnce should increase according to mortgage.
	BOOST_REQUIRE(balance >= u256(550000000000000000) - u256(1000000000000000) * 3);

	//// Assign 10 votes for vote.
	//client.assign(account[0], 10);//nonce = 3
	//client.produce_blocks();

	//// DeAssign 2 votes. Account 0 have 8 votes now.
	//client.deAssign(account[0], 2);//nonce = 4
	//client.produce_blocks();

	// approve
	client.approve_producer(account[0], account[0], 8);//nonce = 5

	// check this producer's vote is 1
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers.size(), 1);
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 8);// Account 0 should have 8 votes from himself.

	// unapprove
	client.unapprove_producer(account[0], account[0], 8);//nonce = 6
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 0);

	//// Account 1 mortage 0.4 eth with 40 votes.
	//client.mortgage_eth(account[1], 400000000000000000);//nonce = 1
	//client.produce_blocks();
	//balance = client.balance(Address(account[1].address));
	//BOOST_REQUIRE(balance < u256(600000000000000000));
	//BOOST_REQUIRE(balance >= u256(600000000000000000) - u256(1000000000000000) * 2);

	//client.unmake_producer(account[0]);// Account 0 unmake productor.
	//client.produce_blocks();
	//client.assign(account[1], 10);// Account 1 assign 10 votes.
	//client.produce_blocks();
	//client.approve_producer(account[1], account[0]);// Account 1 vote to account 0 who isn't productor.
	//client.produce_blocks();
	//all_producers = client.get_all_producers();
	//BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 0);// Account 0 should have zero votes.

	//client.make_producer(account[0]);// Account 0 make productor.
	//client.produce_blocks();
	//client.approve_producer(account[1], account[0]);// Account 1 vote to account 0 who isn productor this time.
	//client.produce_blocks();
	//all_producers = client.get_all_producers();
	//BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 10);// Account 0 should have 10 votes from account 1;.

	//client.assign(account[1], 20);// Account 1 assign 20 votes.
	//client.produce_blocks();
	//all_producers = client.get_all_producers();
	//BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 30);// Account 0 should have 30 votes(20 votes increase).

	//client.deAssign(account[1], 5);// Account 1 deassign 5 votes.
	//client.produce_blocks();
	//all_producers = client.get_all_producers();
	//BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 25);// Account 0 should have 25 votes(5 votes reduce).

	//client.approve_producer(account[0], account[0]);// Account 0 vote himself.
	//client.produce_blocks();
	//all_producers = client.get_all_producers();
	//BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 33);// Account 0 should have 33 votes(8 votes increase).

	//client.send(account[1], account[0], 2);// Account 1 send 2 unassign votes to account 0 as assign votes. Voted number increased immediately.
	//client.produce_blocks();
	//all_producers = client.get_all_producers();
	//BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 35);// Account 0 should have 35 votes(2 votes increase).

	//client.redeem_eth(account[0], 50 - 5 - 10 + 2 + 2); //Account 0 try to redeem 39 votes.
	//client.produce_blocks();
	//auto all_votes = client.get_votes();
	//BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getUnAssignNumber(), 37); // Redeem false. Can't redeen send votes.
	//balance = client.balance(Address(account[0].address));
	//BOOST_REQUIRE(balance < u256(550000000000000000));// Account 0's balnce should unchange.
	//BOOST_REQUIRE(balance >= u256(550000000000000000) - u256(1000000000000000) * 11);

	//client.redeem_eth(account[0], 50 - 5 - 10 + 2); //Account 0 try to redeem 37 votes.
	//client.produce_blocks();
	//all_votes = client.get_votes();
	//BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getUnAssignNumber(), 0); // Redeem success.
	//balance = client.balance(Address(account[0].address));
	//BOOST_REQUIRE(balance < u256(920000000000000000));// Account 0's balnce should increase according to redeem.
	//BOOST_REQUIRE(balance >= u256(920000000000000000) - u256(1000000000000000) * 12);
}
/*
BOOST_AUTO_TEST_CASE(dtApproveProducer2)
{
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	client.mortgage_eth(account[0], 500000000000000000);
	client.assign(account[0], 40); // Account 0 assign 40 votes.
	client.produce_blocks();
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getUnAssignNumber(), 10); // 10 unassign votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 40); // 40 assign votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 0); // 0 votes number.

	client.approve_producer(account[0], account[1]); // Account 0 votes to account 1. Error happened.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 0); // 0 votes number.

	client.make_producer(account[1]);
	client.approve_producer(account[0], account[1]); // Account 0 votes to account 1.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 40); // 40 votes number.

	client.assign(account[0], 5); // Account 0 deassign 5 votes.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 45); // 45 votes number.

	client.deAssign(account[0], 20); // Account 0 deassign 20 votes.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 25); // 25 votes number.

	client.mortgage_eth(account[2], 500000000000000000);
	client.send(account[2], account[0], 11); // Account 2 send 1 votes to account 0.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 36); // 36 votes number.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedAssignedNumber(), 11);

	client.mortgage_eth(account[1], 500000000000000000);
	client.assign(account[1], 50);
	client.approve_producer(account[1], account[1]);
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 86);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getVoteTo(), Address(account[1].address));

	client.unmake_producer(account[1]); // Account 1 unmake producer. Reset everyone's votes vote to him.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getIsCandidate(), false);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getIsVoted(), false);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getVoteTo(), Address());
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedAssignedNumber(), 11);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 36);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsVoted(), false);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getVoteTo(), Address()); // Vote to address has been reseted.

	client.deAssign(account[0], 36); // Account 0 deassign 36 votes.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 36); // Received votes from others can't be deassign.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedAssignedNumber(), 11);

	client.deAssign(account[0], 25); // Account 0 deassign 25 votes.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 11); // Only assign votes by himself can be deassign.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedAssignedNumber(), 11);

	client.send(account[0], account[0], 10); // Account 0 send 10 votes to himself. Errror happened.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 11); // Nothing happened.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedAssignedNumber(), 11);

	client.make_producer(account[0]);
	client.make_producer(account[2]);
	client.produce_blocks();
	client.approve_producer(account[0], account[2]); //Account 0 vote to account 2. Both of them are producers.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsCandidate(), true);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsVoted(), true);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getVoteTo(), Address(account[2].address));
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getIsCandidate(), true);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getReceivedVotedNumber(), 11);

	client.unmake_producer(account[0]); // Account 0 unmake producer. Who he votes to is also valid.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsCandidate(), false);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsVoted(), true); // Valid.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getVoteTo(), Address(account[2].address)); // Valid.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getIsCandidate(), true);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getReceivedVotedNumber(), 11);
}

BOOST_AUTO_TEST_CASE(dtApproveProducer3)
{
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	client.mortgage_eth(account[0], 500000000000000000);
	client.assign(account[0], 40); // Account 0 assign 40 votes.
	client.make_producer(account[0]);
	client.make_producer(account[1]);
	client.approve_producer(account[0], account[0]);
	client.produce_blocks();
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getUnAssignNumber(), 10); // 10 unassign votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getAssignNumber(), 40); // 40 assign votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsCandidate(), true); // Is candidate.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedVotedNumber(), 40); // 40 votes number.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getIsCandidate(), true); // Is candidate.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 0); // 0 votes number.

	client.approve_producer(account[0], account[1]);
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsCandidate(), true); // Is candidate.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedVotedNumber(), 0); // 0 votes number.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getIsCandidate(), true); // Is candidate.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 40); // 40 votes number.
}

BOOST_AUTO_TEST_CASE(dtNewActiveProducer)
{
	//g_logVerbosity = 13;
	// make blockchain
	DposTestClient client;

	auto& accounts = client.get_accounts();
	BOOST_REQUIRE(accounts.size() >= config::BlocksPerRound+1);

	for (int i = 0; i < config::BlocksPerRound; i++)
	{
		client.make_producer(accounts[i]);
		client.mortgage_eth(accounts[i], 500000000000000000);
		client.assign(accounts[i], 50);
		client.approve_producer(accounts[i], accounts[i]);
	}

	client.produce_blocks(config::BlocksPerRound);

	int new_active_producer_num = config::BlocksPerRound;

	auto active_producers = client.get_active_producers();
	BOOST_REQUIRE(boost::find(active_producers, types::AccountName(accounts[new_active_producer_num].address)) == active_producers.end());

	client.make_producer(accounts[new_active_producer_num]);
	client.mortgage_eth(accounts[new_active_producer_num], 600000000000000000);
	client.assign(accounts[new_active_producer_num], 60);
	client.approve_producer(accounts[new_active_producer_num], accounts[new_active_producer_num]);
	
	client.produce_blocks(config::BlocksPerRound);

	// assert this producer are in gpo.active_producers
	active_producers = client.get_active_producers();

	//std::cout << "active producers: " << std::endl;
	//for (const auto& p : active_producers)
	//	std::cout << p << std::endl;
	//std::cout << accounts[3].address << std::endl;

	BOOST_REQUIRE(boost::find(active_producers, types::AccountName(accounts[new_active_producer_num].address)) != active_producers.end());
}

BOOST_AUTO_TEST_CASE(dtNoEnoughProducer)
{
	DposTestClient client;

	auto all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers.size(), 0);

	auto active_producers = client.get_active_producers();
	BOOST_REQUIRE_NE(active_producers.size(), 0);
}


BOOST_AUTO_TEST_CASE(dtSameVotes)
{
	DposTestClient client;

	int producers_num = config::BlocksPerRound + 1;
	int producers_by_votes_num = config::VotedProducersPerRound;

	auto& accounts = client.get_accounts();
	vector<Address> all_producer_addresses;
	BOOST_REQUIRE(accounts.size() >= producers_num);

	// make more than config::BlocksPerRound producers 
	for (int i = 0; i < producers_num; i++)
	{
		client.make_producer(accounts[i]);
		client.mortgage_eth(accounts[i], 200000000000000000);
		client.assign(accounts[i], 20);
		all_producer_addresses.push_back(Address(accounts[i].address));
	}

	client.produce_blocks(config::BlocksPerRound);
	auto& active_producers = client.get_active_producers();
	std::sort(all_producer_addresses.rbegin(), all_producer_addresses.rend());

	// the producer with id num small than config::BlocksPerRound-1 will be active producer
	for (int i = 0; i < producers_by_votes_num; i++)
	{
		BOOST_REQUIRE(boost::find(active_producers, all_producer_addresses[i]) != active_producers.end());
	}

	// votes 1
	for (int i = 0; i < producers_num; i++)
	{
		client.approve_producer(accounts[i], accounts[i]);
	}

	client.produce_blocks();
	for (int i = 0; i < producers_by_votes_num; i++)
	{
		BOOST_REQUIRE(boost::find(active_producers, all_producer_addresses[i]) != active_producers.end());
	}

}

BOOST_AUTO_TEST_CASE(dtVeryFewVotes)
{
	DposTestClient client;

	int producers_num = config::BlocksPerRound + 1;
	int producers_by_votes_num = config::VotedProducersPerRound;

	auto& accounts = client.get_accounts();
	vector<Address> all_producer_addresses;
	BOOST_REQUIRE(accounts.size() >= producers_num);

	// make more than config::BlocksPerRound producers 
	for (int i = 0; i < producers_num; i++)
	{
		client.make_producer(accounts[i]);
		client.mortgage_eth(accounts[i], 10000000000000000);
		client.assign(accounts[i], 1);
		all_producer_addresses.push_back(Address(accounts[i].address));
	}

	client.produce_blocks(config::BlocksPerRound);
	auto& active_producers = client.get_active_producers();
	std::sort(all_producer_addresses.rbegin(), all_producer_addresses.rend());

	// every producer has 0 vote
	// check producer that id num less than config::BlocksPerRound-1 is an active producer
	for (int i = 0; i < producers_by_votes_num; i++)
	{
		BOOST_REQUIRE(boost::find(active_producers, all_producer_addresses[i]) != active_producers.end());
	}

	// votes 1
	for (int i = 0; i < producers_num; i++)
	{
		client.approve_producer(accounts[i], accounts[i]);
	}

	// approve all producers, every producer has 1 vote
	// check producer that id num less than config::BlocksPerRound-1 is an active producer
	client.produce_blocks();
	for (int i = 0; i < producers_by_votes_num; i++)
	{
		BOOST_REQUIRE(boost::find(active_producers, all_producer_addresses[i]) != active_producers.end());
	}

}
*/

BOOST_AUTO_TEST_CASE(dtMakePowProducer) {
	g_logVerbosity = 14;
	//创建生产者
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& account = client.get_accounts()[0];

	//当前轮次没有pow，即：当前有一个accountname为空
	auto currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 0);
	
	client.make_pow_producer(account);
	client.produce_blocks(config::TotalProducersPerRound);
	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 1);
}




BOOST_AUTO_TEST_SUITE_END()


//对链进行测试
BOOST_FIXTURE_TEST_SUITE(DposBlockTests, DpTestFixture)

BOOST_AUTO_TEST_CASE(bcMultiChainTest) {}
BOOST_AUTO_TEST_CASE(bcForkChainTest) {}
BOOST_AUTO_TEST_CASE(bcForkStressTest) {}
BOOST_AUTO_TEST_CASE(bcTransactionLimitTest) {}

BOOST_AUTO_TEST_SUITE_END()


}
}
