#include "P2PUnitTest.h"
#include <string> 
#include <libethereum/CommonNet.h>
#include <libp2p/Common.h> 
#include <libp2p/Common.h> 
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 
#include <libdevcore/SHA3.h>
#include <libethereum/Client.h>

#define BOOST_TEST_NO_MAIN
#include <boost/test/included/unit_test.hpp>

using namespace dev;
using namespace dev::eth;
namespace P2PTest {

int P2PHostProxy::m_currTest = -1;
std::vector<P2PUnitTest*> P2PHostProxy::m_unitTestList;

//用于用例初始化
void P2PUnitTest::init()
{
	NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
	m_hostProxy.connectToHost(id);

	const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
	m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
		bc.currentHash(), bc.genesisHash(),	bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());

	ctrace << "P2PTestNewBlockAttack::init";
}



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
	void P2PHostProxy::switchSignalHandler(int signum) 
	{ 
		ctrace << "switchSignalHandler"; 
		switchUnitTest((m_currTest + 1) % m_unitTestList.size());
	}

#endif

	P2PHostProxy::P2PHostProxy(dev::p2p::FakeHost& _h, boost::asio::io_service& _ioService) : m_host(_h), m_ioService(_ioService)
	{
		m_timer = make_shared<boost::asio::deadline_timer>(m_ioService);
		m_timer->expires_from_now(boost::posix_time::milliseconds(100));
	}

	P2PUnitTest* P2PHostProxy::getCurrUnitTest() const
	{
		if (m_currTest != -1)
		{
			return m_unitTestList[m_currTest];
		}
		return nullptr;
	}

	void P2PHostProxy::registerAttackUnitTest()
	{

#if defined(_WIN32)
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)P2PHostProxy::CtrlHandler, TRUE);
#else
		signal(SIGQUIT, &P2PHostProxy::switchSignalHandler);
