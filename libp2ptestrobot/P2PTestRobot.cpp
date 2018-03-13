#include "P2PTestRobot.hpp"
#include <string>

#include <utils/json_spirit/json_spirit_value.h>
#include <utils/json_spirit/json_spirit_reader_template.h>
#include <utils/json_spirit/json_spirit_writer_template.h>
#include <libweb3jsonrpc/JsonHelper.h>

#include <libethereum/CommonNet.h>
#include <libp2p/common.h>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
//#include <libp2p/FakeHost.h>


using namespace P2PTest;
using namespace dev;
using namespace dev::eth;
using namespace dev::p2p;

P2PTestRobot::P2PTestRobot(dev::p2p::FakeHost& _host) : m_host(_host)
{
	m_idOffset = UserPacket;
}

P2PTestRobot::~P2PTestRobot()
{

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
	m_host.recvFromHost(_s);
}

void P2PTestRobot::run()
{
	u256 _hostNetworkId = u256();
	u256 _chainTotalDifficulty = u256();
	h256 _chainCurrentHash = h256();
	h256 _chainGenesisHash = h256();
	u256 _lastIrrBlock = u256();

	NodeID  m_remote("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");

	m_host.connectToHost(m_remote);

	while (true)
	{
		requestStatus(_hostNetworkId, _chainTotalDifficulty, _chainCurrentHash, _chainGenesisHash, _lastIrrBlock);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10000));
	}
}

void P2PTestRobot::loadConfig()
{
	std::string configPath = "P2PTestRobotConfig.json";
	boost::filesystem::path _path(configPath);
	if (_path.is_relative())
	{
		std::string filePath(boost::filesystem::current_path().string());
		_path = boost::filesystem::path(filePath + "/" + configPath);
	}
	std::string s = dev::contentsString(_path);
	if (s.size() == 0)
	{
		BOOST_THROW_EXCEPTION(std::runtime_error("Config file doesn't exist!"));
	}
	json_spirit::mValue v;
	json_spirit::read_string(s, v);
	json_spirit::mObject& json_config = v.get_obj();

	if (!json_config.count("attackType") || !json_config.count("interval"))
	{
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid config file!"));
	}
}

