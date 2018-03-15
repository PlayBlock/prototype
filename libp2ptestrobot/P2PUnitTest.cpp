#include "P2PUnitTest.h"
#include <string> 
#include <libethereum/CommonNet.h>
#include <libp2p/common.h> 
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

using namespace dev;
using namespace dev::eth;

namespace P2PTest {

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
		if (packetType < UserPacket)
			return; 
		RLP r(frame.cropped(1));
		interpret(packetType - UserPacket, r); 
	}

	void P2PHostProxy::connectToHost(NodeID const& _id)
	{
		m_host.connectToHost(_id);
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

}