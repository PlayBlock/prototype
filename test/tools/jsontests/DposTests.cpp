#include "DposTests.h"
#include <boost/range/algorithm/find.hpp>
using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

namespace dev {
namespace test {

fs::path DposBlockTestSuite::suiteFolder() const
{
	return "DposBlockTests";
}
fs::path DposBlockTestSuite::suiteFillerFolder() const
{
	return "DposBlockTestsFiller";
}

fs::path FakeBlockTestSuite::suiteFolder() const
{
	return "FakeBlockTests";
}
fs::path FakeBlockTestSuite::suiteFillerFolder() const
{
	return "FakeBlockTestsFiller";
}
json_spirit::mObject fillFakeTest(json_spirit::mObject const& _input)
{
	//g_logVerbosity = 14;
	json_spirit::mObject output;
	string const& testName = TestOutputHelper::get().testName();
	TestBlock genesisBlock(_input.at("genesisBlockHeader").get_obj(), _input.at("pre").get_obj());
	genesisBlock.setBlockHeader(genesisBlock.blockHeader());

	TestBlockChain testChain(genesisBlock);
	assert(testChain.getInterface().isKnown(genesisBlock.blockHeader().hash(WithSeal)));

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
	unsigned int  Timeout_num = 0;
	//判断是否是作假签名
	if (_input.count("fake"))
	{
		mObject fakeObj = _input.at("fake").get_obj();
		if (fakeObj.count("signature"))
			signe_num = (int)toInt(fakeObj.at("signature").get_obj().at("B"));
		if(fakeObj.count("Timeout"))
			Timeout_num = (int)toInt(fakeObj.at("Timeout").get_obj().at("B"));
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
		chain::chain_controller & _chain(blockchain.getProducerPluginPtr()->get_chain_controller());


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

		cnote << "Mining block" << importBlockNumber << "for chain" << chainname << "at test " << testName;
		//block.mine(blockchain);
		//生产块
		auto slot = 1;
		if (Timeout_num != 0 && chainname == "B" && importBlockNumber == Timeout_num)
		{
			slot = 2;
		}
		auto accountName = _chain.get_scheduled_producer(slot);
		while (accountName == AccountName())
			accountName = _chain.get_scheduled_producer(++slot);
		auto pro = _chain.get_producer(accountName);
		auto private_key = blockchain.getProducerPluginPtr()->get_private_key(pro.owner);
		block.dposMine(blockchain, _chain.get_slot_time(slot), pro.owner, private_key);
		cnote << "Block mined with...";
		cnote << "Transactions: " << block.transactionQueue().topTransactions(100).size();
		cnote << "Uncles: " << block.uncles().size();

		TestBlock alterBlock(block);
		checkBlocks(block, alterBlock, testName);

		if (blObjInput.count("blockHeader"))
			overwriteBlockHeaderForTest(blObjInput.at("blockHeader").get_obj(), alterBlock, *chainMap[chainname], private_key);

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
				overwriteBlockHeaderForTest(blObjInput.at("blockHeader").get_obj(), InvalidBlock, *chainMap[chainname], private_key);

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

	TestBlockChain testChain(genesisBlock);
	assert(testChain.getInterface().isKnown(genesisBlock.blockHeader().hash(WithSeal)));

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
	cout << "dtMakeProducer" << endl;
	//g_logVerbosity = 13;

	// make blockchain
	DposTestClient client;

	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 6);
	auto& account = client.get_accounts()[0];
	auto& account2 = client.get_accounts()[1];
	auto& account3 = client.get_accounts()[2];
	auto& account4 = client.get_accounts()[3];
	auto& account5 = client.get_accounts()[4];
	auto& account6 = client.get_accounts()[5];

	// get all producers that have registered	
	auto all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account.address)) == all_producers.end());

	// make producer
	int expectedCost = client.make_producer(account);

	client.produce_blocks();
	all_producers = client.get_all_producers();

	//验证注册生产者，且得票数是“0”
	// check this account has become a producer
	BOOST_REQUIRE(all_producers.find(types::AccountName(account.address)) != all_producers.end());

	// check every producer's vote is 0
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account.address)], 0);

	//验证注册成为生产者花费的gas，期望花费的gas符合相关配置
	u256 balance = client.balance(Address(account.address));
	//cout << "balance.str(): " << balance.str() << endl;
	BOOST_REQUIRE(u256(1000000000000000000) - balance == u256(expectedCost) * u256(2000000000));

	// unmake producer
	int expectedCost2 = client.unmake_producer(account);

	//验证不再是生产者
	// check this account isn't producer anymore
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account.address)) == all_producers.end());

	//验证注册成为生产者话费的gas，期望话费90gas，跟配置项一直
	u256 balance1 = client.balance(Address(account.address));
	u256 cost_1 = balance - balance1;
	//cout << "cost_1.str(): " << cost_1.str() << endl;
	BOOST_REQUIRE(cost_1 == u256(expectedCost2) * u256(2000000000));

	//注册自己是块生产候选人
	client.make_producer(account);
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account.address)], 0);
	u256 balance2 = client.balance(Address(account.address));
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account.address)].getName(), "a");
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account.address)].getURL(), "bcdef");

	//再次注册自己是块生产候选人
	client.make_producer(account, "name_test", "url_test");
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account.address)], 0);
	u256 balance3 = client.balance(Address(account.address));
	BOOST_REQUIRE(balance2 - balance3 <= u256(3300000) * u256(2000000000));
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account.address)].getName(), "a");
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account.address)].getURL(), "bcdef");

	//注册成为生产者
	client.unmake_producer(account);
	client.produce_blocks();
	client.make_producer(account, "name_test", "url_test");
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account.address)], 0);
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account.address)].getName(), "name_test");
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account.address)].getURL(), "url_test");

	//注册成为生产者，name与url取最长值
	string name(40, 'a');
	string url(120, 'n');
	client.make_producer(account2, name, url);
	client.produce_blocks();
	u256 balance4 = client.balance(Address(account2.address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance4 < u256(3300000) * u256(2000000000));
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account2.address)].getName(), name);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account2.address)].getURL(), url);
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account2.address)) != all_producers.end());

	//注册成为生产者，name超长
	string name_long(41, 'a');
	client.make_producer(account3, name_long, "123456");
	client.produce_blocks();
	u256 balance5 = client.balance(Address(account3.address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance5 < u256(3300000) * u256(2000000000));
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account3.address)) == all_producers.end());

	//注册成为生产者，url超长
	string url_long(121, 'n');
	client.make_producer(account4, "123456", url_long);
	client.produce_blocks();
	u256 balance6 = client.balance(Address(account4.address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance6 < u256(3300000) * u256(2000000000));
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account4.address)) == all_producers.end());

	//注册成为生产者，name取最长值
	client.make_producer(account5, name, "123456");
	client.produce_blocks();
	u256 balance7 = client.balance(Address(account5.address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance7 < u256(3300000) * u256(2000000000));
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account5.address)].getName(), name);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account5.address)].getURL(), "123456");
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account5.address)) != all_producers.end());

	//注册成为生产者，url取最长值
	client.make_producer(account6, "123456", url);
	client.produce_blocks();
	u256 balance8 = client.balance(Address(account6.address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance8 < u256(3300000) * u256(2000000000));
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account6.address)].getName(), "123456");
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account6.address)].getURL(), url);
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account6.address)) != all_producers.end());
}

