#include "P2PTestClient.hpp"

using namespace P2PTest;
using fc::ecc::private_key;


P2PTestClient::P2PTestClient() :
	m_bc(TestBlockChain::defaultDposGenesisBlock(), true),
	m_producer_plugin(m_bc.getProducerPluginPtr()),
	m_chain(m_producer_plugin->get_chain_controller())
{

}

P2PTestClient::~P2PTestClient()
{

}

bool P2PTestClient::addBlock(bytes const& _block)
{
	TestBlock tb(RLP(_block).toString());
	m_bc.addBlock(tb);

	return true;
}

bytes P2PTestClient::produceBlock()
{
	auto slot = 1;
	auto producer = m_chain.get_scheduled_producer(slot);
	while (producer == AccountName())
		producer = m_chain.get_scheduled_producer(++slot);

	//auto producer =  _chain.get_scheduled_producer(slot);
	auto& private_key = get_private_key(producer);
	m_working.dposMine(m_bc, m_chain.get_slot_time(slot), producer, private_key);
	m_bc.addBlock(m_working);
	
	bytes retBytes = m_working.bytes();
	m_working = TestBlock();

	return retBytes;
}

const private_key& P2PTestClient::get_private_key(const AccountName& address) const
{
	return m_private_keys.at(address);
}