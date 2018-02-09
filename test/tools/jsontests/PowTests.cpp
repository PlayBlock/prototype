#include "DPwTestsHelper.h"
using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

namespace dev {
	namespace test {

BOOST_FIXTURE_TEST_SUITE(PowTestsSuite, TestOutputHelperFixture)
BOOST_AUTO_TEST_CASE(dtMakePowProducer)
{
	cout << "dtMakePowProducer" << endl;

	//g_logVerbosity = 14;
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
	num = 0;
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 1);

	//出了一轮后，pow生产者没有了--创世期检测产块是否是pow
	client.produce_blocks(config::TotalProducersPerRound);

	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	num = 0;
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 0);
}

BOOST_AUTO_TEST_CASE(dtGetErrorSignature)
{
	cout << "dtGetErrorSignature" << endl;

	//g_logVerbosity = 14;
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
	cout << "dtGetErrorTarget" << endl;

	//g_logVerbosity = 14;
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
	cout << "dtGetErrorBlockid" << endl;

	//g_logVerbosity = 14;
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

	ctrace << "account.balance = " << client.balance(AccountName(account.address));
	BOOST_REQUIRE((u256(1000000000000000000) - client.balance(AccountName(account.address))) >=0);
}
//同时有N个节点计算出来target
BOOST_AUTO_TEST_CASE(dtlowTarget)
{
	cout << "dtlowTarget" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	EthashCPUMiner::setNumInstances(1);
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	//auto currentProducers = client.get_active_producers();
	BOOST_REQUIRE(client.get_dpo_witnesses() == 0);

	for (auto i : accounts)
	{
		client.make_pow_producer(i, setPowTest::none);
	}

	client.produce_blocks(5);

	BOOST_REQUIRE(client.get_dpo_witnesses() == accounts.size());
	//比较本地算出的与矿工算出来的是否相同
	BOOST_REQUIRE(client.get_ownpow_target() == client.get_pow_target());
}
//pow计算过程中接收到新的块，停止计算
BOOST_AUTO_TEST_CASE(dtHighTarget)
{
	cout << "dtHighTarget" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	BOOST_REQUIRE(client.get_dpo_witnesses() == 0);

	for (auto i : accounts)
	{
		client.make_pow_producer(i, setPowTest::none);
	}

	client.produce_blocks(2);

	BOOST_REQUIRE(client.get_dpo_witnesses() == accounts.size());
	
}
//一个生产者计算出多个解
BOOST_AUTO_TEST_CASE(dtGetTwoTarget)
{
	cout << "dtGetTwoTarget" << endl;

	//g_logVerbosity = 14;
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

	for (int i = 1; i <= 2; i++)
	{
		client.make_pow_producer(account, setPowTest::none);
		client.produce_blocks(1);
	}
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

//判断pow被选中的时候，队列中减少对应的生产者
BOOST_AUTO_TEST_CASE(dtPowWitness)
{
	cout << "dtPowWitness" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	EthashCPUMiner::setNumInstances(1);
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	//先出三轮块结束创世期
	client.produce_blocks(config::TotalProducersPerRound * 3);

	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	BOOST_REQUIRE(client.get_dpo_witnesses() == 0);

	for (auto i : accounts)
	{
		client.make_pow_producer(i, setPowTest::none);
	}

	client.produce_blocks(2);
	BOOST_REQUIRE(client.get_dpo_witnesses() == accounts.size());

	//第n轮之后判断pow人员是否减少
	int n = 2;
	for (auto i =1; i <= n;i++)
	{
		if (client.get_dpo_witnesses() > config::TotalProducersPerRound)
		{
			//获取当前pow队列中的人
			const auto& allProducerbyPow_start = client.get_allproducer_power();
			auto itr = allProducerbyPow_start.upper_bound(0);

			client.produce_blocks(config::TotalProducersPerRound);
			BOOST_REQUIRE(client.get_dpo_witnesses() == (accounts.size() - (config::POWProducersPerRound+1)*i));

			const auto& allProducerbyPow_end = client.get_allproducer_power();
			auto first = allProducerbyPow_end.upper_bound(0);
			BOOST_REQUIRE(itr != first);
		}
		else
		{
			client.produce_blocks(config::TotalProducersPerRound);
			BOOST_REQUIRE(client.get_dpo_witnesses() == (accounts.size() - config::POWProducersPerRound*i));
		}
		
	}
}

//Pow计算的时候产生新块，
BOOST_AUTO_TEST_CASE(dtPowingaddBlock)
{
	cout << "dtPowingaddBlock" << endl;

	//g_logVerbosity = 14;
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

	client.add_new_Work(account);
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



BOOST_FIXTURE_TEST_SUITE(BlockTestsSuite, TestOutputHelperFixture)
/*创世期的test，需要把创世期的块高度改成84*/
BOOST_AUTO_TEST_CASE(dtMoreDposProducer)
{
	cout << "dtMoreDposProducer" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	DposTestClient client;
	int num = 0;
	// pick account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前没有DPOS生产者
	auto VoteProducers = client.get_all_producers();
	BOOST_REQUIRE(VoteProducers.size() == 0);

	//注册DPOS生产者
	for (auto &i : accounts)
	{
		client.make_producer(i);
		client.mortgage_eth(i, 500000000000000000);
	}
	client.produce_blocks();
	auto all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(accounts[0].address)].getHoldVoteNumber(), 50);
	for (auto &i : accounts)
	{
		client.approve_producer(i, i, 30);
	}
	//创世期出块
	client.produce_blocks(config::TotalProducersPerRound-1);
	all_votes = client.get_votes();
	BOOST_REQUIRE_EQUAL(all_votes[types::AccountName(accounts[0].address)].getReceivedVotedNumber(), 30);

	//当前全部都是DPOS生产者出块
	VoteProducers = client.get_all_producers();
	BOOST_REQUIRE(VoteProducers.size() == accounts.size());

	auto currentProducers = client.get_active_producers();
	std::vector<Address> initProducers = client.getGenesisAccount();
	for (auto i : initProducers)
	{
		for (auto pro : currentProducers)
		{
			if (types::AccountName(i) == types::AccountName(pro))
				num++;
		}
	}
	BOOST_REQUIRE_EQUAL(num,0);
}

