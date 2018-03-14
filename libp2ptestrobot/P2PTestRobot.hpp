#pragma once
#include <libdevcore/FixedHash.h>
#include <libdevcore/RLP.h>
#include <libp2p/FakeHost.h>
#include "P2PTestClient.hpp"
//class dev::p2p::FakeHost;


namespace P2PTest {

	using namespace dev;
	using namespace dev::p2p;

class P2PTestRobot 
{
public:
	P2PTestRobot(dev::p2p::FakeHost& _host);
	~P2PTestRobot();
	//void P2PTestRobot::loadConfig();
	void requestBlockHeaders(dev::h256 const& _startHash, unsigned _count, unsigned _skip, bool _reverse);
	void requestBlockHeaders(unsigned _startNumber, unsigned _count, unsigned _skip, bool _reverse);

	
	RLPStream& prep(RLPStream& _s, unsigned _id, unsigned _args);

	void P2PTestRobot::requestStatus(u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash, u256 _lastIrrBlock);

	void sealAndSend(dev::RLPStream& _s);

	void sendToHost(bytes& _s);
	void recvFromHost(bytes& _s);
	bool interpret(unsigned _id, RLP const& _r);


	void run();

	unsigned m_idOffset;
	dev::p2p::FakeHost& m_host;
	P2PTestClient m_client;


	u256 m_hostNetworkId;
	u256 m_chainTotalDifficulty;
	h256 m_chainCurrentHash;
	h256 m_chainGenesisHash;
	u256 m_lastIrrBlock;

	NodeID m_nodeID;
};



}