#endif
		/*
		* 攻击测试
		*/
		///eth需要可以正常出块，config.json文件中需要有AccountName
		///eth运行参数 --p2pfiller-path  P2PTestPackageAttack.json --chainname "A"
		registerUnitTest(new P2PTestRequestHeaderAttack(*this));
		registerUnitTest(new P2PTestSendHeaderAttack(*this));		
		registerUnitTest(new P2PTestNewBlockAttack(*this));
		registerUnitTest(new P2PTestStatusPacketAttack(*this));
		registerUnitTest(new P2PTestNewBlockHashesAttack(*this));
		registerUnitTest(new P2PTestGetBlockBodiesPacket(*this));
		registerUnitTest(new P2PTestBlockBodiesPacket(*this));
		registerUnitTest(new P2PTestGetNodeDataPacket(*this));
		registerUnitTest(new P2PTestNodeDataPacket(*this));
		registerUnitTest(new P2PTestGetReceiptsPacket(*this));
		registerUnitTest(new P2PTestReceiptsPacket(*this));
	}

	void P2PHostProxy::registerUnitTest(const string& unitTestName, const string& subTestName)
	{
		m_subTestName = subTestName;
		if (unitTestName == "AttackUnitTest")
		{
			registerAttackUnitTest();
		}
		else if (unitTestName == "P2PTestInvalidNetworkIDStatus")
		{
			registerUnitTest(new P2PTestInvalidNetworkIDStatus(*this));
		}
		else if (unitTestName == "P2PTestInvalidGenesisHashStatus")
		{
			registerUnitTest(new P2PTestInvalidGenesisHashStatus(*this));
		}
		else if (unitTestName == "P2PTestInvalidProtocolVersionStatus")
		{
			registerUnitTest(new P2PTestInvalidProtocolVersionStatus(*this));
		}
		else if (unitTestName == "P2PTestIrrDifferentHashStatus")
		{
			registerUnitTest(new P2PTestIrrDifferentHashStatus(*this));
		}
		else if (unitTestName == "P2PTestIrrHashDifferent")
		{
			registerUnitTest(new P2PTestIrrHashDifferent(*this));
		}
		else if (unitTestName == "P2PTestChainASyncB")
		{
			registerUnitTest(new P2PTestChainASyncB(*this));
		}
		else if (unitTestName == "P2PTestNoProduceStart")
		{
			registerUnitTest(new P2PTestNoProduceStart(*this));
		}
		else if (unitTestName == "P2PTestIdlChainASyncBNew")
		{
			registerUnitTest(new P2PTestIdlChainASyncBNew(*this));
		}
		else if (unitTestName == "P2PTestFindingCommonChainASyncBNew")
		{
			registerUnitTest(new P2PTestFindingCommonChainASyncBNew(*this));
		}
		else if (unitTestName == "P2PTestFindingCommonBlockHeader")
		{
			registerUnitTest(new P2PTestFindingCommonBlockHeader(*this));
		}
		else if (unitTestName == "P2PTestIdlNewPeerConnected")
		{
			registerUnitTest(new P2PTestIdlNewPeerConnected(*this));
		}
		else if (unitTestName == "P2PTestFindingCommonNewPeerStatus")
		{
			registerUnitTest(new P2PTestFindingCommonNewPeerStatus(*this));
		}
		else if (unitTestName == "P2PTestSyncBlocks")
		{
			registerUnitTest(new P2PTestSyncBlocks(*this));
		}
		else
		{
			ctrace << "Error unit test name: " << unitTestName;
			return;
		}

		switchUnitTest(0);
	}

	void P2PHostProxy::registerAllUnitTest()
	{
		/*
		* 测试用例页面
		* http://192.168.1.110:8090/pages/viewpage.action?pageId=11993092
		*/

		/*
		* 攻击测试
		*/
		///eth需要可以正常出块，config.json文件中需要有AccountName
		///eth运行参数 --p2pfiller-path  P2PTestPackageAttack.json --chainname "A"
		//registerUnitTest(new P2PTestRequestHeaderAttack(*this));
		//registerUnitTest(new P2PTestSendHeaderAttack(*this));		
		//registerUnitTest(new P2PTestNewBlockAttack(*this));
		registerUnitTest(new P2PTestStatusPacketAttack(*this));
		//registerUnitTest(new P2PTestNewBlockHashesAttack(*this));
		//registerUnitTest(new P2PTestGetBlockBodiesPacket(*this));
		//registerUnitTest(new P2PTestBlockBodiesPacket(*this));
		//registerUnitTest(new P2PTestGetNodeDataPacket(*this));
		//registerUnitTest(new P2PTestNodeDataPacket(*this));
		//registerUnitTest(new P2PTestGetReceiptsPacket(*this));
		//registerUnitTest(new P2PTestReceiptsPacket(*this));

		/*
		*建立连接测试
		*/
		///statue不匹配，包括版本、创世哈希、networkid
		registerUnitTest(new P2PTestInvalidNetworkIDStatus(*this));
		registerUnitTest(new P2PTestInvalidGenesisHashStatus(*this));
		registerUnitTest(new P2PTestInvalidProtocolVersionStatus(*this));


		///eth运行参数 --p2pfiller-path IrrHashDifferent.json --chainname "A"
		//registerUnitTest(new P2PTestIrrDifferentHashPacket(*this));

		/*
		*收块测试
		*/
		///eth运行参数 --p2pfiller-path IrrHashDifferent.json --chainname "A"
		//registerUnitTest(new P2PTestIrrHashDifferent(*this));

		///eth运行参数 --p2pfiller-path ChainASyncB.json --chainname "A"
		//registerUnitTest(new P2PTestChainASyncB(*this));

		/*
		* 初始启动测试
		*/
		///运行时参数使用--p2pfiller-path P2PTestNoProduceStart.json --chainname "A"
		///config.json中不能有参数"enableStaleProduction"，
		///运行前需要删除eth数据文件夹
		//registerUnitTest(new P2PTestNoProduceStart(*this));

		switchUnitTest(0);
	}

	pair<bytes, unsigned> P2PHostProxy::blockHeaders(RLP const& _blockId, unsigned _maxHeaders, u256 _skip, bool _reverse, int32_t _invalidNum) 
	{
		auto numHeadersToSend = _maxHeaders;

		auto step = static_cast<unsigned>(_skip) + 1;
		assert(step > 0 && "step must not be 0");

		h256 blockHash;
		auto& _chain = getClient().getBlockChain();
		if (_blockId.size() == 32) // block id is a hash
		{
			blockHash = _blockId.toHash<h256>();
			clog(NetMessageSummary) << "GetBlockHeaders (block (hash): " << blockHash
				<< ", maxHeaders: " << _maxHeaders
				<< ", skip: " << _skip << ", reverse: " << _reverse << ")";

			if (!_chain.isKnown(blockHash))
				blockHash = {};
			else if (!_reverse)
			{
				auto n = _chain.number(blockHash);
				if (numHeadersToSend == 0)
					blockHash = {};
				else if (n != 0 || blockHash == _chain.genesisHash())
				{
					auto top = n + uint64_t(step) * numHeadersToSend - 1;
					auto lastBlock = _chain.number();
					if (top > lastBlock)
					{
						numHeadersToSend = (lastBlock - n) / step + 1;
						top = n + step * (numHeadersToSend - 1);
					}
					assert(top <= lastBlock && "invalid top block calculated");
					blockHash = _chain.numberHash(static_cast<unsigned>(top)); // override start block hash with the hash of the top block we have
				}
				else
					blockHash = {};
			}
		}
		else // block id is a number
		{
			auto n = _blockId.toInt<bigint>();
			clog(NetMessageSummary) << "GetBlockHeaders (" << n
				<< "max: " << _maxHeaders
				<< "skip: " << _skip << (_reverse ? "reverse" : "") << ")";

			ctrace << "n = " << n << " lastBlock = " << _chain.number() << " numHeadersToSend = " << numHeadersToSend;
			if (!_reverse)
			{
				auto lastBlock = _chain.number();
				if (n > lastBlock || numHeadersToSend == 0)
					blockHash = {};
				else
				{
					bigint top = n + uint64_t(step) * (numHeadersToSend - 1);
					if (top > lastBlock)
					{
						numHeadersToSend = (lastBlock - static_cast<unsigned>(n)) / step + 1;
						top = n + step * (numHeadersToSend - 1);
					}
					ctrace << "top =" << top << " lastBlock =" << lastBlock;
					assert(top <= lastBlock && "invalid top block calculated");
					blockHash = _chain.numberHash(static_cast<unsigned>(top)); // override start block hash with the hash of the top block we have
					ctrace << "blockHash = _chain.numberHash blockHash = " << blockHash;
				}
			}
			else if (n <= std::numeric_limits<unsigned>::max())
				blockHash = _chain.numberHash(static_cast<unsigned>(n));
			else {
				blockHash = {};
			}
		}

		ctrace << "blockHash = " << blockHash;

		auto nextHash = [&_chain, this](h256 _h, unsigned _step)
		{
			static const unsigned c_blockNumberUsageLimit = 1000;

			const auto lastBlock = _chain.number();
			const auto limitBlock = lastBlock > c_blockNumberUsageLimit ? lastBlock - c_blockNumberUsageLimit : 0; // find the number of the block below which we don't expect BC changes.

			while (_step) // parent hash traversal
			{
				auto details = _chain.details(_h);
				if (details.number < limitBlock)
					break; // stop using parent hash traversal, fallback to using block numbers
				_h = details.parent;
				--_step;
			}

			if (_step) // still need lower block
			{
				auto n = _chain.number(_h);
				if (n >= _step)
					_h = _chain.numberHash(n - _step);
				else
					_h = {};
			}


			return _h;
		};

		bytes rlp;
		unsigned itemCount = 0;
		vector<h256> hashes;
		for (unsigned i = 0; i != numHeadersToSend; ++i)
		{
			if (!blockHash || !_chain.isKnown(blockHash))
				break;

			hashes.push_back(blockHash);
			++itemCount;

			blockHash = nextHash(blockHash, step);
		}

		for (unsigned i = 0; i < hashes.size() && rlp.size() < c_maxPayload; ++i)
		{
			if (i == _invalidNum)
			{
				auto bh = _chain.info(hashes[_reverse ? i : hashes.size() - 1 - i]);
				bytes extradata;
				extradata.push_back('0');
				bh.setExtraData(extradata);
				RLPStream ret;
				bh.streamRLP(ret);
				rlp += ret.out();
			}
			else
			{
				rlp += _chain.headerData(hashes[_reverse ? i : hashes.size() - 1 - i]);
			}
		}

		ctrace << "itemCount = " << itemCount;

		return make_pair(rlp, itemCount);
	}

	pair<bytes, unsigned> P2PHostProxy::blockBodies(RLP const& _blockHashes)
	{
		unsigned const count = static_cast<unsigned>(_blockHashes.itemCount());

		bytes rlp;
		unsigned n = 0;
		auto numBodiesToSend = std::min(count, c_maxBlocks);
		for (unsigned i = 0; i < numBodiesToSend && rlp.size() < c_maxPayload; ++i)
		{
			auto h = _blockHashes[i].toHash<h256>();
			auto& _chain = getClient().getBlockChain();
			if (_chain.isKnown(h))
			{
				bytes blockBytes = _chain.block(h);
				RLP block{ blockBytes };
				RLPStream body;
				body.appendList(2);
				body.appendRaw(block[1].data()); // transactions
				body.appendRaw(block[2].data()); // uncles
				auto bodyBytes = body.out();
				rlp.insert(rlp.end(), bodyBytes.begin(), bodyBytes.end());
				++n;
			}
		}
		if (count > 20 && n == 0)
			clog(NetWarn) << "all" << count << "unknown blocks requested; peer on different chain?";
		else
			clog(NetMessageSummary) << n << "blocks known and returned;" << (numBodiesToSend - n) << "blocks unknown;" << (count > c_maxBlocks ? count - c_maxBlocks : 0) << "blocks ignored";

		return make_pair(rlp, n);
	}

	void P2PHostProxy::requestStatus(u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash, u256 _lastIrrBlock, h256 _lastIrrBlockHash, unsigned _hostProtocolVersion)
	{
		RLPStream s;
		prep(s, StatusPacket, 7)
			<< _hostProtocolVersion
			<< _hostNetworkId
			<< _chainTotalDifficulty
			<< _chainCurrentHash
			<< _chainGenesisHash
			<< _lastIrrBlock
			<< _lastIrrBlockHash
			;

		sealAndSend(s);
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

	void P2PHostProxy::sendNewBlockHash(const h256& block, unsigned number)
	{
		RLPStream ts;
		prep(ts, NewBlockHashesPacket, 1);

		ts.appendList(2);
		ts.append(block);
		ts.append(number);

		sealAndSend(ts);
	}

	void P2PHostProxy::sendNewBlock(const bytes& block, const u256& irreversibleBlockNum, const h256& irreversibleBlockHash)
	{
		RLPStream ts;
		if (irreversibleBlockNum == 0 || irreversibleBlockHash == h256())
		{
			prep(ts, NewBlockPacket, 3).appendRaw(block, 1).append(u256(m_client.getBlockChain().getIrreversibleBlock())).append(m_client.getBlockChain().getIrreversibleBlockHash());
		}
		else
		{
			prep(ts, NewBlockPacket, 3).appendRaw(block, 1).append(irreversibleBlockNum).append(irreversibleBlockHash);
		}
		sealAndSend(ts);
	}

	void P2PHostProxy::sendBlockHeader(const bytes& block, unsigned _count)
	{
		RLPStream s;
		prep(s, BlockHeadersPacket, _count).appendRaw(block, _count);
		sealAndSend(s);
	}


	void P2PHostProxy::getBlockBodiesPacket(const h256& block, unsigned number)
	{
		RLPStream ts;
		prep(ts, GetBlockBodiesPacket,1);
		ts.append(block);

		sealAndSend(ts);
	}

	void P2PHostProxy::sendBlockBodiesPacket(const bytes& block, unsigned number)
	{
		RLPStream ts;
		prep(ts, BlockBodiesPacket, number).appendRaw(block, number);
		sealAndSend(ts);
	}
	
	void P2PHostProxy::getNodeDataPacket(const h256& block, unsigned number)
	{
		RLPStream ts;
		prep(ts, GetNodeDataPacket, 1);
		ts.append(block);

		sealAndSend(ts);
	}

	void P2PHostProxy::sendNodeDataPacket(const h256& block, unsigned number)
	{
		RLPStream ts;
		prep(ts, NodeDataPacket, 1);
		ts.appendList(2);
		ts.append(block);
		ts.append(number);

		sealAndSend(ts);
	}

	void P2PHostProxy::getReceiptsPacket(const h256& block, unsigned number)
	{
		RLPStream ts;
		prep(ts, GetReceiptsPacket, 1);
		ts.append(block);

		sealAndSend(ts);
	}
	
	void P2PHostProxy::sendReceiptsPacket(const h256& block, unsigned number)
	{
		RLPStream ts;
		prep(ts, ReceiptsPacket, 1);
		ts.appendList(2);
		ts.append(block);
		ts.append(number);

		sealAndSend(ts);
	}

	dev::RLPStream& P2PHostProxy::prep(RLPStream& _s, unsigned _id, unsigned _args)
	{
		return _s.appendRaw(bytes(1, _id + UserPacket)).appendList(_args);
	}

	void P2PHostProxy::sealAndSend(dev::RLPStream& _s)
	{
		bytes b;
		_s.swapOut(b);
		sendToHost(b);
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

	void P2PHostProxy::run(boost::system::error_code const&)
	{
		auto pUnitTest = getCurrUnitTest();
		if (nullptr != pUnitTest)
		{
			pUnitTest->step();
		}

		auto runcb = [this](boost::system::error_code const& error) { run(error); };
		m_timer->expires_from_now(boost::posix_time::milliseconds(500));
		m_timer->async_wait(runcb);
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

	/*
	* UnitTest: P2PTestRequestHeaderAttack
	*/

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

	/*
	* UnitTest: P2PTestSendHeaderAttack
	*/

	//用例名称
	std::string P2PTestSendHeaderAttack::name() const
	{
		return "P2PTestSendHeaderAttack";
	}

	//用例销毁
	void P2PTestSendHeaderAttack::destroy()
	{
		ctrace << "P2PTestSendHeaderAttack::destroy";
	}

	//用来解析传来的协议包
	void P2PTestSendHeaderAttack::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestSendHeaderAttack::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestSendHeaderAttack::interpret";
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
	void P2PTestSendHeaderAttack::step()
	{
		auto tid = std::this_thread::get_id();

		static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

		uint64_t number = s_eng();

		P2PTestClient& _client = m_hostProxy.getClient();

        bytes header = _client.getBlockChain().headerData();
		m_hostProxy.sendBlockHeader(header);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "P2PTestSendHeaderAttack::step()";
	}


	/*
	* UnitTest: P2PTestNewBlockAttack
	*/

	//用例名称
	std::string P2PTestNewBlockAttack::name() const
	{
		return "P2PTestNewBlockAttack";
	}



	//用例销毁
	void P2PTestNewBlockAttack::destroy()
	{
		ctrace << "P2PTestNewBlockAttack::destroy";
	}

	//用来解析传来的协议包
	void P2PTestNewBlockAttack::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestNewBlockAttack::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestNewBlockAttack::interpret";
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
			case GetBlockHeadersPacket:
			{
				/// Packet layout:
				/// [ block: { P , B_32 }, maxHeaders: P, skip: P, reverse: P in { 0 , 1 } ]
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				//auto numHeadersToSend = maxHeaders <= c_maxHeadersToSend ? static_cast<unsigned>(maxHeaders) : c_maxHeadersToSend;

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();

					auto& _client = m_hostProxy.getClient();
					auto& _chain = _client.getBlockChain();
					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}
				break;
			}
			case NewBlockPacket:
			{
				if (_r.itemCount() != 3)
				{
					return;
				}

				BlockHeader info(_r[0][0].data(), HeaderData);
				m_latestBlockNum = info.number().convert_to<uint32_t>();
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
	void P2PTestNewBlockAttack::step()
	{
		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
        bytes curHash = bc.block(bc.currentHash());
		m_hostProxy.sendNewBlock(curHash);
		ctrace << "P2PTestNewBlockAttack::step()";
	}

	/*
	* UnitTest: P2PTestStatusPacketAttack
	*/

	//用例名称
	std::string P2PTestStatusPacketAttack::name() const
	{
		return "P2PTestStatusPacketAttack";
	}

	//用例销毁
	void P2PTestStatusPacketAttack::destroy()
	{
		ctrace << "P2PTestStatusPacketAttack::destroy";
	}

	//用来解析传来的协议包
	void P2PTestStatusPacketAttack::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestStatusPacketAttack::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestStatusPacketAttack::interpret";
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
			case NewBlockPacket:
			{
				if (_r.itemCount() != 3)
				{
					return;
				}

				BlockHeader info(_r[0][0].data(), HeaderData);
				m_lattBlockNum = info.number().convert_to<uint32_t>();
				m_lastIrrBlock = _r[1].toInt<u256>().convert_to<uint32_t>();
				m_lastIrrBlockHash = _r[2].toHash<h256>();
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
	void P2PTestStatusPacketAttack::step()
	{
		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());

		ctrace << "P2PTestStatusPacketAttack::step()";
	}

	/*
	* UnitTest: P2PTestInvalidNetworkIDStatus
	*/

	//用例名称
	std::string P2PTestInvalidNetworkIDStatus::name() const
	{
		return "P2PTestInvalidNetworkIDStatus";
	}

	//用于用例初始化
	void P2PTestInvalidNetworkIDStatus::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);
		m_passTest = true;
		ctrace << "P2PTestInvalidNetworkIDStatus::init";
	}

	//用例销毁
	void P2PTestInvalidNetworkIDStatus::destroy()
	{
		ctrace << "P2PTestInvalidNetworkIDStatus::destroy";
	}

	//用来解析传来的协议包
	void P2PTestInvalidNetworkIDStatus::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestInvalidNetworkIDStatus::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestInvalidNetworkIDStatus::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				m_passTest = false;

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
	void P2PTestInvalidNetworkIDStatus::step()
	{
		static bool send = false;
		if (send)
		{
			if (!m_passTest)
			{
				ctrace << "P2PTestInvalidNetworkIDStatus Test failed!!";
			}
			else
			{
				ctrace << "P2PTestInvalidNetworkIDStatus Test success!!";
			}
			return;
		}

		// invalid network id
		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(u256(), bc.details().totalDifficulty,
			bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		send = true;

		ctrace << "P2PTestInvalidNetworkIDStatus::step()";
	}

	/*
	* UnitTest: P2PTestInvalidGenesisHashStatus
	*/

	//用例名称
	std::string P2PTestInvalidGenesisHashStatus::name() const
	{
		return "P2PTestInvalidGenesisHashStatus";
	}

	//用于用例初始化
	void P2PTestInvalidGenesisHashStatus::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);
		m_passTest = true;
		ctrace << "P2PTestInvalidGenesisHashStatus::init";
	}

	//用例销毁
	void P2PTestInvalidGenesisHashStatus::destroy()
	{
		ctrace << "P2PTestInvalidGenesisHashStatus::destroy";
	}

	//用来解析传来的协议包
	void P2PTestInvalidGenesisHashStatus::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestInvalidGenesisHashStatus::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestInvalidGenesisHashStatus::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				m_passTest = false;

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
	void P2PTestInvalidGenesisHashStatus::step()
	{
		static bool send = false;
		if (send)
		{
			if (!m_passTest)
			{
				ctrace << "P2PTestInvalidNetworkIDStatus Test failed!!";
			}
			else
			{
				ctrace << "P2PTestInvalidNetworkIDStatus Test success!!";
			}
			return;
		}

		// invalid GenesisHash
		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.currentHash(), h256(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		send = true;

		ctrace << "P2PTestInvalidGenesisHashStatus::step()";
	}

	/*
	* UnitTest: P2PTestInvalidProtocolVersionStatus
	*/

	//用例名称
	std::string P2PTestInvalidProtocolVersionStatus::name() const
	{
		return "P2PTestInvalidProtocolVersionStatus";
	}

	//用于用例初始化
	void P2PTestInvalidProtocolVersionStatus::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);
		m_passTest = true;
		ctrace << "P2PTestInvalidProtocolVersionStatus::init";
	}

	//用例销毁
	void P2PTestInvalidProtocolVersionStatus::destroy()
	{
		ctrace << "P2PTestInvalidProtocolVersionStatus::destroy";
	}

	//用来解析传来的协议包
	void P2PTestInvalidProtocolVersionStatus::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestInvalidProtocolVersionStatus::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestInvalidProtocolVersionStatus::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				m_passTest = false;

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
	void P2PTestInvalidProtocolVersionStatus::step()
	{
		static bool send = false;
		if (send)
		{
			if (!m_passTest)
			{
				ctrace << "P2PTestInvalidNetworkIDStatus Test failed!!";
			}
			else
			{
				ctrace << "P2PTestInvalidNetworkIDStatus Test success!!";
			}
			return;
		}

		// invalid ProtocolVersion
		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash(), 62);

		send = true;

		ctrace << "P2PTestInvalidProtocolVersionStatus::step()";
	}

	/*
	* UnitTest: P2PTestIrrDifferentHashStatus
	*/

	//用例名称
	std::string P2PTestIrrDifferentHashStatus::name() const
	{
		return "P2PTestIrrDifferentHashPacket";
	}

	//用于用例初始化
	void P2PTestIrrDifferentHashStatus::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);
		m_passTest = true;
		ctrace << "P2PTestIrrDifferentHashPacket::init";
	}

	//用例销毁
	void P2PTestIrrDifferentHashStatus::destroy()
	{
		ctrace << "P2PTestIrrDifferentHashPacket::destroy";
	}

	//用来解析传来的协议包
	void P2PTestIrrDifferentHashStatus::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestIrrDifferentHashStatus::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestIrrDifferentHashPacket::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				if (blockId.toHash<h256>() == m_hostProxy.getClient().m_chainCurrentHash)
					m_passTest = false;

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
	void P2PTestIrrDifferentHashStatus::step()
	{
		static int steps = 0;
		static int failedStep = -1;
		if (!m_passTest)
		{
			failedStep = steps;
			steps = -1;
		}

		//如果连接是发现对方的不可逆块高度小于自己，且相同高度上的块hash不同
		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		steps++;

		if (steps != -1)
			ctrace << "P2PTestIrrHashDifferent  success!!";
		else
			ctrace << "P2PTestIrrHashDifferent  failed !!!";
	

		ctrace << "P2PTestIrrDifferentHashPacket::step()";
	}

	/*
	* UnitTest: P2PTestBlockChainIrrket
	*/

	//用例名称
	std::string P2PTestChainASyncB::name() const
	{
		return "P2PTestChainASyncB";
	}

	//用于用例初始化
	void P2PTestChainASyncB::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);

		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		m_passTest = false;

		ctrace << "P2PTestChainASyncB::init";
	}

	//用例销毁
	void P2PTestChainASyncB::destroy()
	{
		ctrace << "P2PTestChainASyncB::destroy";
	}

	//用来解析传来的协议包
	void P2PTestChainASyncB::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		switch (_t)
		{
		case DisconnectPacket:
		{
			m_passTest = false;
			string reason = "Unspecified";
			auto r = (DisconnectReason)_r[0].toInt<int>();
			if (!_r[0].isInt())
				ctrace << "Disconnect (reason: no reason)";
			else
			{
				reason = reasonOf(r);
				ctrace << "Disconnect (reason: " << reason << ")";
			}
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

	void P2PTestChainASyncB::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestChainASyncB::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();

					auto& _client = m_hostProxy.getClient();
					auto& _chain = _client.getBlockChain();
					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}

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
	void P2PTestChainASyncB::step()
	{
		static int steps = 0;
		static int failedStep = -1;
		P2PTestClient& _client = m_hostProxy.getClient();
		
		if (m_passTest)
		{
			failedStep = steps;
			steps = -1;
		}
		switch (steps)
		{
		case 0:
        {
			//如果块高度小于等于本地节点的不可逆高度，且相同高度上的块hash相同
			ctrace << "case 0";
			bytes block = _client.getBlockChain().block(_client.getBlockChain().numberHash(10));
			m_hostProxy.sendNewBlock(block);
			m_needSync = false;
			steps++;
			break;
        }
		case 1:
		{
			// 块高度小于当前节点的最新导入块高度，但此外部节点不可逆却大于当前节点不可逆
			ctrace << "case 1";
			bytes block = _client.getBlockChain().block(_client.getBlockChain().numberHash(20));
			m_hostProxy.sendNewBlock(block);
			steps++;
			m_needSync = true;
			break;
		}
		case 2:
            {
			// 块高度大于本地节点不可逆转块高度
			ctrace << "case 2";
            bytes block = _client.getBlockChain().block(_client.getBlockChain().numberHash(25));
			m_hostProxy.sendNewBlock(block);
			steps++;
			m_needSync = true;
			break;
            }
		case -1:
			ctrace << "Test finished failed! step: " << failedStep;
			break;
		default:
			ctrace << "Test finished success!!";
			break;
		}
		
		ctrace << "P2PTestChainASyncB::step()";
	}

	/*
	* UnitTest: P2PTestBlockChainIrrket
	*/

	//用例名称
	std::string P2PTestIrrHashDifferent::name() const
	{
		return "P2PTestIrrHashDifferent";
	}

	//用例销毁
	void P2PTestIrrHashDifferent::destroy()
	{
		ctrace << "P2PTestIrrHashDifferent::destroy";
	}

	//用于用例初始化
	void P2PTestIrrHashDifferent::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);

		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		m_passTest = true;

		ctrace << "P2PTestChainASyncB::init";
	}

	//用来解析传来的协议包
	void P2PTestIrrHashDifferent::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestIrrHashDifferent::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestIrrHashDifferent::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				if (m_bagin)
					m_passTest = false;

				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();

					auto& _client = m_hostProxy.getClient();
					auto& _chain = _client.getBlockChain();
					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}

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
	void P2PTest::P2PTestIrrHashDifferent::step()
	{
		static int steps = 0;
		static int failedStep = -1;
		P2PTestClient& _client = m_hostProxy.getClient();

       //收到的块所记录的不可逆块高度小于小于本地节点的不可逆块高度，且相同高度上的块hash不同
		if (!m_passTest)
		{
			failedStep = steps;
			steps = -1;
		}

        bytes block = _client.getBlockChain().block();
		m_hostProxy.sendNewBlock(block);
		m_bagin = true;

		if (steps != -1)
			ctrace << "P2PTestIrrHashDifferent  success!!";
		else
			ctrace << "P2PTestIrrHashDifferent  failed !!!";
			


		ctrace << "P2PTestIrrHashDifferent::step()";
	}

	/*
	* UnitTest: P2PTestNewBlockHashesAttack
	*/

	//用例名称
	std::string P2PTestNewBlockHashesAttack::name() const
	{
		return "P2PTestNewBlockHashesAttack";
	}

	//用例销毁
	void P2PTestNewBlockHashesAttack::destroy()
	{
		ctrace << "P2PTestNewBlockHashesAttack::destroy";
	}

	//用来解析传来的协议包
	void P2PTestNewBlockHashesAttack::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestNewBlockHashesAttack::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestNewBlockHashesAttack::interpret";
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
	void P2PTestNewBlockHashesAttack::step()
	{
		auto tid = std::this_thread::get_id();

		static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

		uint64_t number = s_eng();

        h256 bhash = dev::sha3(to_string(number));
		m_hostProxy.sendNewBlockHash(bhash, number);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "P2PTestNewBlockHashesAttack::step()";
	}

	/*
	* UnitTest: P2PTestGetBlockBodiesPacket
	*/

	//用例名称
	std::string P2PTestGetBlockBodiesPacket::name() const
	{
		return "P2PTestNewBlockHashesAttack";
	}

	//用例销毁
	void P2PTestGetBlockBodiesPacket::destroy()
	{
		ctrace << "GetBlockBodiesPacket::destroy";
	}

	//用来解析传来的协议包
	void P2PTestGetBlockBodiesPacket::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestGetBlockBodiesPacket::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestGetBlockBodiesPacket::interpret";
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
			case BlockBodiesPacket:
			{
			/*	if (m_asking != Asking::BlockBodies)
					clog(NetImpolite) << "Peer giving us block bodies when we didn't ask for them.";
				else
				{
					setIdle();
					observer->onPeerBlockBodies(dynamic_pointer_cast<EthereumPeer>(shared_from_this()), _r);
				}*/
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
	void P2PTestGetBlockBodiesPacket::step()
	{
		auto tid = std::this_thread::get_id();

		static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

		uint64_t number = s_eng();

        h256 bhash = dev::sha3(to_string(number));
		m_hostProxy.getBlockBodiesPacket(bhash, number);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "GetBlockBodiesPacket::step()";
	}


	/*
	* UnitTest: P2PTestBlockBodiesPacket
	*/

	std::string P2PTestBlockBodiesPacket::name() const
	{
		return "P2PTestBlockBodiesPacket";
	}

	//用例销毁
	void P2PTestBlockBodiesPacket::destroy()
	{
		ctrace << "P2PTestBlockBodiesPacket::destroy";
	}

	//用来解析传来的协议包
	void P2PTestBlockBodiesPacket::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestBlockBodiesPacket::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestBlockBodiesPacket::interpret";
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
			case GetBlockBodiesPacket:
			{
				/*unsigned count = static_cast<unsigned>(_r.itemCount());
				clog(NetMessageSummary) << "GetBlockBodies (" << dec << count << "entries)";

				if (!count)
				{
					clog(NetImpolite) << "Zero-entry GetBlockBodies: Not replying.";
					addRating(-10);
					break;
				}

				pair<bytes, unsigned> const rlpAndItemCount = hostData->blockBodies(_r);

				addRating(0);
				RLPStream s;
				prep(s, BlockBodiesPacket, rlpAndItemCount.second).appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
				sealAndSend(s);*/
				step();
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
	void P2PTestBlockBodiesPacket::step()
	{
		auto tid = std::this_thread::get_id();

		static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

		uint64_t number = s_eng();

		m_hostProxy.sendBlockBodiesPacket(bytes(), 0);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "sendBlockBodiesPacket::step()";
	}


	/*
	* UnitTest: P2PTestGetNodeDataPacket
	*/

	std::string P2PTestGetNodeDataPacket::name() const
	{
		return "P2PTestGetNodeDataPacket";
	}

	//用例销毁
	void P2PTestGetNodeDataPacket::destroy()
	{
		ctrace << "P2PTestGetNodeDataPacket::destroy";
	}

	//用来解析传来的协议包
	void P2PTestGetNodeDataPacket::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestGetNodeDataPacket::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestGetNodeDataPacket::interpret";
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
			case GetBlockBodiesPacket:
			{
				/*unsigned count = static_cast<unsigned>(_r.itemCount());
				clog(NetMessageSummary) << "GetBlockBodies (" << dec << count << "entries)";

				if (!count)
				{
				clog(NetImpolite) << "Zero-entry GetBlockBodies: Not replying.";
				addRating(-10);
				break;
				}

				pair<bytes, unsigned> const rlpAndItemCount = hostData->blockBodies(_r);

				addRating(0);
				RLPStream s;
				prep(s, BlockBodiesPacket, rlpAndItemCount.second).appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
				sealAndSend(s);*/
				step();
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
	void P2PTestGetNodeDataPacket::step()
	{
		auto tid = std::this_thread::get_id();

		static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

		uint64_t number = s_eng();

        h256 bhash = dev::sha3(to_string(number));
		m_hostProxy.getNodeDataPacket(bhash, number);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "P2PTestGetNodeDataPacket::step()";
	}

	/*
	* UnitTest: P2PTestNodeDataPacket
	*/

	std::string P2PTestNodeDataPacket::name() const
	{
		return "P2PTestNodeDataPacket";
	}

	//用例销毁
	void P2PTestNodeDataPacket::destroy()
	{
		ctrace << "P2PTestNodeDataPacket::destroy";
	}

	//用来解析传来的协议包
	void P2PTestNodeDataPacket::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestNodeDataPacket::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestNodeDataPacket::interpret";
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
			case GetBlockBodiesPacket:
			{
				/*unsigned count = static_cast<unsigned>(_r.itemCount());
				clog(NetMessageSummary) << "GetBlockBodies (" << dec << count << "entries)";

				if (!count)
				{
				clog(NetImpolite) << "Zero-entry GetBlockBodies: Not replying.";
				addRating(-10);
				break;
				}

				pair<bytes, unsigned> const rlpAndItemCount = hostData->blockBodies(_r);

				addRating(0);
				RLPStream s;
				prep(s, BlockBodiesPacket, rlpAndItemCount.second).appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
				sealAndSend(s);*/
				step();
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
	void P2PTestNodeDataPacket::step()
	{
		auto tid = std::this_thread::get_id();

		static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

		uint64_t number = s_eng();

        h256 bhash = dev::sha3(to_string(number));
		m_hostProxy.sendNodeDataPacket(bhash, number);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "P2PTestNodeDataPacket::step()";
	}


	/*
	* UnitTest: P2PTestGetReceiptsPacket
	*/

	std::string P2PTestGetReceiptsPacket::name() const
	{
		return "P2PTestGetReceiptsPacket";
	}

	//用例销毁
	void P2PTestGetReceiptsPacket::destroy()
	{
		ctrace << "P2PTestGetReceiptsPacket::destroy";
	}

	//用来解析传来的协议包
	void P2PTestGetReceiptsPacket::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestGetReceiptsPacket::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestGetReceiptsPacket::interpret";
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
			case GetBlockBodiesPacket:
			{
				/*unsigned count = static_cast<unsigned>(_r.itemCount());
				clog(NetMessageSummary) << "GetBlockBodies (" << dec << count << "entries)";

				if (!count)
				{
				clog(NetImpolite) << "Zero-entry GetBlockBodies: Not replying.";
				addRating(-10);
				break;
				}

				pair<bytes, unsigned> const rlpAndItemCount = hostData->blockBodies(_r);

				addRating(0);
				RLPStream s;
				prep(s, BlockBodiesPacket, rlpAndItemCount.second).appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
				sealAndSend(s);*/
				step();
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
	void P2PTestGetReceiptsPacket::step()
	{
		auto tid = std::this_thread::get_id();

		static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

		uint64_t number = s_eng();

        h256 bhash = dev::sha3(to_string(number));
		m_hostProxy.getReceiptsPacket(bhash, number);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "P2PTestGetReceiptsPacket::step()";
	}


	/*
	* UnitTest: P2PTestReceiptsPacket
	*/

	std::string P2PTestReceiptsPacket::name() const
	{
		return "P2PTestReceiptsPacket";
	}

	//用例销毁
	void P2PTestReceiptsPacket::destroy()
	{
		ctrace << "P2PTestReceiptsPacket::destroy";
	}

	//用来解析传来的协议包
	void P2PTestReceiptsPacket::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestReceiptsPacket::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestReceiptsPacket::interpret";
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
			case GetBlockBodiesPacket:
			{
				/*unsigned count = static_cast<unsigned>(_r.itemCount());
				clog(NetMessageSummary) << "GetBlockBodies (" << dec << count << "entries)";

				if (!count)
				{
				clog(NetImpolite) << "Zero-entry GetBlockBodies: Not replying.";
				addRating(-10);
				break;
				}

				pair<bytes, unsigned> const rlpAndItemCount = hostData->blockBodies(_r);

				addRating(0);
				RLPStream s;
				prep(s, BlockBodiesPacket, rlpAndItemCount.second).appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
				sealAndSend(s);*/
				step();
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
	void P2PTestReceiptsPacket::step()
	{
		auto tid = std::this_thread::get_id();

		static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

		uint64_t number = s_eng();

        h256 bhash = dev::sha3(to_string(number));
		m_hostProxy.sendReceiptsPacket(bhash, number);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		ctrace << "P2PTestReceiptsPacket::step()";
	}

	/*
	* UnitTest: P2PTestNoProduceStart
	*/

	std::string P2PTestNoProduceStart::name() const
	{
		return "P2PTestReceiptsPacket";
	}

	//用例销毁
	void P2PTestNoProduceStart::destroy()
	{
		ctrace << "P2PTestNoProduceStart::destroy";
	}

	//用来解析传来的协议包
	void P2PTestNoProduceStart::interpretProtocolPacket(PacketType _t, RLP const& _r)
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

	void P2PTestNoProduceStart::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestNoProduceStart::interpret";
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
			case GetBlockHeadersPacket:
			{
				/// Packet layout:
				/// [ block: { P , B_32 }, maxHeaders: P, skip: P, reverse: P in { 0 , 1 } ]
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				//auto numHeadersToSend = maxHeaders <= c_maxHeadersToSend ? static_cast<unsigned>(maxHeaders) : c_maxHeadersToSend;

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();

					auto& _client = m_hostProxy.getClient();
					auto& _chain = _client.getBlockChain();
					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}
				break;
			}
			case NewBlockPacket:
			{
				m_testPass = true;
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
	void P2PTestNoProduceStart::step()
	{
		static auto startTime = utcTime();
		static uint64_t lastProduceTime = 0;
		auto curTime = utcTime();

		P2PTestClient& _client = m_hostProxy.getClient();
		if (m_testPass)
		{
			ctrace << "P2PTestNoProduceStart passed!";
			return;
		}

		if (curTime - lastProduceTime >= 3)
		{
			bytes block;
			block = _client.produceBlock(curTime);
			if (!block.empty())
				m_hostProxy.sendNewBlock(block);

			lastProduceTime = curTime;
		}

		ctrace << "P2PTestNoProduceStart::step()";
	}

	/*
	* UnitTest: P2PTestIdlNewPeerConnected
	*/

	//用例名称
	std::string P2PTestIdlNewPeerConnected::name() const
	{
		return "P2PTestIdlNewPeerConnected";
	}

	//用例销毁
	void P2PTestIdlNewPeerConnected::destroy()
	{
		ctrace << "P2PTestIdlChainASyncBNew::destroy";
	}

	//用来解析传来的协议包
	void P2PTestIdlNewPeerConnected::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		switch (_t)
		{
		case DisconnectPacket:
		{
			m_passTest = false;
			string reason = "Unspecified";
			auto r = (DisconnectReason)_r[0].toInt<int>();
			if (!_r[0].isInt())
				ctrace << "Disconnect (reason: no reason)";
			else
			{
				reason = reasonOf(r);
				ctrace << "Disconnect (reason: " << reason << ")";
			}
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

	void P2PTestIdlNewPeerConnected::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestIdlNewPeerConnected::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();

					auto& _client = m_hostProxy.getClient();
					auto& _chain = _client.getBlockChain();
					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}

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
	void P2PTestIdlNewPeerConnected::step()
	{
		
		///0:Peer合法检查通过，Peer最新块Hash已知
		///1:Peer合法检查通过，Peer最新块Hash未知
		const string &subUnitTest = m_hostProxy.subTestName();

		if (subUnitTest == "BlockHashIsKnow")
		{
			const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
			ctrace << "P2PTestIdlNewPeerConnected::BlockHashIsKnow";
		}
		else if (subUnitTest == "BlockHashIsnotKnow")
		{
			ctrace << "P2PTestIdlNewPeerConnected::BlockHashIsnotKnow";
		}
		else
		{
			ctrace << "Invalid subUnitTest name!";
		}

		
	}



	/*
	* UnitTest: P2PTestIdlChainASyncBNew
	*/

	//用例名称
	std::string P2PTestIdlChainASyncBNew::name() const
	{
		return "P2PTestIdlChainASyncBNew";
	}

	//用于用例初始化
	void P2PTestIdlChainASyncBNew::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);

		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.numberHash(19), bc.genesisHash(), 15, bc.numberHash(15));
		m_passTest = false;

		ctrace << "P2PTestIdlChainASyncBNew::init";
	}

	//用例销毁
	void P2PTestIdlChainASyncBNew::destroy()
	{
		ctrace << "P2PTestIdlChainASyncBNew::destroy";
	}

	//用来解析传来的协议包
	void P2PTestIdlChainASyncBNew::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		switch (_t)
		{
		case DisconnectPacket:
		{
			m_passTest = false;
			string reason = "Unspecified";
			auto r = (DisconnectReason)_r[0].toInt<int>();
			if (!_r[0].isInt())
				ctrace << "Disconnect (reason: no reason)";
			else
			{
				reason = reasonOf(r);
				ctrace << "Disconnect (reason: " << reason << ")";
			}
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

	void P2PTestIdlChainASyncBNew::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestIdlChainASyncBNew::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();

					auto& _client = m_hostProxy.getClient();
					auto& _chain = _client.getBlockChain();
					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}

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
	void P2PTestIdlChainASyncBNew::step()
	{
		const string &subUnitTest = m_hostProxy.subTestName();
		auto& _client = m_hostProxy.getClient();

		if (subUnitTest == "BlockNumberUnderLast")
		{
			// 块高度小于当前节点的最新导入块高度，但此外部节点不可逆却大于当前节点不可逆
			ctrace << "BlockNumberUnderLast";
			bytes block = _client.getBlockChain().block(_client.getBlockChain().numberHash(20));
			m_hostProxy.sendNewBlock(block);
		}
		else if(subUnitTest == "BlockNumberUpperIrr")
		{
			// 块高度大于本地节点不可逆转块高度
			ctrace << "BlockNumberUpperIrr";
			bytes block = _client.getBlockChain().block(_client.getBlockChain().numberHash(25));
			m_hostProxy.sendNewBlock(block);
		}
		else if (subUnitTest == "NewBlockAddTow")
		{
			// 传入新块恰等于当前节点最后导入块的后继  +2
			ctrace << "NewBlockAddTow";
			bytes block = _client.getBlockChain().block(_client.getBlockChain().numberHash(27));
			m_hostProxy.sendNewBlock(block);

		}
		else if (subUnitTest == "NewBlockAddOne")
		{
			// 传入新块高度等于当前节点最新导入块高度+1  
			ctrace << "NewBlockAddOne";
			static auto startTime = utcTime();
			static uint64_t lastProduceTime = 0;
			auto curTime = utcTime();

			if (curTime - lastProduceTime >= 3)
			{
				bytes block;
				block = _client.produceBlock(curTime);
				if (!block.empty())
					m_hostProxy.sendNewBlock(block);

				lastProduceTime = curTime;
			}

		}
		else
		{
			ctrace << "Invalid subUnitTest name!";
		}

		ctrace << "P2PTestIdlChainASyncBNew::step()";
	}




	/*
	* UnitTest: P2PTestFindingCommonNewPeerStatus
	*/

	//用例名称
	std::string P2PTestFindingCommonNewPeerStatus::name() const
	{
		return "P2PTestIdlNewPeerStatus";
	}

	//用例销毁
	void P2PTestFindingCommonNewPeerStatus::destroy()
	{
		ctrace << "P2PTestIdlNewPeerStatus::destroy";
	}

	//用来解析传来的协议包
	void P2PTestFindingCommonNewPeerStatus::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		switch (_t)
		{
		case DisconnectPacket:
		{
			m_passTest = false;
			string reason = "Unspecified";
			auto r = (DisconnectReason)_r[0].toInt<int>();
			if (!_r[0].isInt())
				ctrace << "Disconnect (reason: no reason)";
			else
			{
				reason = reasonOf(r);
				ctrace << "Disconnect (reason: " << reason << ")";
			}
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

	void P2PTestFindingCommonNewPeerStatus::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestIdlNewPeerConnected::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();

					auto& _client = m_hostProxy.getClient();
					auto& _chain = _client.getBlockChain();
					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}

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
	void P2PTestFindingCommonNewPeerStatus::step()
	{

		///0:创世hash不同
		///1:协议版本不同
		///2:networkId不同
		const string &subUnitTest = m_hostProxy.subTestName();
		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();

		if (subUnitTest == "InvalidGenesisHashStatus")
		{
			ctrace << "InvalidGenesisHashStatus!";
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), h256(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		}
		else if(subUnitTest == "InvalidProtocolVersionStatus")
		{
			ctrace << "InvalidProtocolVersionStatus!";
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash(), 62);
		}
		else if (subUnitTest == "InvalidNetworkIDStatus")
		{
			ctrace << "InvalidNetworkIDStatus!";
			m_hostProxy.requestStatus(u256(), bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		}
		else
		{
			ctrace << "Invalid subUnitTest name!";
		}


		ctrace << "P2PTestIdlNewPeerStatus::step()";
	}

	/*
	* UnitTest: P2PTestFindingCommonChainASyncBNew
	*/

	//用例名称
	std::string P2PTestFindingCommonChainASyncBNew::name() const
	{
		return "P2PTestFindingCommonChainASyncBNew";
	}

	//用于用例初始化
	void P2PTestFindingCommonChainASyncBNew::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);

		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		m_passTest = false;

		ctrace << "P2PTestFindingCommonChainASyncBNew::init";
	}

	//用例销毁
	void P2PTestFindingCommonChainASyncBNew::destroy()
	{
		ctrace << "P2PTestFindingCommonChainASyncBNew::destroy";
	}

	//用来解析传来的协议包
	void P2PTestFindingCommonChainASyncBNew::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		switch (_t)
		{
		case DisconnectPacket:
		{
			m_passTest = false;
			string reason = "Unspecified";
			auto r = (DisconnectReason)_r[0].toInt<int>();
			if (!_r[0].isInt())
				ctrace << "Disconnect (reason: no reason)";
			else
			{
				reason = reasonOf(r);
				ctrace << "Disconnect (reason: " << reason << ")";
			}
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

	void P2PTestFindingCommonChainASyncBNew::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestFindingCommonChainASyncBNew::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();

					auto& _client = m_hostProxy.getClient();
					auto& _chain = _client.getBlockChain();
					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}

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
	void P2PTestFindingCommonChainASyncBNew::step()
	{
		const string &subUnitTest = m_hostProxy.subTestName();
		P2PTestClient& _client = m_hostProxy.getClient();

		if (subUnitTest == "BlockNumberUnderIrr")
		{
			/// 传入新块高度小于当前节点的不可逆高度
			ctrace << "BlockNumberUnderIrr";
			bytes block = _client.getBlockChain().block(_client.getBlockChain().numberHash(5));
			m_hostProxy.sendNewBlock(block);
		}
		else if (subUnitTest == "NewBlockHash")
		{
			///新Hash 
			ctrace << "NewBlockHash";
			auto tid = std::this_thread::get_id();

			static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

			uint64_t number = s_eng();

			h256 bhash = dev::sha3(to_string(number));
			m_hostProxy.sendNewBlockHash(bhash, number);


		}
		else
		{
			ctrace << "Erro subtest!!!";
		}

		ctrace << "P2PTestFindingCommonChainASyncBNew::step()";
	}

	/*
	* UnitTest: P2PTestFindingCommonBlockHeader
	*/

	//用例名称
	std::string P2PTestFindingCommonBlockHeader::name() const
	{
		return "P2PTestFindingCommonBlockHeader";
	}

	//用例销毁
	void P2PTestFindingCommonBlockHeader::destroy()
	{
		ctrace << "P2PTestFindingCommonBlockHeader::destroy";
	}

	//用于用例初始化
	void P2PTestFindingCommonBlockHeader::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);
		m_passTest = false;

		ctrace << "P2PTestFindingCommonBlockHeader::init";
	}

	//用来解析传来的协议包
	void P2PTestFindingCommonBlockHeader::interpretProtocolPacket(PacketType _t, RLP const& _r)
	{
		switch (_t)
		{
		case DisconnectPacket:
		{
			m_passTest = false;
			string reason = "Unspecified";
			auto r = (DisconnectReason)_r[0].toInt<int>();
			if (!_r[0].isInt())
				ctrace << "Disconnect (reason: no reason)";
			else
			{
				reason = reasonOf(r);
				ctrace << "Disconnect (reason: " << reason << ")";
			}
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

	void P2PTestFindingCommonBlockHeader::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestFindingCommonBlockHeader::interpret";
		try
		{
			switch (_id)
			{
			case GetBlockHeadersPacket:
			{
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}
				auto& _client = m_hostProxy.getClient();
				auto& _chain = _client.getBlockChain();
				if (blockId.size() == 32) // block id is a hash
				{
					auto blockHash = blockId.toHash<h256>();
					if (!_chain.isKnown(blockHash))
						break;

					//auto headerBytes = _chain.headerData(blockHash);
					//m_hostProxy.sendBlockHeader(headerBytes);
					const string &subUnitTest = m_hostProxy.subTestName();
					if (subUnitTest == "BlockHeaderMuch" ) 
					{
						// 过滤块头数过多的消息
						ctrace << "case 0:interpret  BlockHeaderMuch";
						auto tid = std::this_thread::get_id();

						static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

						uint64_t number = s_eng();

						P2PTestClient& _client = m_hostProxy.getClient();

						m_hostProxy.sendBlockHeader(bytes(), 1);
					}
					else if (subUnitTest == "BlockHeaderZero")
					{
						//块头数为0
						ctrace << "case 1:interpret  BlockHeaderZero";

						m_hostProxy.sendBlockHeader(bytes(), 0);

					}
					else if (subUnitTest == "BlockHeaderIsnotRequest")
					{
						//块头数为1，非预期块头（即非询问的块头）
						ctrace << "case 2:interpret BlockHeaderIsnotRequest";
						auto tid = std::this_thread::get_id();

						static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

						uint64_t number = s_eng();

						P2PTestClient& _client = m_hostProxy.getClient();

						bytes header = _client.getBlockChain().headerData();

						m_hostProxy.sendBlockHeader(header);

					}
					else if(subUnitTest == "BlockHeaderNumberUnderIrr"|| subUnitTest == "BlockHeaderNumberIsCommon"|| subUnitTest == "BlockHeaderNumberUpperIrr"|| subUnitTest == "BlockHeaderNumberEqualIrr")
					{
						ctrace << "case 3/4/5/6:interpret ";
						auto headerBytes = _chain.headerData(blockHash);
						m_hostProxy.sendBlockHeader(headerBytes);
					}
					else if(subUnitTest == "BlockBodies")
					{
						ctrace << "case 7:interpret BlockBodies";
						auto tid = std::this_thread::get_id();

						static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

						uint64_t number = s_eng();

						m_hostProxy.sendBlockBodiesPacket(bytes(), 1);
					}
					else
					{
						ctrace << "Invalid subUnitTest name!";
					}
				}
				else // block id is a number
				{
					auto n = blockId.toInt<bigint>();
					h256 blockHash;
					clog(NetMessageSummary) << "GetBlockHeaders (" << n
						<< "max: " << maxHeaders
						<< "skip: " << skip << (reverse ? "reverse" : "") << ")";

					if (!reverse)
					{
						auto lastBlock = _chain.number();
						if (n > lastBlock || n <= 0)
							blockHash = {};
						else
						{
							blockHash = _chain.numberHash(static_cast<unsigned>(n)); // override start block hash with the hash of the top block we have
							ctrace << "blockHash = _chain.numberHash blockHash = " << blockHash;
						}
					}
					else if (n <= std::numeric_limits<unsigned>::max())
						blockHash = _chain.numberHash(static_cast<unsigned>(n));
					else {
						blockHash = {};
					}

					if (!_chain.isKnown(blockHash))
						break;

					auto headerBytes = _chain.headerData(blockHash);
					m_hostProxy.sendBlockHeader(headerBytes);
				}

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
	void P2PTestFindingCommonBlockHeader::step()
	{

		/// 0:过滤块头数过多的消息
		/// 1:块头数为0
		/// 2:块头数为1，非预期块头（即非询问的块头）
		/// 3:块头数为1，预期块头，且块头高度小于当前不可逆 
		/// 4:块头数为1，预期块头，且为当前客户端链上块头，即找到相同块头
		/// 5:块头数为1，预期块头，块头来不同分叉，且块高度大于当前节点不可逆高度
		/// 6:块头数为1，预期块头，块头来不同分叉，且块高度等于当前节点的不可逆，且不等于当前节点最新不可逆
		/// 7:块体信息
		const string &subUnitTest = m_hostProxy.subTestName();
		if (subUnitTest == "BlockHeaderZero" || subUnitTest == "BlockHeaderMuch"|| subUnitTest == "BlockHeaderIsnotRequest")
		{
			ctrace << "case 0/1/2:"+ subUnitTest;
			const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		}
		else if (subUnitTest == "BlockHeaderNumberUnderIrr" )
		{
			ctrace << "case 3:BlockHeaderNumberUnderIrr";
			const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.numberHash(10), bc.genesisHash(), 4, bc.numberHash(4));
		}
		else if (subUnitTest == "BlockHeaderNumberIsCommon")
		{
			ctrace << "case 4:BlockHeaderNumberIsCommon";
			const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.numberHash(24), bc.genesisHash(), 12, bc.numberHash(12));
		}
		else if (subUnitTest == "BlockHeaderNumberUpperIrr")
		{
			ctrace << "case 5:BlockHeaderNumberUpperIrr";
			const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.numberHash(25), bc.genesisHash(), 12, bc.numberHash(12));
		}
		else if (subUnitTest == "BlockHeaderNumberEqualIrr")
		{
			ctrace << "case 6:BlockHeaderNumberEqualIrr";
			const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		}
		else if (subUnitTest == "BlockBodies")
		{
			ctrace << "case 7:BlockBodies";
			const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		}
		else
		{
			ctrace << "Invalid subUnitTest name!";
		}

		ctrace << "P2PTestFindingCommonNewBlockHeader::step()";
	}


	/*
	* UnitTest: P2PTestSyncBlocks
	*/

	std::string P2PTestSyncBlocks::name() const
	{
		return "P2PTestSyncBlocks";
	}

	//用例销毁
	void P2PTestSyncBlocks::destroy()
	{
		ctrace << "P2PTestSyncBlocks::destroy";
	}

	//用来解析传来的协议包
	void P2PTestSyncBlocks::interpretProtocolPacket(PacketType _t, RLP const& _r)
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
		case PingPacket:
		{
			//m_hostProxy.sendPong();
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
	
	void P2PTestSyncBlocks::interpret(unsigned _id, RLP const& _r)
	{
		ctrace << "P2PTestSyncBlocks::interpret";
		const string& subUnitTest = m_hostProxy.subTestName();
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
			case GetBlockHeadersPacket:
			{
				/// Packet layout:
				/// [ block: { P , B_32 }, maxHeaders: P, skip: P, reverse: P in { 0 , 1 } ]
				const auto blockId = _r[0];
				const auto maxHeaders = _r[1].toInt<u256>();
				const auto skip = _r[2].toInt<u256>();
				const auto reverse = _r[3].toInt<bool>();

				auto numHeadersToSend = maxHeaders <= 1024 ? static_cast<unsigned>(maxHeaders) : 1024;

				if (skip > std::numeric_limits<unsigned>::max() - 1)
				{
					clog(NetAllDetail) << "Requested block skip is too big: " << skip;
					break;
				}

				pair<bytes, unsigned> rlpAndItemCount;

				if (subUnitTest == "NewHeaderInvalidHeaderMiddle")
				{
					ctrace << "GetBlockHeadersPacket - NewHeaderInvalidHeaderMiddle";
					rlpAndItemCount = m_hostProxy.blockHeaders(blockId, numHeadersToSend, skip, reverse, numHeadersToSend <= 2 ? -1 : numHeadersToSend-2);
				}
				else if (subUnitTest == "NewHeaderInvalidHeaderTail")
				{
					ctrace << "GetBlockHeadersPacket - NewHeaderInvalidHeaderTail";
					rlpAndItemCount = m_hostProxy.blockHeaders(blockId, numHeadersToSend, skip, reverse, numHeadersToSend <= 1 ? -1 : numHeadersToSend-1);
				}
				else
				{
					rlpAndItemCount = m_hostProxy.blockHeaders(blockId, numHeadersToSend, skip, reverse);
				}


				m_hostProxy.sendBlockHeader(rlpAndItemCount.first, rlpAndItemCount.second);
				break;
			}
			case GetBlockBodiesPacket:
			{
				unsigned count = static_cast<unsigned>(_r.itemCount());
				clog(NetMessageSummary) << "GetBlockBodies (" << dec << count << "entries)";

				if (!count)
				{
					clog(NetImpolite) << "Zero-entry GetBlockBodies: Not replying.";
					break;
				}

				pair<bytes, unsigned> const rlpAndItemCount = m_hostProxy.blockBodies(_r);

				m_hostProxy.sendBlockBodiesPacket(rlpAndItemCount.first, rlpAndItemCount.second);
				break;
			}
			case NewBlockPacket:
			{
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

	//用于用例初始化
	void P2PTestSyncBlocks::init()
	{
		NodeID id = NodeID("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
		m_hostProxy.connectToHost(id);

		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();
		if (bc.number() <= 0)
		{
			ctrace << "P2PTestNewBlockAttack::init error, no specified a filled file!";
			return;
		}

		m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
			bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());

		ctrace << "P2PTestNewBlockAttack::init";
	}

	//在host线程
	void P2PTestSyncBlocks::step()
	{
		const string& subUnitTest = m_hostProxy.subTestName();
		const BlockChain& bc = m_hostProxy.getClient().getBlockChain();

		if (subUnitTest == "ValidStatus")
		{
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		}
		else if (subUnitTest == "InvalidNetworkIDStatus")
		{
			m_hostProxy.requestStatus(u256(), bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		}
		else if (subUnitTest == "InvalidGenesisHashStatus")
		{
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), h256(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash());
		}
		else if (subUnitTest == "InvalidProtocolVersionStatus")
		{
			m_hostProxy.requestStatus(bc.chainParams().networkID, bc.details().totalDifficulty,
				bc.currentHash(), bc.genesisHash(), bc.getIrreversibleBlock(), bc.getIrreversibleBlockHash(), 62);
		}
		else if (subUnitTest == "NewBlockPeerIrrLowerThanIrr")
		{
			auto& bc = m_hostProxy.getClient().getBlockChain();
			auto block = bc.block();
			m_hostProxy.sendNewBlock(block, 11, bc.numberHash(11));
		}
		else if (subUnitTest == "NewBlockBlockNumLowerThanIrr")
		{
			auto& bc = m_hostProxy.getClient().getBlockChain();
			auto block = bc.block(bc.numberHash(11));
			m_hostProxy.sendNewBlock(block);
		}
		else if (subUnitTest == "NewHash")
		{
			auto& bc = m_hostProxy.getClient().getBlockChain();
			m_hostProxy.sendNewBlockHash(bc.currentHash(), 1);
		}
		else if (subUnitTest == "NewHeaderInvalidHeaderMiddle")
		{
			ctrace << "NewHeaderInvalidHeaderMiddle";

		}
		else if (subUnitTest == "NewHeaderInvalidHeaderTail")
		{
			ctrace << "NewHeaderInvalidHeaderTail";

		}
		else
		{
			ctrace << "Invalid subUnitTest name!";
		}

		ctrace << "P2PTestSyncBlocks::step()";
	}
}