BOOST_AUTO_TEST_CASE(dtMortgage)
{
	cout << "dtMortgage" << endl;
	//g_logVerbosity = 3;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	int expectedCost = client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();
	u256 balance = client.balance(Address(account[0].address));

	//验证扣费包括：  1、抵押的金额  2、调用预编译合约的手续费
	BOOST_REQUIRE(balance == u256(1000000000000000000) - u256(500000000000000000) - u256(expectedCost) * u256(2000000000));

	//验证未使用的投票权
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 50); // Mortage eth to have 50 votes.

	int expectedCost2 = client.redeem_eth(account[0], 5);// Redeem 5 votes with 0.05 eth.
	client.produce_blocks();
	u256 balance2 = client.balance(Address(account[0].address));

	//验证最新余额：  在之前余额的基础上增加赎回的金额，并扣除手续费
	BOOST_REQUIRE(balance2 == balance + u256(50000000000000000) - u256(expectedCost2) * u256(2000000000));
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 45); // Redeem eth reduce to 45 votes.

	client.redeem_eth(account[0], 50);// Redeem 50 votes with 0.5 eth.
	client.produce_blocks();
	balance = client.balance(Address(account[0].address));
	//BOOST_REQUIRE(balance < u256(550000000000000000));// Account 0's balnce shouldn't increase without enough votes.
	//BOOST_REQUIRE(balance >= u256(550000000000000000) - u256(1000000000000000) * 3);
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 45); // Redeem eth reduce to 45 votes.

	client.redeem_eth(account[1], 5);// Redeem 5 votes with 0.05 eth.
	client.produce_blocks();
	balance = client.balance(Address(account[1].address));
	BOOST_REQUIRE(balance < u256(1000000000000000000));// Account 1's balnce shouldn't increase without votes.
	BOOST_REQUIRE(balance >= u256(1000000000000000000) - u256(1000000000000000) * 1);

	client.mortgage_eth(account[2], 1100000000000000000);// Account 2 mortage 1.1 eth with 110 votes, but have no enough eth.
	client.produce_blocks();
	client.mortgage_eth(account[3], 1000000000000000000);// Account 3 mortage 1.0 eth with 100 votes,but have no enough eth.
	client.produce_blocks();
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
	client.produce_blocks();
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks(); 
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks(); 
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks(); 
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks(); 
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks(); 
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks(); 
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks(); 
	client.mortgage_eth(account[5], 100000000000000000);// Mortage 0.1 eth eth with 10 votes.
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[5].address)].getHoldVoteNumber(), 90);
	balance = client.balance(Address(account[5].address));
	BOOST_REQUIRE(balance < u256(100000000000000000));// Account 0's balnce should reduce according to mortgage.
	BOOST_REQUIRE(balance >= u256(100000000000000000) - u256(1000000000000000) * 9);

	//使用其他人传给自己的选票去获取token
	client.send(account[5], account[6], 10);
	client.produce_blocks();
	client.redeem_eth(account[6], 20);
	client.produce_blocks();
	balance = client.balance(Address(account[6].address));
	BOOST_REQUIRE(balance < u256(1000000000000000000));
	BOOST_REQUIRE(balance > u256(1000000000000000000) - u256(1000000000000000));

	client.redeem_eth(account[6], 10);
	client.produce_blocks();
	balance = client.balance(Address(account[6].address));
	BOOST_REQUIRE(balance < u256(1100000000000000000));
	BOOST_REQUIRE(balance > u256(1000000000000000000) - u256(1000000000000000) * u256(2));
}

