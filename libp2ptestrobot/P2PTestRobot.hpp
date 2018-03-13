#pragma once
#include <libdevcore/FixedHash.h>
#include <libdevcore/RLP.h>
#include <libp2p/FakeHost.h>
#include "P2PTestClient.hpp"
//class dev::p2p::FakeHost;


namespace P2PTest {

	using namespace dev;

class P2PTestRobot 
{
public:
	P2PTestRobot(dev::p2p::FakeHost& _host);
	~P2PTestRobot();
	//void P2PTestRobot::loadConfig();
	void requestBlockHeaders(dev::h256 const& _startHash, unsigned _count, unsigned _skip, bool _reverse);
	RLPStream& prep(RLPStream& _s, unsigned _id, unsigned _args);

	void P2PTestRobot::requestStatus(u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash, u256 _lastIrrBlock);

	void sealAndSend(dev::RLPStream& _s);

	void sendToHost(bytes& _s);
	void recvFromHost(bytes& _s);

	void run();

	unsigned m_idOffset;
	dev::p2p::FakeHost& m_host;
	P2PTestClient m_client;
};



}