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
	const BlockChain& getBlockChain() { return m_bc.getInterface(); }

private:
	bool addBlock(bytes const& _block);
	bytes produceBlock();

	const private_key& get_private_key(const AccountName& address) const;

private:

	TestBlockChain m_bc;
	TestBlock m_working;
	std::shared_ptr<producer_plugin> m_producer_plugin;
	eth::chain::chain_controller& m_chain;
	//Accounts m_accounts;
	std::map<types::AccountName, fc::ecc::private_key> m_private_keys;
};

}