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
void DposTestClient::make_pow_producer(Account& _from, enum class setPowTest tests)
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
		//sol.op. = _sol.op;
		sol.op.worker_pubkey = _sol.op.worker_pubkey;
		sol.op.block_id = _sol.op.block_id;
		sol.op.nonce = _sol.op.nonce;
		sol.op.input = _sol.op.input;
		sol.op.work = _sol.op.work;
		sol.op.signature = _sol.op.signature;

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

	if (setPowTest::errorSignature == tests)
	{
		sol.op.signature = SignatureStruct();
	}
	else if (setPowTest::errorTarget == tests)
	{
		for (; ; tryNonce++)
		{
			op.nonce = tryNonce;
			op.create(priviteKey, op.work_input());
			// 计算结果小于等于target的时候退出，报告找到的这个解
			if (op.work > target)
				break;

		}
		sol.op.work = op.work;
	}
	else if(setPowTest::errorBlockid == tests)
	{
		produce_blocks(1);

	}
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

json_spirit::mObject fillFakeTest(json_spirit::mObject const& _input)
{
	g_logVerbosity = 14;
	json_spirit::mObject output;
	string const& testName = TestOutputHelper::get().testName();
	TestBlock genesisBlock(_input.at("genesisBlockHeader").get_obj(), _input.at("pre").get_obj());
	genesisBlock.setBlockHeader(genesisBlock.blockHeader());

	TestBlockChain testChain(genesisBlock);
	assert(testChain.getInterface().isKnown(genesisBlock.blockHeader().hash(WithSeal)));

	//创建生产者
	std::shared_ptr<class producer_plugin> p = make_shared<class producer_plugin>(testChain.getInterface());
	p->get_chain_controller().setStateDB(testChain.testGenesis().state().db());
	testChain.interfaceUnsafe().setProducer(p);

	output["genesisBlockHeader"] = writeBlockHeaderToJson(genesisBlock.blockHeader());
	output["genesisRLP"] = toHexPrefixed(genesisBlock.bytes());
	BOOST_REQUIRE(_input.count("blocks"));

	mArray blArray;
	size_t importBlockNumber = 0;
	string chainname = "default";
	string chainnetwork = "default";
	std::map<string, ChainBranch*> chainMap = { { chainname , new ChainBranch(genesisBlock) } };

	if (_input.count("network") > 0)
		output["network"] = _input.at("network");
	unsigned int  signe_num = 0;
	unsigned int  irr_num = 0;
	//判断是否是作假签名
	if (_input.count("fake"))
	{
		mObject fakeObj = _input.at("fake").get_obj();
		if (fakeObj.count("signature"))
			signe_num = (int)toInt(fakeObj.at("signature").get_obj().at("B"));
	}

	for (auto const& bl : _input.at("blocks").get_array())
	{
		mObject const& blObjInput = bl.get_obj();
		mObject blObj;
		if (blObjInput.count("blocknumber") > 0)
		{
			importBlockNumber = max((int)toInt(blObjInput.at("blocknumber")), 1);
			blObj["blocknumber"] = blObjInput.at("blocknumber");
		}
		else
			importBlockNumber++;

		if (blObjInput.count("chainname") > 0)
		{
			chainname = blObjInput.at("chainname").get_str();
			blObj["chainname"] = blObjInput.at("chainname");
		}
		else
			chainname = "default";

		if (blObjInput.count("chainnetwork") > 0)
		{
			chainnetwork = blObjInput.at("chainnetwork").get_str();
			blObj["chainnetwork"] = blObjInput.at("chainnetwork");
		}
		else
			chainnetwork = "default";

		// Copy expectException* fields
		for (auto const& field : blObjInput)
			if (field.first.substr(0, 15) == "expectException")
				blObj[field.first] = field.second;

		if (chainMap.count(chainname) > 0)
		{
			if (_input.count("noBlockChainHistory") == 0)
			{
				ChainBranch::forceBlockchain(chainnetwork);
				chainMap[chainname]->reset();
				ChainBranch::resetBlockchain();
				chainMap[chainname]->restoreFromHistory(importBlockNumber);
			}
		}
		else
		{
			ChainBranch::forceBlockchain(chainnetwork);
			//chainMap[chainname] = new ChainBranch(genesisBlock);
			chainMap[chainname] = new ChainBranch(chainMap["default"]);
			ChainBranch::resetBlockchain();
			chainMap[chainname]->restoreFromHistory(importBlockNumber);
		}

		TestBlock block;
		TestBlockChain& blockchain = chainMap[chainname]->blockchain;
		vector<TestBlock>& importedBlocks = chainMap[chainname]->importedBlocks;
		chain::chain_controller & _chain(chainMap[chainname]->producer->get_chain_controller());


		BOOST_REQUIRE(blObjInput.count("transactions"));
		for (auto& txObj : blObjInput.at("transactions").get_array())
		{
			TestTransaction transaction(txObj.get_obj());
			block.addTransaction(transaction);
		}

		//Import Uncles
		for (auto const& uHObj : blObjInput.at("uncleHeaders").get_array())
		{
			cnote << "Generating uncle block at test " << testName;
			TestBlock uncle;
			mObject uncleHeaderObj = uHObj.get_obj();
			string uncleChainName = chainname;
			if (uncleHeaderObj.count("chainname") > 0)
				uncleChainName = uncleHeaderObj["chainname"].get_str();

			overwriteUncleHeaderForTest(uncleHeaderObj, uncle, block.uncles(), *chainMap[uncleChainName]);
			block.addUncle(uncle);
		}

		vector<TestBlock> validUncles = blockchain.syncUncles(block.uncles());
		block.setUncles(validUncles);

		if (blObjInput.count("blockHeaderPremine"))
			overwriteBlockHeaderForTest(blObjInput.at("blockHeaderPremine").get_obj(), block, *chainMap[chainname]);

		cnote << "Mining block" << importBlockNumber << "for chain" << chainname << "at test " << testName;
		//block.mine(blockchain);
		//生产块
		auto slot = 1;
		auto accountName = _chain.get_scheduled_producer(slot);
		while (accountName == AccountName())
			accountName = _chain.get_scheduled_producer(++slot);
		auto pro = _chain.get_producer(accountName);
		auto private_key = chainMap[chainname]->producer->get_private_key(pro.owner);
		block.dposMine(blockchain, _chain.get_slot_time(slot), pro.owner, private_key);
		cnote << "Block mined with...";
		cnote << "Transactions: " << block.transactionQueue().topTransactions(100).size();
		cnote << "Uncles: " << block.uncles().size();

		TestBlock alterBlock(block);
		checkBlocks(block, alterBlock, testName);

		if (blObjInput.count("blockHeader"))
			overwriteBlockHeaderForTest(blObjInput.at("blockHeader").get_obj(), alterBlock, *chainMap[chainname]);

		blObj["rlp"] = toHexPrefixed(alterBlock.bytes());
		blObj["blockHeader"] = writeBlockHeaderToJson(alterBlock.blockHeader());

		//判断是否需要作假签名
		if (signe_num != 0 && chainname == "B" && importBlockNumber == signe_num)
		{
			auto slot = 1;
			AccountName accountName("0x06f7740ac1bf8323c61423e1e98df6db737dac5c");
			fc::ecc::private_key private_key("0ff6814a57898936fe085835a3070aeaf3877b4f9d1aec7e4fdb81eab2120de8");

			TestBlock InvalidBlock(block);
			checkBlocks(block, InvalidBlock, testName);

			InvalidBlock.dposMine(blockchain, _chain.get_slot_time(slot), accountName, private_key);
			if (blObjInput.count("blockHeader"))
				overwriteBlockHeaderForTest(blObjInput.at("blockHeader").get_obj(), InvalidBlock, *chainMap[chainname]);

			blObj["rlp"] = toHexPrefixed(InvalidBlock.bytes());
			blObj["blockHeader"] = writeBlockHeaderToJson(InvalidBlock.blockHeader());
		}

		mArray aUncleList;
		for (auto const& uncle : alterBlock.uncles())
		{
			mObject uncleHeaderObj = writeBlockHeaderToJson(uncle.blockHeader());
			aUncleList.push_back(uncleHeaderObj);
		}
		blObj["uncleHeaders"] = aUncleList;
		blObj["transactions"] = writeTransactionsToJson(alterBlock.transactionQueue());

		compareBlocks(block, alterBlock);
		try
		{
			if (blObjInput.count("expectException"))
				BOOST_ERROR("Deprecated expectException field! " + testName);

			blockchain.addBlock(alterBlock);
			if (testChain.addBlock(alterBlock))
				cnote << "The most recent best Block now is " << importBlockNumber << "in chain" << chainname << "at test " << testName;


			bool isException = (blObjInput.count("expectException" + test::netIdToString(test::TestBlockChain::s_sealEngineNetwork))
				|| blObjInput.count("expectExceptionALL"));
			BOOST_REQUIRE_MESSAGE(!isException, "block import expected exception, but no exception was thrown!");

			if (_input.count("noBlockChainHistory") == 0)
			{
				importedBlocks.push_back(alterBlock);
				importedBlocks.back().clearState(); //close the state as it wont be needed. too many open states would lead to exception.
			}
		}
		catch (dev::eth::ExceedIrreversibleBlock)
		{
			cnote << testName + "block import throw an exception: " << "import an irreversibleBlock!";
		}
		catch (dev::eth::ExceedRollbackImportBlock)
		{
			cnote << testName + "block import throw an exception: " << "Rollback block chain!";
		}
		catch (Exception const& _e)
		{
			cnote << testName + "block import throw an exception: " << diagnostic_information(_e);
			checkExpectedException(blObj, _e);
			eraseJsonSectionForInvalidBlock(blObj);
		}
		catch (std::exception const& _e)
		{
			cnote << testName + "block import throw an exception: " << _e.what();
			cout << testName + "block import thrown std exeption\n";
			eraseJsonSectionForInvalidBlock(blObj);
		}
		catch (...)
		{
			cout << testName + "block import thrown unknown exeption\n";
			eraseJsonSectionForInvalidBlock(blObj);
		}

		blArray.push_back(blObj);  //json data
	}//each blocks

	 //如果是作假的链，就改变filler中的值
	if (signe_num != 0 && chainname == "B" && importBlockNumber == signe_num)
	{
		if (_input.count("expect") > 0)
		{
			AccountMaskMap expectStateMap;
			State stateExpect(State::Null);
			ImportTest::importState(_input.at("expect").get_obj(), stateExpect, expectStateMap);
			if (ImportTest::compareStates(stateExpect, chainMap["A"]->blockchain.topBlock().state(), expectStateMap, WhenError::Throw))
				cerr << testName << "\n";
		}

		output["blocks"] = blArray;
		output["postState"] = fillJsonWithState(chainMap["A"]->blockchain.topBlock().state());
		output["lastblockhash"] = toHexPrefixed(chainMap["A"]->blockchain.topBlock().blockHeader().hash(WithSeal));
	}
	else
	{//没有作假时正常的流程
		if (_input.count("expect") > 0)
		{
			AccountMaskMap expectStateMap;
			State stateExpect(State::Null);
			ImportTest::importState(_input.at("expect").get_obj(), stateExpect, expectStateMap);
			if (ImportTest::compareStates(stateExpect, testChain.topBlock().state(), expectStateMap, WhenError::Throw))
				cerr << testName << "\n";
		}

		output["blocks"] = blArray;
		output["postState"] = fillJsonWithState(testChain.topBlock().state());
		output["lastblockhash"] = toHexPrefixed(testChain.topBlock().blockHeader().hash(WithSeal));
	}

	//make all values hex in pre section
	State prestate(State::Null);
	ImportTest::importState(_input.at("pre").get_obj(), prestate);
	output["pre"] = fillJsonWithState(prestate);

	for (auto iterator = chainMap.begin(); iterator != chainMap.end(); iterator++)
		delete iterator->second;

	return output;
}
void testFakeBCTest(json_spirit::mObject const& _o)
{
	string testName = TestOutputHelper::get().testName();
	TestBlock genesisBlock(_o.at("genesisBlockHeader").get_obj(), _o.at("pre").get_obj());
	TestBlockChain blockchain(genesisBlock);


	std::shared_ptr<class producer_plugin> producer = make_shared<class producer_plugin>(blockchain.getInterface());
	producer->get_chain_controller().setStateDB(blockchain.testGenesis().state().db());
	blockchain.interfaceUnsafe().setProducer(producer);

	TestBlockChain testChain(genesisBlock);
	assert(testChain.getInterface().isKnown(genesisBlock.blockHeader().hash(WithSeal)));
	std::shared_ptr<class producer_plugin> p = make_shared<class producer_plugin>(testChain.getInterface());
	p->get_chain_controller().setStateDB(testChain.testGenesis().state().db());
	testChain.interfaceUnsafe().setProducer(p);

	if (_o.count("genesisRLP") > 0)
	{
		TestBlock genesisFromRLP(_o.at("genesisRLP").get_str());
		checkBlocks(genesisBlock, genesisFromRLP, testName);
	}


	for (auto const& bl : _o.at("blocks").get_array())
	{
		mObject blObj = bl.get_obj();
		TestBlock blockFromRlp;
		State const preState = testChain.topBlock().state();
		h256 const preHash = testChain.topBlock().blockHeader().hash();
		try
		{
			TestBlock blRlp(blObj["rlp"].get_str());
			blockFromRlp = blRlp;
			if (blObj.count("blockHeader") == 0)
				blockFromRlp.noteDirty();			//disable blockHeader check in TestBlock
			testChain.addBlock(blockFromRlp);
		}
		// if exception is thrown, RLP is invalid and no blockHeader, Transaction list, or Uncle list should be given
		catch (dev::eth::ExceedIrreversibleBlock)
		{
			cnote << testName + "block import throw an exception: " << "import an irreversibleBlock!";
		}
		catch (dev::eth::ExceedRollbackImportBlock)
		{
			cnote << testName + "block import throw an exception: " << "Rollback block chain!";
		}
		catch (Exception const& _e)
		{
			cnote << testName + "state sync or block import did throw an exception: " << diagnostic_information(_e);
			checkJsonSectionForInvalidBlock(blObj);
			continue;
		}
		catch (std::exception const& _e)
		{
			cnote << testName + "state sync or block import did throw an exception: " << _e.what();
			checkJsonSectionForInvalidBlock(blObj);
			continue;
		}
		catch (...)
		{
			cnote << testName + "state sync or block import did throw an exception\n";
			checkJsonSectionForInvalidBlock(blObj);
			continue;
		}

		//block from RLP successfully imported. now compare this rlp to test sections
		BOOST_REQUIRE_MESSAGE(blObj.count("blockHeader"),
			"blockHeader field is not found. "
			"filename: " + TestOutputHelper::get().testFileName() +
			" testname: " + TestOutputHelper::get().testName()
		);

		//Check Provided Header against block in RLP
		TestBlock blockFromFields(blObj["blockHeader"].get_obj());

		//ImportTransactions
		BOOST_REQUIRE_MESSAGE(blObj.count("transactions"),
			"transactions field is not found. "
			"filename: " + TestOutputHelper::get().testFileName() +
			" testname: " + TestOutputHelper::get().testName()
		);
		for (auto const& txObj : blObj["transactions"].get_array())
		{
			TestTransaction transaction(txObj.get_obj());
			blockFromFields.addTransaction(transaction);
		}

		// ImportUncles
		vector<u256> uncleNumbers;
		if (blObj["uncleHeaders"].type() != json_spirit::null_type)
		{
			BOOST_REQUIRE_MESSAGE(blObj["uncleHeaders"].get_array().size() <= 2, "Too many uncle headers in block! " + TestOutputHelper::get().testName());
			for (auto const& uBlHeaderObj : blObj["uncleHeaders"].get_array())
			{
				mObject uBlH = uBlHeaderObj.get_obj();
				BOOST_REQUIRE((uBlH.size() == 16));

				TestBlock uncle(uBlH);
				blockFromFields.addUncle(uncle);
				uncleNumbers.push_back(uncle.blockHeader().number());
			}
		}

		checkBlocks(blockFromFields, blockFromRlp, testName);

		try
		{
			blockchain.addBlock(blockFromFields);
		}
		catch (dev::eth::ExceedIrreversibleBlock)
		{
			cnote << testName + "block import throw an exception: " << "import an irreversibleBlock!";
			break;
		}
		catch (dev::eth::ExceedRollbackImportBlock)
		{
			cnote << testName + "block import throw an exception: " << "Rollback block chain!";
			break;
		}
		catch (Exception const& _e)
		{
			cerr << testName + "Error importing block from fields to blockchain: " << diagnostic_information(_e);
			break;
		}

		//Check that imported block to the chain is equal to declared block from test
		bytes importedblock = testChain.getInterface().block(blockFromFields.blockHeader().hash(WithSeal));
		TestBlock inchainBlock(toHex(importedblock));
		checkBlocks(inchainBlock, blockFromFields, testName);

		string blockNumber = toString(testChain.getInterface().number());
		string blockChainName = "default";
		if (blObj.count("chainname") > 0)
			blockChainName = blObj["chainname"].get_str();
		if (blObj.count("blocknumber") > 0)
			blockNumber = blObj["blocknumber"].get_str();

		//check the balance before and after the block according to mining rules
		if (blockFromFields.blockHeader().parentHash() == preHash)
		{
			State const postState = testChain.topBlock().state();
			assert(testChain.getInterface().sealEngine());
			bigint reward = calculateMiningReward(testChain.topBlock().blockHeader().number(), uncleNumbers.size() >= 1 ? uncleNumbers[0] : 0, uncleNumbers.size() >= 2 ? uncleNumbers[1] : 0, *testChain.getInterface().sealEngine());
			ImportTest::checkBalance(preState, postState, reward);
		}
		else
		{
			cnote << "Block Number " << testChain.topBlock().blockHeader().number();
			cnote << "Skipping the balance validation of potential correct block: " << TestOutputHelper::get().testName();
		}

		cnote << "Tested topblock number" << blockNumber << "for chain " << blockChainName << testName;

	}//allBlocks

	 //Check lastblock hash
	BOOST_REQUIRE((_o.count("lastblockhash") > 0));
	string lastTrueBlockHash = toHexPrefixed(testChain.topBlock().blockHeader().hash(WithSeal));
	BOOST_CHECK_MESSAGE(lastTrueBlockHash == _o.at("lastblockhash").get_str(),
		testName + "Boost check: lastblockhash does not match " + lastTrueBlockHash + " expected: " + _o.at("lastblockhash").get_str());

	//Check final state (just to be sure)
	BOOST_CHECK_MESSAGE(toString(testChain.topBlock().state().rootHash()) ==
		toString(blockchain.topBlock().state().rootHash()),
		testName + "State root in chain from RLP blocks != State root in chain from Field blocks!");

	State postState(State::Null); //Compare post states
	BOOST_REQUIRE((_o.count("postState") > 0));
	ImportTest::importState(_o.at("postState").get_obj(), postState);
	ImportTest::compareStates(postState, testChain.topBlock().state());
	ImportTest::compareStates(postState, blockchain.topBlock().state());
}
json_spirit::mValue FakeBlockTestSuite::doTests(json_spirit::mValue const& _input, bool _fillin) const
{
	json_spirit::mObject tests;
	for (auto const& i : _input.get_obj())
	{
		string const& testname = i.first;
		json_spirit::mObject const& inputTest = i.second.get_obj();

		//Select test by name if --singletest is set and not filling state tests as blockchain
		if (!Options::get().fillchain && !TestOutputHelper::get().checkTest(testname))
			continue;

		BOOST_REQUIRE_MESSAGE(inputTest.count("genesisBlockHeader"),
			"\"genesisBlockHeader\" field is not found. filename: " + TestOutputHelper::get().testFileName() +
			" testname: " + TestOutputHelper::get().testName()
		);
		BOOST_REQUIRE_MESSAGE(inputTest.count("pre"),
			"\"pre\" field is not found. filename: " + TestOutputHelper::get().testFileName() +
			" testname: " + TestOutputHelper::get().testName()
		);

		if (inputTest.count("expect"))
		{
			BOOST_REQUIRE_MESSAGE(_fillin, "a filled test should not contain any expect fields.");
			spellCheckNetworkNamesInExpectField(inputTest.at("expect").get_array());
		}

		if (_fillin)
		{
			//create a blockchain test for each network
			for (auto& network : test::getNetworks())
			{
				if (!Options::get().singleTestNet.empty() && Options::get().singleTestNet != test::netIdToString(network))
					continue;

				dev::test::TestBlockChain::s_sealEngineNetwork = network;
				string newtestname = testname + "_" + test::netIdToString(network);

				json_spirit::mObject jObjOutput = inputTest;
				if (inputTest.count("expect"))
				{
					//prepare the corresponding expect section for the test
					json_spirit::mArray const& expects = inputTest.at("expect").get_array();
					bool found = false;

					for (auto& expect : expects)
					{
						vector<string> netlist;
						json_spirit::mObject const& expectObj = expect.get_obj();
						ImportTest::parseJsonStrValueIntoVector(expectObj.at("network"), netlist);

						if (std::find(netlist.begin(), netlist.end(), test::netIdToString(network)) != netlist.end() ||
							std::find(netlist.begin(), netlist.end(), "ALL") != netlist.end())
						{
							jObjOutput["expect"] = expectObj.at("result");
							found = true;
							break;
						}
					}
					if (!found)
						jObjOutput.erase(jObjOutput.find("expect"));
				}
				TestOutputHelper::get().setCurrentTestName(newtestname);
				jObjOutput = fillFakeTest(jObjOutput);
				jObjOutput["network"] = test::netIdToString(network);
				if (inputTest.count("_info"))
					jObjOutput["_info"] = inputTest.at("_info");
				tests[newtestname] = jObjOutput;
			}
		}
		else
		{
			BOOST_REQUIRE_MESSAGE(inputTest.count("network"),
				"\"network\" field is not found. filename: " + TestOutputHelper::get().testFileName() +
				" testname: " + TestOutputHelper::get().testName()
			);
			dev::test::TestBlockChain::s_sealEngineNetwork = stringToNetId(inputTest.at("network").get_str());
			if (test::isDisabledNetwork(dev::test::TestBlockChain::s_sealEngineNetwork))
				continue;
			testFakeBCTest(inputTest);
		}
	}

	return tests;
}
fs::path FakeBlockTestSuite::suiteFolder() const
{
	return "FakeBlockTests";
}
fs::path FakeBlockTestSuite::suiteFillerFolder() const
{
	return "FakeBlockTestsFiller";
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

class FokeFixture {
public:
	FokeFixture()
	{
		FakeBlockTestSuite suite;
		string const& casename = boost::unit_test::framework::current_test_case().p_name;

		suite.runAllTestsInFolder(casename);
	}
};


BOOST_FIXTURE_TEST_SUITE(DposTestsSuite, TestOutputHelperFixture)
BOOST_AUTO_TEST_CASE(dtMakeProducer)
{
	//g_logVerbosity = 13;

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

	//验证注册生产者，且得票数是“0”
	// check this account has become a producer
	BOOST_REQUIRE(all_producers.find(types::AccountName(account.address)) != all_producers.end());

	// check every producer's vote is 0
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account.address)], 0);

	//验证注册成为生产者话费的gas，期望话费90gas，跟配置项一直
	u256 balance = client.balance(Address(account.address));
	cout << "balance.str(): " << balance.str() << endl;
	BOOST_REQUIRE(u256(1000000000000000000) - balance = u256(2100000) * u256(2000000000));

	// unmake producer
	client.unmake_producer(account);

	//验证不再是生产者
	// check this account isn't producer anymore
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account.address)) == all_producers.end());

	//验证注册成为生产者话费的gas，期望话费90gas，跟配置项一直
	u256 balance1 = client.balance(Address(account.address));
	u256 cost_1 = balance - balance1;
	cout << "cost_1.str(): " << cost_1.str() << endl;
	BOOST_REQUIRE(cost_1  = u256(210000) * u256(2000000000));
}