BOOST_AUTO_TEST_CASE(dtApproveProducer)
{
	cout << "dtApproveProducer" << endl;
	//g_logVerbosity = 3;

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

	u256 balance = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance < u256(1000000000000000000));
	BOOST_REQUIRE(balance > u256(1000000000000000000) - (u256(3300000) * u256(2000000000)));

	// Mortage 0.5 eth eth with 50 votes.
	client.mortgage_eth(account[0], 500000000000000000);//nonce = 1
	client.produce_blocks();
	u256 balance2 = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance2 < u256(500000000000000000));// Account 0's balnce should reduce according to mortgage.

	// Redeem 5 votes with 0.05 eth.
	client.redeem_eth(account[0], 5);//nonce = 2
	client.produce_blocks();
	u256 balance3 = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance3 < u256(550000000000000000));// Account 0's balnce should increase according to mortgage.

	// 投票给自己，再取消投票
	int expectedCost = client.approve_producer(account[0], account[0], 8);//nonce = 5

	// check this producer's vote is 1
	client.produce_blocks();
	all_producers = client.get_all_producers();
	u256 balance6 = client.balance(Address(account[0].address));
	BOOST_REQUIRE_EQUAL(all_producers.size(), 1);
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 8);// Account 0 should have 8 votes from himself.
	BOOST_REQUIRE_EQUAL(balance3 - balance6, u256(expectedCost) * u256(2000000000));

	// unapprove
	int expectedCost2 = client.unapprove_producer(account[0], account[0], 3);//nonce = 6
	client.produce_blocks();
	all_producers = client.get_all_producers();
	u256 balance7 = client.balance(Address(account[0].address));
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[0].address)], 5);
	BOOST_REQUIRE_EQUAL(balance6 - balance7, u256(expectedCost2) * u256(2000000000));


	// 投票给别人，再取消投票
	client.make_producer(account[1]);
	client.produce_blocks();
	client.approve_producer(account[0], account[1], 10);
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[1].address)], 10);

	client.unapprove_producer(account[0], account[1], 4);
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[1].address)], 6);

	// 再次注册成为块候选人，什么也不改变，正常扣除gas
	u256 balance4 = client.balance(Address(account[0].address));
	client.make_producer(account[1]);
	client.produce_blocks();

	all_producers = client.get_all_producers();
	BOOST_REQUIRE_EQUAL(all_producers[types::AccountName(account[1].address)], 6);
	u256 balance5 = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance4 - balance5 < u256(3300000) * u256(2000000000));

	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[1].address)], 6);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 34);
}

