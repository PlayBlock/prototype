#include "P2PTestClient.hpp"
#include <libp2p/Common.h>
#include <libethereum/Client.h>

using namespace P2PTest;
using namespace dev::p2p;
using fc::ecc::private_key;


P2PTestClient::P2PTestClient() 
{
	TestBlock tb(TestBlockChain::P2PTestGenesisBlock());
	m_tbc = std::make_shared<TestBlockChain>(tb, true);
	m_producer_plugin = m_tbc->getProducerPluginPtr();
	m_chain_controller = &m_producer_plugin->get_chain_controller();

	if (!g_p2ptestPath.empty() && !g_BlockChainName.empty())
		importBlocksFromFile(g_p2ptestPath, g_BlockChainName);

	m_idOffset = UserPacket;

	BlockChain& bc = m_tbc->getInterface();
	m_hostNetworkId = bc.chainParams().networkID;
	m_chainTotalDifficulty = bc.details().totalDifficulty;
	m_chainCurrentHash = bc.currentHash();
	m_chainGenesisHash = bc.genesisHash();
	m_lastIrrBlock = bc.getIrreversibleBlock();

	//load private keys
	loadPrivateKeys();
}

P2PTestClient::~P2PTestClient()
{

}

void P2PTestClient::loadPrivateKeys()
{
	// read all prikey-address pair

	auto& params = m_tbc->getInterface().chainParams();
	for (auto& key : params.privateKeys)
	{
		m_private_keys.insert(key);
	}
}

void P2PTestClient::importBlocksFromFile(boost::filesystem::path& _path, string _notChainName)
{
	//g_logVerbosity = 14;
	//boost::filesystem::path  boostTestPath = g_p2ptestPath;// "./p2ptest.json";

	json_spirit::mValue v;
	string const s = asString(dev::contents(_path));
	json_spirit::read_string(s, v);

	for (auto const& i : v.get_obj())
	{
		string const& testname = i.first;
		json_spirit::mObject const& inputTest = i.second.get_obj();

		//判断_bc中的genesis与预导入块的json中的是否相同
		/*auto gsRLP = inputTest.at("genesisRLP").get_str();
		bytes paramGenesis = _bc.chainParams().genesisBlock();
		ctrace << RLP(paramGenesis);

		bytes ll = fromHex(gsRLP.substr(0, 2) == "0x" ? gsRLP.substr(2) : gsRLP, WhenError::Throw);
		ctrace << RLP(ll);*/

		for (auto const& bl : inputTest.at("blocks").get_array())
		{
			json_spirit::mObject blObj = bl.get_obj();
			if (_notChainName.size() > 0 && (blObj.count("chainname") == 0 || !(blObj["chainname"].get_str() == _notChainName)))
			{
				//TestBlock blockFromRlp;
				try
				{
					string blockStr = blObj["rlp"].get_str();
					m_tbc->addBlock(blockStr);

				}
				catch (Exception const& _e)
				{
					cnote << "state sync or block import did throw an exception: " << diagnostic_information(_e);
				}
			}
		}

	}
}

bytes P2PTestClient::produceBlock(uint32_t slot)
{
	auto producer = m_chain_controller->get_scheduled_producer(slot);
	while (producer == AccountName())
		producer = m_chain_controller->get_scheduled_producer(++slot);

	//auto producer =  _chain.get_scheduled_producer(slot);
	auto& private_key = get_private_key(producer);
	m_working.dposMine(*m_tbc, m_chain_controller->get_slot_time(slot), producer, private_key);
	m_tbc->addBlock(m_working);
	
	bytes retBytes = m_working.bytes();
	m_working = TestBlock();

	return retBytes;
}

bytes P2PTestClient::produceBlock(uint64_t time)
{
	auto slot = m_chain_controller->get_slot_at_time(fc::time_point_sec(time));

	auto producer = m_chain_controller->get_scheduled_producer(slot);
	if (producer == AccountName())
		return bytes();

	//auto producer =  _chain.get_scheduled_producer(slot);
	auto& private_key = get_private_key(producer);
	m_working.dposMine(*m_tbc, m_chain_controller->get_slot_time(slot), producer, private_key);
	m_tbc->addBlock(m_working);

	bytes retBytes = m_working.bytes();
	m_working = TestBlock();

	return retBytes;
}

const private_key& P2PTestClient::get_private_key(const AccountName& address) const
{
	return m_private_keys.at(address);
}
