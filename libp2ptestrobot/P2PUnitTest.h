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
		
		//��������
		virtual std::string name() const = 0; 

		//����������ʼ��
		virtual void init() = 0;

		//��������
		virtual void destroy() = 0;

		//��������������Э���
		virtual void interpret(unsigned _id, RLP const& _r) = 0;
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r) = 0;

		//��host�߳�
		virtual void step() = 0;

	protected:
		P2PHostProxy& m_hostProxy;
	};

	class PeerInstance
	{
	public:
		PeerInstance(P2PHostProxy& _proxy) :m_hostProxy(_proxy) {}
		~PeerInstance() {}

		//Peerʵ����ʼ��
		virtual void init() {}

		//Peerʵ��������
		virtual void destroy() {}

		//��������������Э���
		virtual void interpret(unsigned _id, RLP const& _r) {}

		//��host�߳�
		virtual void step() {}

	protected:
		P2PHostProxy& m_hostProxy;
	};

	class P2PHostProxy
	{
	public:
		P2PHostProxy(dev::p2p::FakeHost& _h, boost::asio::io_service& _ioService);

		~P2PHostProxy() {} 
		 
		RLPStream& prep(RLPStream& _s, unsigned _id, unsigned _args);  
		void sealAndSend(dev::RLPStream& _s);   
		void interpret(unsigned _id, RLP const& _r); 
		void interpretProtocolPacket(PacketType _t, RLP const& _r);
		void run(boost::system::error_code const&);

		void connectToHost(NodeID const& _id);
		void sendToHost(bytes& _s);
		void recvFromHost(bytes& _s);

		void onSessionClosed(NodeID const& _id);

	public: //��������ע��
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
		//TODO:������Ҫ�ɵ�
		void registerAllUnitTest();

	public: //����Э�����ֺ���
		void requestStatus(u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash, u256 _lastIrrBlock);
		void requestBlockHeaders(dev::h256 const& _startHash, unsigned _count, unsigned _skip, bool _reverse);
		void requestBlockHeaders(unsigned _startNumber, unsigned _count, unsigned _skip, bool _reverse);
		 

	protected:
		dev::p2p::FakeHost& m_host; 

		static std::vector<P2PUnitTest*> m_unitTestList;

		static int m_currTest;
		boost::asio::io_service& m_ioService;
		shared_ptr<boost::asio::deadline_timer> m_timer;
	};

	
	class P2PTestDriveUnitTest : public P2PUnitTest
	{
	public:
		P2PTestDriveUnitTest(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestDriveUnitTest() {}

		//��������
		virtual std::string name() const;

		//����������ʼ��
		virtual void init();

		//��������
		virtual void destroy();

		//��������������Э���
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//��host�߳�
		virtual void step();

	};

	class P2PTestRequestHeaderAttack : public P2PUnitTest
	{
	public:
		P2PTestRequestHeaderAttack(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestRequestHeaderAttack() {}

		//��������
		virtual std::string name() const;

		//����������ʼ��
		virtual void init();

		//��������
		virtual void destroy();

		//��������������Э���
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//��host�߳�
		virtual void step();

	};

	class P2PTestNewBlockAttack : public P2PUnitTest
	{
	public:
		P2PTestNewBlockAttack(P2PHostProxy& _proxy) :P2PUnitTest(_proxy) {}
		~P2PTestNewBlockAttack() {}

		//��������
		virtual std::string name() const;

		//����������ʼ��
		virtual void init();

		//��������
		virtual void destroy();

		//��������������Э���
		virtual void interpret(unsigned _id, RLP const& _r);
		virtual void interpretProtocolPacket(PacketType _t, RLP const& _r);

		//��host�߳�
		virtual void step();

	};
}