BOOST_AUTO_TEST_CASE(dtSend)
{
	cout << "dtSend" << endl;
	//g_logVerbosity = 3;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 16);
	auto account = client.get_accounts();

	int expectedCost = client.send(account[0], account[1], 10);
	client.produce_blocks();

	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 0);
	u256 balance = client.balance(Address(account[0].address));
	BOOST_REQUIRE_EQUAL(u256(1000000000000000000) - balance , u256(expectedCost) * u256(2000000000));

	//将抵押的选票发送给别人
	//将选票发送给，没有“未使用的选票”的人
	client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();
	u256 balance2 = client.balance(Address(account[0].address));
	int expectedCost2 = client.send(account[0], account[1], 10);
	client.produce_blocks();

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 40);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 10);
	u256 balance3 = client.balance(Address(account[0].address));
	BOOST_REQUIRE_EQUAL(balance2 - balance3 , u256(expectedCost2) * u256(2000000000));

	//将接受到的选票，发送给别人
	//将选票发送给，没有“未使用的选票”的人
	client.send(account[1], account[2], 3);
	client.produce_blocks();

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 7);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getHoldVoteNumber(), 3);
	u256 balance4 = client.balance(Address(account[1].address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance4 > 0);

	//发送选票给自己
	client.send(account[2], account[2], 2);
	client.produce_blocks();

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getHoldVoteNumber(), 3);
	u256 balance5 = client.balance(Address(account[2].address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance5 > 0);

	//发送给别人的选票为负
	client.send(account[0], account[1], -10);
	client.produce_blocks();

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 40);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 7);
	u256 balance6 = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance3 - balance6 > 0);

	//将选票发送给，已经有“未使用的选票”的人
	client.send(account[0], account[1], 10);
	client.produce_blocks();

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 30);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 17);
	u256 balance7 = client.balance(Address(account[0].address));
	BOOST_REQUIRE(balance6 - balance7 > 0);

	//在其他人已经投满候选人的时候，将选票传给他
	for (int i = 1; i <= 15; i++)
	{
		client.make_producer(account[i]);
		client.produce_blocks();
		client.approve_producer(account[0], account[i], 1);
		client.produce_blocks();
	}
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 15);
	client.send(account[1], account[0], 10);
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 25);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 7);
}

