#include "P2PTestRobot.hpp"
#include <string>

//#include <utils/json_spirit/json_spirit_value.h>
//#include <utils/json_spirit/json_spirit_reader_template.h>
//#include <utils/json_spirit/json_spirit_writer_template.h>
//#include <libweb3jsonrpc/JsonHelper.h>

#include <libethereum/CommonNet.h>
#include <libp2p/common.h>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
//#include <libp2p/FakeHost.h>


using namespace P2PTest;
using namespace dev;
using namespace dev::eth;

P2PTestRobot::P2PTestRobot(dev::p2p::FakeHost& _host) : m_host(_host)
{
	m_idOffset = UserPacket;

	const BlockChain& bc = m_client.getBlockChain();
	m_hostNetworkId = bc.chainParams().networkID;
	m_chainTotalDifficulty = bc.details().totalDifficulty;
	m_chainCurrentHash = bc.currentHash();
	m_chainGenesisHash = bc.genesisHash();
	m_lastIrrBlock = bc.getIrreversibleBlock();

	m_nodeID = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");


}

P2PTestRobot::~P2PTestRobot()
{

}

void P2PTestRobot::requestBlockHeaders(unsigned _startNumber, unsigned _count, unsigned _skip, bool _reverse)
{
	RLPStream s;
	prep(s, GetBlockHeadersPacket, 4) << _startNumber << _count << _skip << (_reverse ? 1 : 0);
	ctrace << "Requesting " << _count << " block headers starting from " << _startNumber << (_reverse ? " in reverse" : "") << " to: " << this->m_nodeID;
	//m_lastAskedHeaders = _count;
	sealAndSend(s);
}

void P2PTestRobot::requestBlockHeaders(dev::h256 const& _startHash, unsigned _count, unsigned _skip, bool _reverse)
{
	RLPStream s;
	//s.appendRaw(bytes(1, GetBlockHeadersPacket + 4)).appendList(_args) << _startHash << _count << _skip << (_reverse ? 1 : 0);
	prep(s, GetBlockHeadersPacket, 4) << _startHash << _count << _skip << (_reverse ? 1 : 0);
	//clog(NetMessageDetail) << "Requesting " << _count << " block headers starting from " << _startHash << (_reverse ? " in reverse" : "");
	ctrace << "Requesting " << _count << " block headers starting from " << _startHash << (_reverse ? " in reverse" : "");
	sealAndSend(s);
}

void P2PTestRobot::requestStatus(u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash, u256 _lastIrrBlock)
{
	const unsigned m_hostProtocolVersion = 63;

	RLPStream s;
	prep(s, StatusPacket, 6)
		<< m_hostProtocolVersion
		<< _hostNetworkId
		<< _chainTotalDifficulty
		<< _chainCurrentHash
		<< _chainGenesisHash
		<< _lastIrrBlock
		;

	sealAndSend(s);
}

RLPStream& P2PTestRobot::prep(RLPStream& _s, unsigned _id, unsigned _args)
{
	return _s.appendRaw(bytes(1, _id + m_idOffset)).appendList(_args);
}

void P2PTestRobot::sealAndSend(RLPStream& _s)
{
	bytes b;
	_s.swapOut(b);
	sendToHost(move(b));
}

void P2PTestRobot::sendToHost(bytes& _s)
{
	m_host.sendToHost(_s);
}

