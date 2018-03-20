#include "P2PTestClient.hpp"
#include <libp2p/Common.h>

using namespace P2PTest;
using namespace dev::p2p;
using fc::ecc::private_key;


P2PTestClient::P2PTestClient() 
{
	TestBlock tb(TestBlockChain::defaultDposGenesisBlock());
	m_tbc = std::make_shared<TestBlockChain>(tb, true);
	m_producer_plugin = m_tbc->getProducerPluginPtr();
	m_chain_controller = &m_producer_plugin->get_chain_controller();

	m_idOffset = UserPacket;

	BlockChain& bc = m_tbc->getInterface();
	m_hostNetworkId = bc.chainParams().networkID;
	m_chainTotalDifficulty = bc.details().totalDifficulty;
	m_chainCurrentHash = bc.currentHash();
	m_chainGenesisHash = bc.genesisHash();
	m_lastIrrBlock = bc.getIrreversibleBlock();

}

P2PTestClient::~P2PTestClient()
{

}

void P2PTestClient::importBlocksFromFile(boost::filesystem::path& _path, string& _chainName)
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
			if (_chainName.size() > 0 && blObj["chainname"] == _chainName)
			{
				//TestBlock blockFromRlp;
				try
				{
					string str = blObj["rlp"].get_str();

					//bytesConstRef blRlp((byte*)str.data(), str.size());
					bytes ss = fromHex(str.substr(0, 2) == "0x" ? str.substr(2) : str, WhenError::Throw);
					//BlockHeader  bh(ss);

					addBlock(ss);

				}
				catch (Exception const& _e)
				{
					cnote << "state sync or block import did throw an exception: " << diagnostic_information(_e);
				}
			}
		}

	}
}

bool P2PTestClient::addBlock(bytes const& _block)
{
	TestBlock tb(RLP(_block).toString());
	m_tbc->addBlock(tb);

	return true;
}

bytes P2PTestClient::produceBlock()
{
	auto slot = 1;
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

const private_key& P2PTestClient::get_private_key(const AccountName& address) const
{
	return m_private_keys.at(address);
}