BOOST_AUTO_TEST_CASE(dtVoteNumTest)
{
	cout << "dtVoteNumTest" << endl;
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	//没有选票的时候，进行投票
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 0); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 0);

	u256 start_balance = client.balance(AccountName(account[0].address));
	int expectedCost = client.approve_producer(account[0], account[1], 5);
	client.produce_blocks();
	u256 end_balance = client.balance(AccountName(account[0].address));

	BOOST_REQUIRE_EQUAL(start_balance - end_balance, u256(expectedCost) * u256(2000000000));
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 0); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 0);


	//1.选取一个账户，抵押获取票数，产块
	client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();

	//2.选取另一个账户作为块候选者、出块
	client.make_producer(account[1]);
	client.produce_blocks();

	//3.验证未使用的投票权
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 50); // Mortage eth to have 50 votes.

																									//4.将选票投给候选人、出块
	client.approve_producer(account[0], account[1], 5);
	client.produce_blocks();

	//5.验证候选人的获得的票数与投票账户的票数
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 45); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 5);

}

BOOST_AUTO_TEST_CASE(dtVoteDiffProducersTest)
{
	cout << "dtVoteDiffProducersTest" << endl;
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	//1.选取一个账户，抵押获取票数，产块
	client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();

	//2.验证未使用的投票权
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 50); // Mortage eth to have 50 votes.

																									//投票给未注册的账户，投票失败
	auto balance = client.balance(AccountName(account[0].address));
	client.approve_producer(account[0], account[1], 2);
	client.produce_blocks();
	auto balance2 = client.balance(AccountName(account[0].address));
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 50);
	BOOST_REQUIRE(balance - balance2 > 0);


	//3.注册生产者
	for (auto i = 1; i <= 15; i++)
	{
		client.make_producer(account[i]);
		client.produce_blocks();
	}

	//4.投票给前15个人的时候，投票正常
	for (auto i = 1; i <= 15; i++)
	{
		client.approve_producer(account[0], account[i], 2);
		client.produce_blocks();
	}

	//4.验证前15个投票成功
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 20); // Mortage eth to have 50 votes.
	for (auto i = 1; i <= 15; i++)
	{
		BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[i].address)].getReceivedVotedNumber(), 2); // Mortage eth to have 50 votes.
	}
	//5.给第16个人投票
	auto start_balance = client.balance(AccountName(account[0].address));
	client.approve_producer(account[0], account[16], 2);
	client.produce_blocks();
	auto end_balance = client.balance(AccountName(account[0].address));

	//6.验证候选人的获得的票数与投票账户的票数
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 20); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[16].address)].getReceivedVotedNumber(), 0);
	BOOST_REQUIRE(end_balance - start_balance > 0);

}

BOOST_AUTO_TEST_CASE(dtVoteSameProducersTest)
{
	cout << "dtVoteSameProducersTest" << endl;
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	//1.选取一个账户，抵押获取票数，产块
	client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();

	//2.验证未使用的投票权
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 50); // Mortage eth to have 50 votes.

																									//3.注册生产者
	client.make_producer(account[1]);
	client.produce_blocks();

	//4.投票给前15个人的时候，投票正常
	for (auto i = 1; i <= 16; i++)
	{
		client.approve_producer(account[0], account[1], 2);
		client.produce_blocks();
	}

	//4.验证前15个投票成功
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 18); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 32); // Mortage eth to have 50 votes.

																										 //5.取消5个投票、出块
	auto start_balance = client.balance(AccountName(account[0].address));
	client.unapprove_producer(account[0], account[1], 2);
	client.produce_blocks();
	auto end_balance = client.balance(AccountName(account[0].address));

	//6.判断剩余投票个数
	BOOST_REQUIRE(end_balance - start_balance > 0);
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 20); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 30); // Mortage eth to have 50 votes.

}