BOOST_AUTO_TEST_CASE(dtMortgage)
{
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();
	u256 balance = client.balance(Address(account[0].address));

	//验证扣费包括：  1、抵押的金额  2、调用预编译合约的手续费
	BOOST_REQUIRE(u256(1000000000000000000) - balance == u256(500000000000000000) + u256(2100000) * u256(2000000000));

	//验证未使用的投票权
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
	//g_logVerbosity = 14;

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

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(PowTestsSuite, TestOutputHelperFixture)
BOOST_AUTO_TEST_CASE(dtMakePowProducer) 
{
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

	client.make_pow_producer(account, setPowTest::none);
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

BOOST_AUTO_TEST_CASE(dtMakePowProducers) 
{
	g_logVerbosity = 14;
	//创建生产者
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	//auto currentProducers = client.get_active_producers();
	BOOST_REQUIRE(client.get_dpo_witnesses()  == 0);

	for (auto i : accounts)
	{
		client.make_pow_producer(i, setPowTest::none);
	}
	
	client.produce_blocks(2);

	BOOST_REQUIRE(client.get_dpo_witnesses() == accounts.size());
	//下轮次pow矿工成功加入生产块的轮次
}

BOOST_AUTO_TEST_CASE(dtGetErrorSignature)
{
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

	client.make_pow_producer(account, setPowTest::errorSignature);
	client.produce_blocks(config::TotalProducersPerRound);
	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 0);
}
BOOST_AUTO_TEST_CASE(dtGetErrorTarget)
{
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

	client.make_pow_producer(account, setPowTest::errorTarget);
	client.produce_blocks(config::TotalProducersPerRound);
	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 0);
}

BOOST_AUTO_TEST_CASE(dtGetErrorBlockid)
{
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

	client.make_pow_producer(account, setPowTest::errorBlockid);
	client.produce_blocks(config::TotalProducersPerRound);
	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 0);
}
BOOST_AUTO_TEST_SUITE_END()

//对链进行测试
BOOST_FIXTURE_TEST_SUITE(DposBlockTests, DpTestFixture)

BOOST_AUTO_TEST_CASE(bcMultiChainTest) {}
BOOST_AUTO_TEST_CASE(bcForkChainTest) {}
BOOST_AUTO_TEST_CASE(bcForkStressTest) {}
BOOST_AUTO_TEST_CASE(bcTransactionLimitTest) {}

BOOST_AUTO_TEST_SUITE_END()

//链中有造假的块，进行异常测试
BOOST_FIXTURE_TEST_SUITE(FakeBlockTests, FokeFixture)
BOOST_AUTO_TEST_CASE(bcFakeBlockTest) {}
BOOST_AUTO_TEST_SUITE_END()


}
}
