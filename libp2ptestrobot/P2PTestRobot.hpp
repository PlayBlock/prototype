#pragma once
#include <libdevcore/FixedHash.h>
#include <libdevcore/RLP.h>


namespace P2PTest {

	using namespace dev;

class P2PTestRobot 
{
public:
	P2PTestRobot();
	~P2PTestRobot();
	void P2PTestRobot::loadConfig();
	void requestBlockHeaders(dev::h256 const& _startHash, unsigned _count, unsigned _skip, bool _reverse);
	RLPStream& prep(RLPStream& _s, unsigned _id, unsigned _args);

	void sealAndSend(dev::RLPStream& _s);
	void sendToHost(bytes& _s);

	unsigned m_idOffset;
};



}