BOOST_AUTO_TEST_CASE(dtLittleDposProducer)
{
	cout << "dtLittleDposProducer" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	DposTestClient client;
	int num = 0;
	// pick account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前没有DPOS生产者
	auto VoteProducers = client.get_all_producers();
	ctrace << "currentProducers : " << VoteProducers.size() ;
	BOOST_REQUIRE(VoteProducers.size() == 0);

	//注册DPOS生产者
	int producerNum = 15;
	for (auto i =0;i <producerNum;i++)
	{
		client.make_producer(accounts[i]);
	}
	//创世期出块
	client.produce_blocks(config::TotalProducersPerRound);

	//当前全部都是DPOS生产者出块
	VoteProducers = client.get_all_producers();
	BOOST_REQUIRE(VoteProducers.size() == producerNum);

	auto currentProducers = client.get_active_producers();
	std::vector<Address> initProducers = client.getGenesisAccount();
	for (auto i : initProducers)
	{
		for (auto pro : currentProducers)
		{
			if (types::AccountName(i) == types::AccountName(pro))
				num++;
		}
	}
	BOOST_REQUIRE(num != 0);
}

BOOST_AUTO_TEST_CASE(dtMakePowProducer)
{
	cout << "dtMakePowProducer" << endl;

	//g_logVerbosity = 14;
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
	num = 0;
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 1);

	//出了一轮后，pow生产者没有了--创世期检测产块是否是pow
	client.produce_pow_blocks(AccountName(account.address), config::TotalProducersPerRound);

	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	num = 0;
	for (auto pro : currentProducers)
	{
		if (types::AccountName(account.address) == types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == 0);
}