BOOST_AUTO_TEST_CASE(dtVoteFakeTest)
{
	cout << "dtVoteFakeTest" << endl;
	//g_logVerbosity = 13;

	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	//1.选取一个账户，抵押获取票数，产块
	client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();

	//2.验证未使用的投票权
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 50); // Mortage eth to have 50 votes.

																									//3.取消投票给未注册的生产者
	auto start_balance = client.balance(AccountName(account[0].address));
	client.unapprove_producer(account[0], account[1], 5);
	client.produce_blocks();
	auto end_balance = client.balance(AccountName(account[0].address));

	//4.判断剩余投票个数
	BOOST_REQUIRE(end_balance - start_balance > 0);
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 50); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getReceivedVotedNumber(), 0); // Mortage eth to have 50 votes.


																										//3.注册生产者
	client.make_producer(account[2]);
	client.produce_blocks();

	//4.account[0]给生产者投票
	client.approve_producer(account[0], account[2], 10);
	client.produce_blocks();

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 40);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getReceivedVotedNumber(), 10);

	//没有票的账户取消account[2]的票
	start_balance = client.balance(AccountName(account[1].address));
	client.unapprove_producer(account[1], account[2], 5);
	client.produce_blocks();
	end_balance = client.balance(AccountName(account[1].address));

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 0); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getReceivedVotedNumber(), 10);

	//5.另一个账户抵押投票
	client.mortgage_eth(account[1], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 50);

	//4.account[1]取消投票，取消失败，票数不变，消耗gas
	start_balance = client.balance(AccountName(account[1].address));
	client.unapprove_producer(account[1], account[2], 5);
	client.produce_blocks();
	end_balance = client.balance(AccountName(account[1].address));

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 50); // Mortage eth to have 50 votes.
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getReceivedVotedNumber(), 10); // Mortage eth to have 50 votes.
	BOOST_REQUIRE(end_balance - start_balance > 0);
}

BOOST_AUTO_TEST_CASE(dtAboutMakeProducerTest)
{
	cout << "dtAboutMakeProducerTest" << endl;
	//g_logVerbosity = 13;
	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();
	
	//1.注册21个块生产者、作抵押
	for (auto i = 0; i < config::TotalProducersPerRound; i++)
		client.make_producer(account[i]);
	client.produce_blocks();

	for (auto i = 0; i < config::TotalProducersPerRound; i++)
		client.mortgage_eth(account[i], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();

	//2.给21个生产者进行投票
	for (auto i = 0; i < config::TotalProducersPerRound; i++)
		client.approve_producer(account[i],account[i],30);
	client.produce_blocks();

	auto all_votes = client.get_votes();
	for (auto i = 0; i < config::TotalProducersPerRound; i++)
		BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[i].address)].getReceivedVotedNumber(), 30);

	//3.注册第22个生产者,投票为0
	client.make_producer(account[21]);
	client.produce_blocks();

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[21].address)].getHoldVoteNumber(), 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[21].address)].getReceivedVotedNumber(), 0);
	BOOST_REQUIRE_EQUAL(client.get_dpo_witnesses(), 0);

	//4.第22个生产者宣布自己做出pow解
	client.make_pow_producer(account[21],setPowTest::none);
	client.produce_blocks(config::TotalProducersPerRound - 4);
	BOOST_REQUIRE_EQUAL(client.get_dpo_witnesses(), 0);

	//5.下一轮出块还没有当前的生产者
	map<AccountName, int> accountName;
	client.produce_blocks_Number(config::TotalProducersPerRound, accountName);

	BOOST_REQUIRE(accountName[AccountName(account[21].address)] == 0);

	//6.第21个生产者给自己投100票
	client.mortgage_eth(account[21], 500000000000000000);
	client.produce_blocks();
	client.approve_producer(account[21],account[21],30);
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[21].address)].getReceivedVotedNumber(), 30);

	//7.第22个生产者宣布自己做出pow解
	client.make_pow_producer(account[21], setPowTest::none);
	client.produce_blocks(config::TotalProducersPerRound - 2);
	BOOST_REQUIRE_EQUAL(client.get_dpo_witnesses(), 0);

	//8.出一轮块
	accountName.clear();
	client.produce_blocks_Number(config::TotalProducersPerRound, accountName);
	BOOST_REQUIRE(accountName.find(AccountName(account[21].address)) != accountName.end());
	BOOST_REQUIRE_EQUAL(client.get_dpo_witnesses(),0);
}
BOOST_AUTO_TEST_CASE(dtAboutPowProducerTest)
{
	cout << "dtAboutPowProducerTest" << endl;
	//g_logVerbosity = 13;
	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	//1.注册21个块生产者、作抵押
	for (auto i = 0; i < config::TotalProducersPerRound; i++)
		client.make_producer(account[i]);
	client.produce_blocks();

	for (auto i = 0; i < config::TotalProducersPerRound; i++)
		client.mortgage_eth(account[i], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();

	//2.给21个生产者进行投票
	for (auto i = 0; i < config::TotalProducersPerRound; i++)
		client.approve_producer(account[i], account[i], 30);
	client.produce_blocks();

	auto all_votes = client.get_votes();
	for (auto i = 0; i < config::TotalProducersPerRound; i++)
		BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[i].address)].getReceivedVotedNumber(), 30);
   
	//4.第22个生产者宣布自己做出pow解
	client.make_pow_producer(account[21], setPowTest::none);
	client.produce_blocks(2);
	BOOST_REQUIRE_EQUAL(client.get_dpo_witnesses(), 1);

	//5.注册第22个生产者,投票为0
	client.make_producer(account[21]);
	client.produce_blocks(2);

	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[21].address)].getHoldVoteNumber(), 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[21].address)].getReceivedVotedNumber(), 0);
	BOOST_REQUIRE_EQUAL(client.get_dpo_witnesses(), 0);
	client.produce_blocks(config::TotalProducersPerRound - 8);
	//6.下一轮出块还没有当前的生产者
	map<AccountName, int> accountName;
	client.produce_blocks_Number(config::TotalProducersPerRound, accountName);

	BOOST_REQUIRE(accountName[AccountName(account[21].address)] == 0);
}

