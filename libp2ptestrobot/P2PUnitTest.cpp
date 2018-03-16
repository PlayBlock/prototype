#include "P2PUnitTest.h"
#include <string> 
#include <libethereum/CommonNet.h>
#include <libp2p/common.h> 
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

using namespace dev;
using namespace dev::eth;
namespace P2PTest {

int P2PHostProxy::m_currTest = -1;
std::vector<P2PUnitTest*> P2PHostProxy::m_unitTestList;

#if defined(_WIN32)
	BOOL P2PHostProxy::CtrlHandler(DWORD fdwCtrlType)
	{
		switch (fdwCtrlType)
		{
			/* Handle the CTRL-C signal. */
		case CTRL_C_EVENT:
			printf("CTRL_C_EVENT \n");
			switchUnitTest((m_currTest+1) % m_unitTestList.size());
			break;
		case CTRL_BREAK_EVENT:
			printf("CTRL_BREAK_EVENT \n");
			break;
		default:
			return FALSE;
		}
		return (TRUE);
	}
#else
	void switchSignalHandler(int signum) 
	{ 
		ctrace << "switchSignalHandler"; 
		switchUnitTest((m_currTest + 1) % m_unitTestList.size());
	}

#endif

	P2PHostProxy::P2PHostProxy(dev::p2p::FakeHost& _h) : m_host(_h)
	{

#if defined(_WIN32)
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)P2PHostProxy::CtrlHandler, TRUE);
#else
		signal(SIGBREAK, &P2PHostProxy::switchSignalHandler);