void P2PTestRobot::recvFromHost(bytes& _s)
{
	//m_host.recvFromHost(id,_s);
	NodeID  id("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
	uint16_t hProtocolId = 63;
	//PacketType type = UserPacket;

	bytesConstRef frame(_s.data(), _s.size());


	auto packetType = (PacketType)RLP(frame.cropped(0, 1)).toInt<unsigned>();

	if (packetType < UserPacket)
		return;

	RLP r(frame.cropped(1));
	bool ok = interpret(packetType - m_idOffset, r);

	if (!ok)
		clog(NetWarn) << "Couldn't interpret packet." << RLP(r);

}

bool P2PTestRobot::interpret(unsigned _id, RLP const& _r)
{
	try
	{
		switch (_id)
		{
		case StatusPacket:
		{
			unsigned _protocolVersion = _r[0].toInt<unsigned>();
			u256 _networkId = _r[1].toInt<u256>();
			u256 _totalDifficulty = _r[2].toInt<u256>();
			h256 _latestHash = _r[3].toHash<h256>();
			h256 _genesisHash = _r[4].toHash<h256>();
			uint32_t _lastIrrBlock = _r[5].toInt<u256>().convert_to<uint32_t>();

			break;
		}
		case TransactionsPacket:
		{
			//observer->onPeerTransactions(dynamic_pointer_cast<EthereumPeer>(dynamic_pointer_cast<EthereumPeer>(shared_from_this())), _r);
			break;
		}
		case GetBlockHeadersPacket:
		{
			/// Packet layout:
			/// [ block: { P , B_32 }, maxHeaders: P, skip: P, reverse: P in { 0 , 1 } ]
			const auto blockId = _r[0];
			const auto maxHeaders = _r[1].toInt<u256>();
			const auto skip = _r[2].toInt<u256>();
			const auto reverse = _r[3].toInt<bool>();

			//auto numHeadersToSend = maxHeaders <= c_maxHeadersToSend ? static_cast<unsigned>(maxHeaders) : c_maxHeadersToSend;

			//if (skip > std::numeric_limits<unsigned>::max() - 1)
			//{
			//	clog(NetAllDetail) << "Requested block skip is too big: " << skip;
			//	break;
			//}

			//pair<bytes, unsigned> const rlpAndItemCount = hostData->blockHeaders(blockId, numHeadersToSend, skip, reverse);

			//RLPStream s;
			//prep(s, BlockHeadersPacket, rlpAndItemCount.second).appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
			//sealAndSend(s);
			//addRating(0);
			break;
		}
		case BlockHeadersPacket:
		{
			//if (m_asking != Asking::BlockHeaders)
			//	clog(NetImpolite) << "Peer giving us block headers when we didn't ask for them.";
			//else
			//{
			//	setIdle();
			//	observer->onPeerBlockHeaders(dynamic_pointer_cast<EthereumPeer>(shared_from_this()), _r);
			//}
			break;
		}
		case GetBlockBodiesPacket:
		{
			unsigned count = static_cast<unsigned>(_r.itemCount());
			clog(NetMessageSummary) << "GetBlockBodies (" << dec << count << "entries)";

			//if (!count)
			//{
			//	clog(NetImpolite) << "Zero-entry GetBlockBodies: Not replying.";
			//	addRating(-10);
			//	break;
			//}

			//pair<bytes, unsigned> const rlpAndItemCount = hostData->blockBodies(_r);

			//addRating(0);
			//RLPStream s;
			//prep(s, BlockBodiesPacket, rlpAndItemCount.second).appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
			//sealAndSend(s);
			break;
		}
		case BlockBodiesPacket:
		{
			//if (m_asking != Asking::BlockBodies)
			//	clog(NetImpolite) << "Peer giving us block bodies when we didn't ask for them.";
			//else
			//{
			//	setIdle();
			//	observer->onPeerBlockBodies(dynamic_pointer_cast<EthereumPeer>(shared_from_this()), _r);
			//}
			break;
		}
		case NewBlockPacket:
		{
			//observer->onPeerNewBlock(dynamic_pointer_cast<EthereumPeer>(shared_from_this()), _r);
			break;
		}
		case NewBlockHashesPacket:
		{
			//unsigned itemCount = _r.itemCount();

			//clog(NetMessageSummary) << "BlockHashes (" << dec << itemCount << "entries)" << (itemCount ? "" : ": NoMoreHashes");

			//if (itemCount > c_maxIncomingNewHashes)
			//{
			//	disable("Too many new hashes");
			//	break;
			//}

			//vector<pair<h256, u256>> hashes(itemCount);
			//for (unsigned i = 0; i < itemCount; ++i)
			//	hashes[i] = std::make_pair(_r[i][0].toHash<h256>(), _r[i][1].toInt<u256>());

			//observer->onPeerNewHashes(dynamic_pointer_cast<EthereumPeer>(shared_from_this()), hashes);
			break;
		}
		case GetNodeDataPacket:
		{
			//unsigned count = static_cast<unsigned>(_r.itemCount());
			//if (!count)
			//{
			//	clog(NetImpolite) << "Zero-entry GetNodeData: Not replying.";
			//	addRating(-10);
			//	break;
			//}
			//clog(NetMessageSummary) << "GetNodeData (" << dec << count << " entries)";

			//strings const data = hostData->nodeData(_r);

			//addRating(0);
			//RLPStream s;
			//prep(s, NodeDataPacket, data.size());
			//for (auto const& element : data)
			//	s.append(element);
			//sealAndSend(s);
			break;
		}
		case GetReceiptsPacket:
		{
			//unsigned count = static_cast<unsigned>(_r.itemCount());
			//if (!count)
			//{
			//	clog(NetImpolite) << "Zero-entry GetReceipts: Not replying.";
			//	addRating(-10);
			//	break;
			//}
			//clog(NetMessageSummary) << "GetReceipts (" << dec << count << " entries)";

			//pair<bytes, unsigned> const rlpAndItemCount = hostData->receipts(_r);

			//addRating(0);
			//RLPStream s;
			//prep(s, ReceiptsPacket, rlpAndItemCount.second).appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
			//sealAndSend(s);
			break;
		}
		case NodeDataPacket:
		{
			//if (m_asking != Asking::NodeData)
			//	clog(NetImpolite) << "Peer giving us node data when we didn't ask for them.";
			//else
			//{
			//	setIdle();
			//	observer->onPeerNodeData(dynamic_pointer_cast<EthereumPeer>(shared_from_this()), _r);
			//}
			break;
		}
		case ReceiptsPacket:
		{
			//if (m_asking != Asking::Receipts)
			//	clog(NetImpolite) << "Peer giving us receipts when we didn't ask for them.";
			//else
			//{
			//	setIdle();
			//	observer->onPeerReceipts(dynamic_pointer_cast<EthereumPeer>(shared_from_this()), _r);
			//}
			break;
		}
		default:
			return false;
		}
	}
	catch (Exception const&)
	{
		clog(NetWarn) << "Peer causing an Exception:" << boost::current_exception_diagnostic_information() << _r;
	}
	catch (std::exception const& _e)
	{
		clog(NetWarn) << "Peer causing an exception:" << _e.what() << _r;
	}

	return true;
}

void P2PTestRobot::run()
{
	m_host.connectToHost(m_nodeID);
	requestStatus(m_hostNetworkId, m_chainTotalDifficulty, m_chainCurrentHash, m_chainGenesisHash, m_lastIrrBlock);

	while (true)
	{
		requestBlockHeaders(1, 1, 0, false);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
		cout << "P2PTestRobot::run()" << endl;
	}
}

//void P2PTestRobot::loadConfig()
//{
//	std::string configPath = "P2PTestRobotConfig.json";
//	boost::filesystem::path _path(configPath);
//	if (_path.is_relative())
//	{
//		std::string filePath(boost::filesystem::current_path().string());
//		_path = boost::filesystem::path(filePath + "/" + configPath);
//	}
//	std::string s = dev::contentsString(_path);
//	if (s.size() == 0)
//	{
//		BOOST_THROW_EXCEPTION(std::runtime_error("Config file doesn't exist!"));
//	}
//	json_spirit::mValue v;
//	json_spirit::read_string(s, v);
//	json_spirit::mObject& json_config = v.get_obj();
//
//	if (!json_config.count("attackType") || !json_config.count("interval"))
//	{
//		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid config file!"));
//	}
//}