BOOST_AUTO_TEST_CASE(dtundoMakeProducerTest)
{
	cout << "dtundoMakeProducerTest" << endl;
	//g_logVerbosity = 14;
	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 5);
	auto& account = client.get_accounts();

	//1.注册生产者、出一轮块
	client.make_producer(account[0]);
	client.produce_blocks(config::TotalProducersPerRound);

	//2.注销生产者、出一轮块
	client.unmake_producer(account[0]);
	map<AccountName, int> accountNum;
	client.produce_blocks_Number(config::TotalProducersPerRound,accountNum);
	//3.判断这一轮是否成功出块
	BOOST_REQUIRE(accountNum[AccountName(account[0].address)] > 0);
	//4.判断下一轮这个生产者还是否出块
	accountNum.clear();
	client.produce_blocks_Number(config::TotalProducersPerRound, accountNum);
	BOOST_REQUIRE(accountNum.find(AccountName(account[0].address)) == accountNum.end());


}

BOOST_AUTO_TEST_CASE(dtUnMakeProducer)
{
	cout << "dtUnMakeProducer" << endl;
	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 16);
	auto account = client.get_accounts();

	client.make_producer(account[0]);
	client.produce_blocks();
	auto all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account[0].address)) != all_producers.end());
	BOOST_REQUIRE(all_producers.size() == 1);

	//不是块生产候选人，发出取消请求，不会有任何响应
	client.unmake_producer(account[1]);
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account[0].address)) != all_producers.end());
	BOOST_REQUIRE(all_producers.size() == 1);
	u256 balance = client.balance(Address(account[1].address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance < u256(3300000) * u256(2000000000));

	//同一个块中，顺序发出交易，成为块生产候选人，再发出请求取消成为块生产候选人。不会有任何响应
	client.make_producer(account[2]);
	client.unmake_producer(account[2]);
	client.produce_blocks();
	all_producers = client.get_all_producers();
	BOOST_REQUIRE(all_producers.find(types::AccountName(account[2].address)) == all_producers.end());
	BOOST_REQUIRE(all_producers.size() == 1);
	u256 balance2 = client.balance(Address(account[2].address));
	BOOST_REQUIRE(u256(1000000000000000000) - balance2 < u256(3300000) * u256(2000000000) * 2);

	//取消注册成为生产者，重置该节点的得票数为0，并且将其他人投给这个节点的票返还
	//该地址投给该地址的选票也会退回，该地址投给其他地址的选票不受影响
	client.mortgage_eth(account[0], 500000000000000000);
	client.produce_blocks();
	client.mortgage_eth(account[1], 500000000000000000);
	client.produce_blocks();
	client.mortgage_eth(account[2], 500000000000000000);
	client.produce_blocks();
	client.approve_producer(account[0], account[0], 10);
	client.produce_blocks();
	client.approve_producer(account[1], account[0], 20);
	client.produce_blocks();
	client.approve_producer(account[2], account[0], 12);
	client.produce_blocks();
	client.make_producer(account[3]);
	client.produce_blocks();
	client.make_producer(account[4]);
	client.produce_blocks();
	client.approve_producer(account[0], account[3], 2);
	client.produce_blocks();
	client.approve_producer(account[0], account[4], 3);
	client.produce_blocks();
	client.approve_producer(account[1], account[4], 4);
	client.produce_blocks();
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsCandidate(), true);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedVotedNumber(), 42);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[0].address)], 10);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[3].address)], 2);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[4].address)], 3);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 35);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteRecord()[Address(account[0].address)], 20);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteRecord()[Address(account[4].address)], 4);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 26);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getHoldVoteRecord()[Address(account[0].address)], 12);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getHoldVoteNumber(), 38);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[3].address)].getReceivedVotedNumber(), 2);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[4].address)].getReceivedVotedNumber(), 7);

	u256 balance3 = client.balance(Address(account[0].address));
	int expectedCost = client.unmake_producer(account[0]);
	client.produce_blocks();
	all_votes = client.get_votes();
	u256 balance4 = client.balance(Address(account[0].address));
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getIsCandidate(), false);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getReceivedVotedNumber(), 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[0].address)], 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[3].address)], 2);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[4].address)], 3);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteNumber(), 45);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteRecord()[Address(account[0].address)], 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteRecord()[Address(account[4].address)], 4);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[1].address)].getHoldVoteNumber(), 46);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getHoldVoteRecord()[Address(account[0].address)], 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[2].address)].getHoldVoteNumber(), 50);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[3].address)].getReceivedVotedNumber(), 2);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[4].address)].getReceivedVotedNumber(), 7);
	BOOST_REQUIRE_EQUAL(balance3 - balance4, u256(expectedCost) * u256(2000000000));
}