BOOST_AUTO_TEST_CASE(dtMakeMorePowProducer)
{
	cout << "dtMakeMorePowProducer" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	EthashCPUMiner::setNumInstances(1);
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	auto currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		for(auto i : accounts)
		   if (types::AccountName(i.address) == types::AccountName(pro))
			   num++;
	}
	BOOST_REQUIRE(num == 0);

	//注册Pow生产者
	for (auto i : accounts)
	{
		client.make_pow_producer(i, setPowTest::none);
	}
	//创世期出块
	client.produce_blocks(config::TotalProducersPerRound-1);

	if (client.get_dpo_witnesses() > config::TotalProducersPerRound)
	{
		client.produce_blocks();
		//下轮次pow矿工成功加入生产块的轮次
		currentProducers = client.get_active_producers();
		num = 0;
		for (auto pro : currentProducers)
		{
			for (auto i : accounts)
				if (types::AccountName(i.address) == types::AccountName(pro))
					num++;
		}
		BOOST_REQUIRE(num == currentProducers.size());
		//第二轮出块，应该是剩余的个pow生产块
		client.produce_blocks(config::TotalProducersPerRound);
		//下轮次pow矿工成功加入生产块的轮次
		currentProducers = client.get_active_producers();
		num = 0;
		for (auto pro : currentProducers)
		{
			for (auto i : accounts)
				if (types::AccountName(i.address) == types::AccountName(pro))
					num++;
		}
		//获取当前pow队列中的人
		BOOST_REQUIRE_EQUAL(num, accounts.size() - config::TotalProducersPerRound -1);
	}
	else
	{
		client.produce_blocks();
		//下轮次pow矿工成功加入生产块的轮次
		currentProducers = client.get_active_producers();
		num = 0;
		for (auto pro : currentProducers)
		{
			for (auto i : accounts)
				if (types::AccountName(i.address) == types::AccountName(pro))
					num++;
		}
		BOOST_REQUIRE(num == currentProducers.size());
		//第二轮出块，应该是剩余的个pow生产块
		client.produce_blocks(config::TotalProducersPerRound);

		//下轮次pow矿工成功加入生产块的轮次
		currentProducers = client.get_active_producers();
		num = 0;
		for (auto pro : currentProducers)
		{
			for (auto i : accounts)
				if (types::AccountName(i.address) == types::AccountName(pro))
					num++;
		}
		client.produce_blocks(config::TotalProducersPerRound);
		BOOST_REQUIRE_EQUAL(num, accounts.size() - currentProducers.size());
	}
	
}

/*注意：测试时，通过控制makeProducerCount个数来控制注册的pow个数*/
BOOST_AUTO_TEST_CASE(dtMakeLittlePowProducer)
{
	cout << "dtMakeLittlePowProducer" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	EthashCPUMiner::setNumInstances(1);
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	auto currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}
	BOOST_REQUIRE(num == 0);

	//注册Pow生产者
	int makeProducerCount = 10;
	for (auto i =0;i < makeProducerCount;i++)
	{
		client.make_pow_producer(accounts[i], setPowTest::none);
	}
	//创世期出块
	client.produce_blocks(config::TotalProducersPerRound);

	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	num = 0;
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}

	BOOST_REQUIRE_EQUAL(num , makeProducerCount);

}

/*创世期->稳定期*/
//测试的时候需要动态调整pow注册的个数
BOOST_AUTO_TEST_CASE(dtCheckPowProducer)
{
	cout << "dtCheckPowProducer" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	EthashCPUMiner::setNumInstances(1);
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	auto currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}
	BOOST_REQUIRE(num == 0);
	//先出两轮，使块高度达到63
	client.produce_blocks(config::TotalProducersPerRound*2);
	//注册Pow生产者
	for (auto i : accounts)
	{
		client.make_pow_producer(i, setPowTest::none);
	}
	//创世期出块
	client.produce_blocks(config::TotalProducersPerRound);

	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	num = 0;
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}

	BOOST_REQUIRE(num == currentProducers.size());

	//稳定期出块，应该是剩余的个pow生产块
	client.produce_blocks(config::TotalProducersPerRound);

	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	num = 0;
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}
	//1)pow充足的情况下（动态调整）
	BOOST_REQUIRE_EQUAL(num , config::POWProducersPerRound);
	//2)pow不充足的情况
	//BOOST_REQUIRE(num == accounts.size()-config::TotalProducersPerRound);
	//BOOST_REQUIRE(num > 0);

}

BOOST_AUTO_TEST_SUITE_END()




BOOST_FIXTURE_TEST_SUITE(StableBlockTestsSuite, TestOutputHelperFixture)

