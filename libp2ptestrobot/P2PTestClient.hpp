#pragma once
#include <test/tools/libtesteth/BlockChainHelper.h>
#include <libproducer/chain_controller.hpp>
#include <libproducer/producer_plugin.hpp>


namespace P2PTest {

using namespace dev::test;
using namespace dev::eth;

using fc::ecc::private_key;

class P2PTestClient
{
public:

	P2PTestClient();
	~P2PTestClient();
	const BlockChain& getBlockChain() { return m_tbc->getInterface(); }

private:
	void importBlocksFromFile(boost::filesystem::path& _path, string& _chainName=string());

	bool addBlock(bytes const& _block);
	bytes produceBlock();

	const private_key& get_private_key(const AccountName& address) const;

public:
	unsigned m_idOffset;

	u256 m_hostNetworkId;
	u256 m_chainTotalDifficulty;
	h256 m_chainCurrentHash;
	h256 m_chainGenesisHash;
	u256 m_lastIrrBlock;

private:

	std::shared_ptr<TestBlockChain> m_tbc;
	TestBlock m_working;
	std::shared_ptr<producer_plugin> m_producer_plugin;
	eth::chain::chain_controller* m_chain_controller;
	//Accounts m_accounts;
	std::map<types::AccountName, fc::ecc::private_key> m_private_keys;


};

}