#pragma once
#include <libdevcore/FixedHash.h>
#include <libdevcore/RLP.h>
#include <libp2p/FakeHost.h>
#include "P2PTestClient.hpp"
#include <vector>


namespace P2PTest {
	using namespace dev;
	using namespace dev::p2p;

	class P2PHostProxy;

	class P2PUnitTest
	{
	public:
		P2PUnitTest(P2PHostProxy& _proxy) :m_hostProxy(_proxy) {}
		~P2PUnitTest() {}
		
		//用例名称
		virtual std::string name() const = 0; 

		//用于用例初始化
		virtual void init() = 0;

		//用例销毁
		virtual void destroy() = 0;

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r) = 0;
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r) = 0;

		//在host线程
		virtual void step() = 0;

	protected:
		P2PHostProxy& m_hostProxy;
	};

	class PeerInstance
	{
	public:
		PeerInstance(P2PHostProxy& _proxy) :m_hostProxy(_proxy) {}
		~PeerInstance() {}

		//Peer实例初始化
		virtual void init() {}

		//Peer实例化销毁
		virtual void destroy() {}

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r) {}

		//在host线程
		virtual void step() {}

	protected:
		P2PHostProxy& m_hostProxy;
	};

	class P2PHostProxy
	{
	public:
		P2PHostProxy(dev::p2p::FakeHost& _h, boost::asio::io_service& _ioService);

		~P2PHostProxy() { for (auto& p : m_unitTestList) { if (p != nullptr) delete p; } }
		 
		RLPStream& prep(RLPStream& _s, unsigned _id, unsigned _args);  
		void sealAndSend(dev::RLPStream& _s);   
		void interpret(unsigned _id, RLP const& _r); 
		void interpretProtocolPacket(PacketType _t, RLP const& _r);
		void run(boost::system::error_code const&);

		void connectToHost(NodeID const& _id);
		void sendToHost(bytes& _s);
		void recvFromHost(bytes& _s);

		void onSessionClosed(NodeID const& _id);

		P2PTestClient& getClient() { return m_client; }

	public: //测试用例注册
		void registerUnitTest(P2PUnitTest* _unit);
		static void switchUnitTest(int i = m_currTest);
		unsigned unitTestCount() const { return m_unitTestList.size(); }
		int currUnitTest() const { return m_currTest; }
		P2PUnitTest* getCurrUnitTest() const; 

#if defined(_WIN32)
		static BOOL P2PHostProxy::CtrlHandler(DWORD fdwCtrlType);

#else
		static void switchSignalHandler(int signum);
#endif
		//TODO:后续需要干掉
		void registerAllUnitTest();

	public: //请求协议助手函数
		void requestStatus(u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash, u256 _lastIrrBlock, unsigned hostProtocolVersion = 63);
		void requestBlockHeaders(dev::h256 const& _startHash, unsigned _count, unsigned _skip, bool _reverse);
		void requestBlockHeaders(unsigned _startNumber, unsigned _count, unsigned _skip, bool _reverse);
		void sendNewBlockHash(h256& block, unsigned number);
		void getBlockBodiesPacket(h256& block, unsigned number);
		void sendBlockBodiesPacket(h256& block, unsigned number);
		void sendNewBlockPacket(h256& block, unsigned number);
		void sendReceiptsPacket(h256& block, unsigned number);
		void getReceiptsPacket(h256& block, unsigned number);
		void getNodeDataPacket(h256& block, unsigned number);
		void sendNodeDataPacket(h256& block, unsigned number);


	protected:
		dev::p2p::FakeHost& m_host; 

		static std::vector<P2PUnitTest*> m_unitTestList;

		static int m_currTest;
		boost::asio::io_service& m_ioService;
		shared_ptr<boost::asio::deadline_timer> m_timer;
		P2PTestClient m_client;
	};

	
	class P2PTestDriveUnitTest : public P2PUnitTest
	{
	public:
		P2PTestDriveUnitTest(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestDriveUnitTest() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();

	};

	class P2PTestRequestHeaderAttack : public P2PUnitTest
	{
	public:
		P2PTestRequestHeaderAttack(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestRequestHeaderAttack() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();

	};

	class P2PTestNewBlockAttack : public P2PUnitTest
	{
	public:
		P2PTestNewBlockAttack(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestNewBlockAttack() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();

	};

	class P2PTestStatusPacketAttack : public P2PUnitTest
	{
	public:
		P2PTestStatusPacketAttack(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestStatusPacketAttack() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();
	};

	class P2PTestInvalidStatusPacket : public P2PUnitTest
	{
	public:
		P2PTestInvalidStatusPacket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestInvalidStatusPacket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();
	private:
		bool m_passTest;
	};
	
	class P2PTestBlockChainIrrket : public P2PUnitTest
	{
	public:
		P2PTestBlockChainIrrket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestBlockChainIrrket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();
	private:
		bool m_passTest;
	};

	class P2PTestNewBlockHashesAttack : public P2PUnitTest
	{
	public:
		P2PTestNewBlockHashesAttack(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestNewBlockHashesAttack() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();
	};

	class P2PTestGetBlockBodiesPacket : public P2PUnitTest
	{
	public:
		P2PTestGetBlockBodiesPacket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestGetBlockBodiesPacket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();
	};

	class P2PTestBlockBodiesPacket : public P2PUnitTest
	{
	public:
		P2PTestBlockBodiesPacket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestBlockBodiesPacket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();
	};

	class P2PTestSendNewBlockPacket : public P2PUnitTest
	{
	public:
		P2PTestSendNewBlockPacket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestSendNewBlockPacket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();

	};

	class P2PTestGetNodeDataPacket : public P2PUnitTest
	{
	public:
		P2PTestGetNodeDataPacket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestGetNodeDataPacket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();

	};

	class P2PTestNodeDataPacket : public P2PUnitTest
	{
	public:
		P2PTestNodeDataPacket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestNodeDataPacket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();

	};

	class P2PTestGetReceiptsPacket : public P2PUnitTest
	{
	public:
		P2PTestGetReceiptsPacket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestGetReceiptsPacket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();

	};

	class P2PTestReceiptsPacket : public P2PUnitTest
	{
	public:
		P2PTestReceiptsPacket(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestReceiptsPacket() {}

		//用例名称
		virtual std::string name() const;

		//用于用例初始化
		virtual void init();

		//用例销毁
		virtual void destroy();

		//用来解析传来的协议包
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//在host线程
		virtual void step();

	};

}