BOOST_AUTO_TEST_CASE(dtVote15Address)
{
	cout << "dtVote15Address" << endl;
	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 16);
	auto account = client.get_accounts();
	
	client.mortgage_eth(account[0], 500000000000000000);// Mortage 0.5 eth eth with 50 votes.
	client.produce_blocks();

	//投票给15个地址
	for (int i = 0; i < 15; i++)
	{
		client.make_producer(account[i]);
		client.produce_blocks();
		client.approve_producer(account[0], account[i], 1);
		client.produce_blocks();
	}
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord().size(), 15);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[14].address)], 1);

	//给第16个地址投票失败
	client.approve_producer(account[0], account[15], 1);
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord().size(), 15);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[15].address)], 0);

	//可以继续给这15个地址追加投票
	client.approve_producer(account[0], account[14], 1);
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord().size(), 15);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[14].address)], 2);

	//取消给一个地址的投票后，可以给新的地址投票
	client.unapprove_producer(account[0], account[14], 2);
	client.produce_blocks();
	client.make_producer(account[15]);
	client.produce_blocks();
	client.approve_producer(account[0], account[15], 1);
	client.produce_blocks();
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord().size(), 15);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[14].address)], 0);
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(account[0].address)].getHoldVoteRecord()[Address(account[15].address)], 1);
}


BOOST_AUTO_TEST_SUITE_END()


//对链进行测试
BOOST_FIXTURE_TEST_SUITE(DposBlockTests, DpTestFixture)

BOOST_AUTO_TEST_CASE(bcMultiChainTest) {}
BOOST_AUTO_TEST_CASE(bcForkChainTest) {}
BOOST_AUTO_TEST_CASE(bcContractTest) {}
//BOOST_AUTO_TEST_CASE(bcTransactionLimitTest) {}

BOOST_AUTO_TEST_SUITE_END()

//链中有造假的块，进行异常测试
BOOST_FIXTURE_TEST_SUITE(FakeBlockTests, FokeFixture)
BOOST_AUTO_TEST_CASE(bcFakeBlockTest) {}
BOOST_AUTO_TEST_SUITE_END()


}
}