//无DPOS节点
BOOST_AUTO_TEST_CASE(dtCheckPowProducers)
{
	cout << "dtCheckPowProducers" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	EthashCPUMiner::setNumInstances(1);
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	auto currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}
	BOOST_REQUIRE_EQUAL(num, 0);
	//先出三轮块结束创世期
	client.produce_blocks(config::TotalProducersPerRound*3);
	//注册Pow生产者
	for (auto i : accounts)
	{
		client.make_pow_producer(i, setPowTest::none);
	}
	//
	client.produce_blocks(config::TotalProducersPerRound);

	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	num = 0;
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}
	//1)pow充足的情况下（动态调整）
	BOOST_REQUIRE_EQUAL(num ,config::POWProducersPerRound);
	//2)pow不充足的情况
	//BOOST_REQUIRE(num == accounts.size());
	//BOOST_REQUIRE(num > 0);

}
//无POW节点
BOOST_AUTO_TEST_CASE(dtNoPowProducers)
{
	cout << "dtNoPowProducers" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	auto currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}
	BOOST_REQUIRE(num == 0);

	//创世期出块
	client.produce_blocks(config::TotalProducersPerRound);

	//下轮次pow矿工成功加入生产块的轮次
	currentProducers = client.get_active_producers();
	num = 0;
	for (auto pro : currentProducers)
	{
		for (auto i : accounts)
			if (types::AccountName(i.address) == types::AccountName(pro))
				num++;
	}
	//1)pow充足的情况下（动态调整）
	//BOOST_REQUIRE(num == config::POWProducersPerRound);
	//2)pow不充足的情况
	BOOST_REQUIRE(num == 0);

}
//无POW节点、DPOS节点充足
BOOST_AUTO_TEST_CASE(dtEnoughDposProducers)
{
	cout << "dtEnoughDposProducers" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	DposTestClient client;
	int num = 0;
	// pick an account
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	auto& accounts = client.get_accounts();

	//当前轮次没有pow，即：当前有一个accountname为空
	auto currentProducers = client.get_active_producers();
	for (auto pro : currentProducers)
	{
		if (types::AccountName() != types::AccountName(pro))
			num++;
	}
	BOOST_REQUIRE(num == client.getGenesisAccount().size());

	for (auto i = 0; i <= 10; i++)
	{
		client.make_producer(accounts[i]);
	}

	//创世期出块
	client.produce_blocks(config::TotalProducersPerRound);

	//下轮次
	currentProducers = client.get_active_producers();
	num = 0;
	for (auto pro : currentProducers)
	{
		if (types::AccountName() != types::AccountName(pro))
			num++;
	}

	BOOST_REQUIRE(num == config::DPOSProducersPerRound);

}

BOOST_AUTO_TEST_CASE(dtRaceSpeedTest)
{
	cout << "dtRaceSpeedTest" << endl;

	//g_logVerbosity = 14;
	//创建生产者
	EthashCPUMiner::setNumInstances(1);
	DposTestClient client;

	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	//先出三轮块结束创世期
	client.produce_blocks(config::TotalProducersPerRound * 3);

	auto& accounts = client.get_accounts();

	//1.注册19个生产者
	for (auto i = 0; i < 19; i++)
	{
		client.make_producer(accounts[i]);
	}
	client.produce_blocks();
	for (auto i = 19; i < 23; i++)
	{
		client.make_pow_producer(accounts[i], setPowTest::none);
	}
	client.produce_blocks();
	//2.选出前16个生产者,抵押、每三个出一个块
	for (auto i = 0; i < 21; i++)
	//for (auto i = 0; i < 19; i++)
	{
		client.mortgage_eth(accounts[i], 600000000000000000);	
	}
	client.produce_blocks();
	//3.投票、出块
	for (auto i = 0; i < config::DPOSVotedProducersPerRound; i++)
	{	
		client.approve_producer(accounts[i], accounts[i], 60);
	}
	client.produce_blocks();
	//4.选出虚拟赛跑的生产者
	for (auto i = config::DPOSVotedProducersPerRound,j =20; i < config::DPOSVotedProducersPerRound + 3; i++,j += 10)
	{
		client.approve_producer(accounts[i], accounts[i], j);
	}
	//结束本轮剩余出块
	client.produce_blocks(17);

	//第3-9轮出块，校验三个虚拟赛跑的生产者出块的个数比是否为2：3：4
	std::map<AccountName, int> account_block;
	for (auto run = 0; run < 9; run++)
	{
		for (auto i = 19; i < 23; i++)
		{
			client.make_pow_producer(accounts[i], setPowTest::none);
		}
		client.produce_blocks_Number(config::TotalProducersPerRound, account_block);

	}

	BOOST_REQUIRE_EQUAL(account_block[AccountName(accounts[16].address)] , 2);
	BOOST_REQUIRE_EQUAL(account_block[AccountName(accounts[17].address)] , 3);
	BOOST_REQUIRE_EQUAL(account_block[AccountName(accounts[18].address)] , 4);
}