#endif
	}

	P2PUnitTest* P2PHostProxy::getCurrUnitTest() const
	{
		if (m_currTest != -1)
		{
			return m_unitTestList[m_currTest];
		}
		return nullptr;
	}


	void P2PHostProxy::registerAllUnitTest()
	{
		//注册用例TestDrive
		registerUnitTest(new P2PTestDriveUnitTest(*this));
		registerUnitTest(new P2PTestRequestHeaderAttack(*this));

		switchUnitTest(0);
	}

	void P2PHostProxy::requestStatus(u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash, u256 _lastIrrBlock)
	{

	}

	void P2PHostProxy::requestBlockHeaders(dev::h256 const& _startHash, unsigned _count, unsigned _skip, bool _reverse)
	{
		RLPStream s;
		prep(s, GetBlockHeadersPacket, 4) << _startHash << _count << _skip << (_reverse ? 1 : 0);
		ctrace << "Requesting " << _count << " block headers starting from " << _startHash << (_reverse ? " in reverse" : "");
		sealAndSend(s);
	}

	void P2PHostProxy::requestBlockHeaders(unsigned _startNumber, unsigned _count, unsigned _skip, bool _reverse)
	{
		RLPStream s;
		prep(s, GetBlockHeadersPacket, 4) << _startNumber << _count << _skip << (_reverse ? 1 : 0);
		ctrace << "Requesting " << _count << " block headers starting from " << _startNumber << (_reverse ? " in reverse" : "") << " to: " << m_host.id();
		sealAndSend(s);
	}

	dev::RLPStream& P2PHostProxy::prep(RLPStream& _s, unsigned _id, unsigned _args)
	{
		return _s.appendRaw(bytes(1, _id + UserPacket)).appendList(_args);
	}

	void P2PHostProxy::sealAndSend(dev::RLPStream& _s)
	{
		bytes b;
		_s.swapOut(b);
		sendToHost(move(b));
	}

	void P2PHostProxy::sendToHost(bytes& _s)
	{
		m_host.sendToHost(_s);
	}

	void P2PHostProxy::recvFromHost(bytes& _s)
	{
		bytesConstRef frame(_s.data(), _s.size());
		auto packetType = (PacketType)RLP(frame.cropped(0, 1)).toInt<unsigned>();
		RLP r(frame.cropped(1));

		if (packetType < UserPacket)
		{
			interpretProtocolPacket(packetType, r);
			return;
		}
		interpret(packetType - UserPacket, r);
	}

	void P2PHostProxy::connectToHost(NodeID const& _id)
	{
		m_host.connectToHost(_id);
	}

	void P2PHostProxy::onSessionClosed(NodeID const& _id)
	{
		ctrace << "P2PHostProxy::onSessionClosed";
	}

	void P2PHostProxy::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		auto pUnitTest = getCurrUnitTest();
		if (nullptr != pUnitTest)
		{
			pUnitTest->interpretProtocolPacket(_t, _r);
		}
	}

	void P2PHostProxy::interpret(unsigned _id, RLP const& _r)
	{
		auto pUnitTest = getCurrUnitTest();
		if (nullptr != pUnitTest)
		{
			pUnitTest->interpret(_id, _r);
		}
	}

	void P2PHostProxy::step()
	{
		auto pUnitTest = getCurrUnitTest();
		if (nullptr != pUnitTest)
		{
			pUnitTest->step();
		}
	}

	void P2PHostProxy::registerUnitTest(P2PUnitTest* _unit)
	{
		m_unitTestList.push_back(_unit);
	}

	void P2PHostProxy::switchUnitTest(int i)
	{
		if (i == -1)
			return;

		if (i >= m_unitTestList.size())
			return;
		ctrace << "switch from " << m_currTest << " to " << i;
		if (m_currTest != -1)
		{//离开旧的UnitTest
			m_unitTestList[m_currTest]->destroy();
		}

		//进入新的UnitTest
		m_currTest = i;
		m_unitTestList[m_currTest]->init();
	}

	std::string P2PTestDriveUnitTest::name() const
	{
		return "TestDrive!!!";
	}

	void P2PTestDriveUnitTest::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);

		ctrace << "P2PTestDriveUnitTest::init";
	}

	void P2PTestDriveUnitTest::destroy()
	{
		ctrace << "P2PTestDriveUnitTest::destroy";
	}

	void P2PTestDriveUnitTest::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		ctrace << "P2PTestDriveUnitTest::interpretProtocolPacket";
	}

	void P2PTestDriveUnitTest::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestDriveUnitTest::interpret";
	}

	void P2PTestDriveUnitTest::step()
	{
		m_hostProxy.requestBlockHeaders(1, 1, 0, false);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
		ctrace << "P2PTestDriveUnitTest::run()";
	}


	//用例名称
	std::string P2PTestRequestHeaderAttack::name() const
	{
		return "P2PTestRequestHeaderAttack";
	}

	//用于用例初始化
	void P2PTestRequestHeaderAttack::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);

		ctrace << "P2PTestRequestHeaderAttack::init";
	}

	//用例销毁
	void P2PTestRequestHeaderAttack::destroy()
	{
		ctrace << "P2PTestRequestHeaderAttack::destroy";
	}

	//用来解析传来的协议包
	void P2PTestRequestHeaderAttack::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		switch (_t)
		{
		case DisconnectPacket:
		{
			string reason = "Unspecified";
			auto r = (DisconnectReason)_r[0].toInt<int>();
			if (!_r[0].isInt())
				ctrace << "Disconnect (reason: no reason)";
			else
			{
				reason = reasonOf(r);
				ctrace << "Disconnect (reason: " << reason << ")";
			}
			//m_hostProxy.switchUnitTest();
			break;
		}

		case GetPeersPacket:
		case PeersPacket:
			break;
		default:
			return;
		}
		return;
	}

	void P2PTestRequestHeaderAttack::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestRequestHeaderAttack::interpret";
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
			case BlockHeadersPacket:
			{
				size_t itemCount = _r.itemCount();
				ctrace << "BlocksHeaders (" << dec << itemCount << "entries)" << (itemCount ? "" : ": NoMoreHeaders");
				BlockHeader header(_r[0].data(), HeaderData);
				unsigned blockNumber = static_cast<unsigned>(header.number());

				ctrace << "start blockNumber: " << blockNumber;
				break;
			}
			case NewBlockPacket:
			{
				//observer->onPeerNewBlock(dynamic_pointer_cast<EthereumPeer>(shared_from_this()), _r);
				break;
			}
			default:
				return;
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


	}



	//在host线程
	void P2PTestRequestHeaderAttack::step()
	{
		m_hostProxy.requestBlockHeaders(1, 1, 0, false);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "P2PTestRequestHeaderAttack::step()";
	}

}