BOOST_AUTO_TEST_CASE(dtVoteChangeTest)
{
	cout << "dtVoteChangeTest" << endl;

	//g_logVerbosity = 13;
	//创建生产者
	EthashCPUMiner::setNumInstances(1);
	DposTestClient client;
	BOOST_REQUIRE(client.get_accounts().size() >= 1);
	//先出三轮块结束创世期
	client.produce_blocks(config::TotalProducersPerRound * 3);

	auto& accounts = client.get_accounts();

	//1.注册19个生产者
	for (auto i = 0; i < 19; i++)
	{
		client.make_producer(accounts[i]);
	}
	client.produce_blocks();
	//注册pow生产者
	for (auto i = 19; i < 23; i++)
	{
		client.make_pow_producer(accounts[i], setPowTest::none);
	}
	client.produce_blocks();
	//2.选出前16个生产者,抵押、每三个出一个块
	for (auto i = 0; i < 19; i++)
	{
		client.mortgage_eth(accounts[i], 600000000000000000);
	}
	client.produce_blocks();
	//3.投票、出块
	for (auto i = 0; i < config::DPOSVotedProducersPerRound; i++)
	{
		client.approve_producer(accounts[i], accounts[i], 60);
	}
	//4.选出虚拟赛跑的生产者
	for (auto i = config::DPOSVotedProducersPerRound, j = 20; i < config::DPOSVotedProducersPerRound + 3; i++, j += 10)
	{
		client.approve_producer(accounts[i], accounts[i], j);
	}
	//5.结束本轮剩余出块
	client.produce_blocks(18);

	//6.第3-9轮出块，校验每个生产者所对应的块数是否正确
	std::map<AccountName, int> account_block;
	for (auto run = 0; run < 9; run++)
	{
		for (auto i = 19; i < 23; i++)
		{
			client.make_pow_producer(accounts[i], setPowTest::none);
		}
		client.produce_blocks_Number(config::TotalProducersPerRound, account_block);

	}
	for (auto i = 0; i < 16; i++)
	{
		BOOST_CHECK_MESSAGE(account_block[AccountName(accounts[0].address)], 9);
	}

	BOOST_REQUIRE_EQUAL(account_block[AccountName(accounts[16].address)] , 2);
	BOOST_REQUIRE_EQUAL(account_block[AccountName(accounts[17].address)] , 3);
	BOOST_REQUIRE_EQUAL(account_block[AccountName(accounts[18].address)] , 4);

	//7.给虚拟赛跑的生产者增加投票信息使投票大于等于60票
    client.approve_producer(accounts[16], accounts[16],40);
	client.produce_blocks(config::TotalProducersPerRound);

	//8.出三轮块
	for (auto run = 0; run < 3; run++)
	{
		for (auto i = 19; i < 23; i++)
		{
			client.make_pow_producer(accounts[i], setPowTest::none);
		}
		client.produce_blocks_Number(config::TotalProducersPerRound, account_block);

	}
	for (auto i = 0; i < 16; i++)
	{
		BOOST_CHECK_MESSAGE(account_block[AccountName(accounts[0].address)], 12);
	}
	BOOST_REQUIRE(account_block[AccountName(accounts[16].address)] == 5);
	ctrace << "17 : " <<account_block[AccountName(accounts[17].address)];
	ctrace << "18 : " << account_block[AccountName(accounts[18].address)];
	BOOST_REQUIRE_MESSAGE(account_block[AccountName(accounts[17].address)], 4);
	BOOST_REQUIRE_MESSAGE(account_block[AccountName(accounts[18].address)],4);
}

BOOST_AUTO_TEST_CASE(dtMakeBlockETHTest)
{
	cout << "dtMakeBlockETHTest" << endl;

	//1.创建一个client
	//g_logVerbosity = 13;
	//创建生产者
	DposTestClient client;
	BOOST_REQUIRE(client.get_accounts().size() >= 1);

	//2.注册一个dpos生产者，获取balance
	auto& account = client.get_accounts()[0];
	client.make_producer(account);

	//3.第一轮出块
	client.produce_blocks(config::TotalProducersPerRound);
	u256 start_balance = client.balance(AccountName(account.address));

	//4.第二轮出块
	std::map<AccountName, int> account_block;
	client.produce_blocks_Number(config::TotalProducersPerRound*10, account_block);

	//5.获取balance和出块数
	u256 end_balance = client.balance(AccountName(account.address));
	int blockNums = account_block[AccountName(account.address)];

	//6.比较balance和出块数
	BOOST_REQUIRE((end_balance - start_balance) / blockNums == (u256)5000000000000000000 );

}

BOOST_AUTO_TEST_SUITE_END()




}
}



