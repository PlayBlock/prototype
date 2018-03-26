

/*
This file is part of cpp-ethereum.

cpp-ethereum is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

cpp-ethereum is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Host.cpp
* @author Alex Leverington <nessence@gmail.com>
* @author Gav Wood <i@gavwood.com>
* @date 2018
*/

#include <set>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <boost/algorithm/string.hpp>
#include <libdevcore/Common.h>
#include <libdevcore/Assertions.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/FileSystem.h>
#include <libp2p/Common.h>
#include <libp2p/Capability.h>
#include <libp2p/UPnP.h>
#include <libp2p/RLPxHandshake.h>
#include "FakeHost.h"
#include "FakePeer.h"
#include "FakeSession.h"
#include <libp2ptestrobot/P2PTestRobot.hpp>
#include <libethereum/Client.h>
using namespace std;
using namespace dev;
using namespace dev::p2p;

/// Interval at which FakeHost::run will call keepAlivePeers to ping peers.
std::chrono::seconds const c_keepAliveInterval = std::chrono::seconds(30);

/// Disconnect timeout after failure to respond to keepAlivePeers ping.
std::chrono::milliseconds const c_keepAliveTimeOut = std::chrono::milliseconds(1000);


FakeHost::FakeHost(string const& _clientVersion, KeyPair const& _alias, NetworkPreferences const& _n)
	: Host(_clientVersion, _alias, _n)
{
	m_hostProxy = new P2PTest::P2PHostProxy(*this, m_ioService);
}

FakeHost::FakeHost(string const& _clientVersion, NetworkPreferences const& _n, bytesConstRef _restoreNetwork) :
	Host(_clientVersion, _n, _restoreNetwork)
{
	m_hostProxy = new P2PTest::P2PHostProxy(*this, m_ioService);
}

FakeHost::~FakeHost()
{
	if (m_hostProxy != nullptr)
		delete m_hostProxy;
}

void FakeHost::start()
{
	DEV_TIMED_FUNCTION_ABOVE(500);
	startWorking();
	while (isWorking() && !haveNetwork())
		this_thread::sleep_for(chrono::milliseconds(10));

	// network start failed!
	if (isWorking())
		return;

	clog(NetWarn) << "Network start failed!";
	doneWorking();
}

void FakeHost::stop()
{
	// called to force io_service to kill any remaining tasks it might have -
	// such tasks may involve socket reads from Capabilities that maintain references
	// to resources we're about to free.

	// ignore if already stopped/stopping, at the same time,
	// signal run() to prepare for shutdown and reset m_timer
	if (!m_run.exchange(false))
		return;

	{
		unique_lock<mutex> l(x_runTimer);
		while (m_timer)
			m_timerReset.wait(l);
	}

	// stop worker thread
	if (isWorking())
		stopWorking();
}

void FakeHost::doneWorking()
{

	DEV_GUARDED(x_timers)
		m_timers.clear();


	// stop capabilities (eth: stops syncing or block/tx broadcast)
	for (auto const& h : m_capabilities)
		h.second->onStopping();

	// disconnect pending handshake, before peers, as a handshake may create a peer
	for (unsigned n = 0;; n = 0)
	{
		DEV_GUARDED(x_connecting)
			for (auto const& i : m_connecting)
				if (auto h = i.lock())
				{
					h->cancel();
					n++;
				}
		if (!n)
			break;
	}

	// disconnect peers
	for (unsigned n = 0;; n = 0)
	{
		DEV_RECURSIVE_GUARDED(x_sessions)
			for (auto i : m_sessions)
				if (auto p = i.second.lock())
					if (p->isConnected())
					{
						p->disconnect(ClientQuit);
						n++;
					}
		if (!n)
			break;

	}

	// finally, clear out peers (in case they're lingering)
	RecursiveGuard l(x_sessions);
	m_sessions.clear();
}

bool FakeHost::isRequiredPeer(NodeID const& _id) const
{
	Guard l(x_requiredPeers);
	return m_requiredPeers.count(_id);
}

bool FakeHost::isEnabled(NodeID const& _id)
{
	if (m_sessions.count(_id) == 0)
		return false;

	return m_sessions[_id].lock()->isConnected();
}

void FakeHost::startPeerSession(Public const& _id, RLP const& _rlp, unique_ptr<RLPXFrameCoder>&& _io, std::shared_ptr<RLPXSocket> const& _s)
{
	// session maybe ingress or egress so m_peers and node table entries may not exist
	shared_ptr<Peer> p;
	DEV_RECURSIVE_GUARDED(x_sessions)
	{
		if (m_peers.count(_id))
			p = m_peers[_id];
		else
		{
			// peer doesn't exist, try to get port info from node table
			if (Node n = nodeFromNodeTable(_id))
				p = make_shared<FakePeer>(n);

			if (!p)
				p = make_shared<FakePeer>(Node(_id, UnspecifiedNodeIPEndpoint));

			m_peers[_id] = p;
		}
	}
	if (p->isOffline())
		p->m_lastConnected = std::chrono::system_clock::now();
	p->endpoint.address = _s->remoteEndpoint().address();

	auto protocolVersion = _rlp[0].toInt<unsigned>();
	auto clientVersion = _rlp[1].toString();
	auto caps = _rlp[2].toVector<CapDesc>();
	auto listenPort = _rlp[3].toInt<unsigned short>();
	auto pub = _rlp[4].toHash<Public>();

	if (pub != _id)
	{
		cdebug << "Wrong ID: " << pub << " vs. " << _id;
		return;
	}

	// clang error (previously: ... << hex << caps ...)
	// "'operator<<' should be declared prior to the call site or in an associated namespace of one of its arguments"
	stringstream capslog;

	// leave only highset mutually supported capability version
	caps.erase(remove_if(caps.begin(), caps.end(), [&](CapDesc const& _r) { return !haveCapability(_r) || any_of(caps.begin(), caps.end(), [&](CapDesc const& _o) { return _r.first == _o.first && _o.second > _r.second && haveCapability(_o); }); }), caps.end());

	for (auto cap : caps)
		capslog << "(" << cap.first << "," << dec << cap.second << ")";

	clog(NetMessageSummary) << "Hello: " << clientVersion << "V[" << protocolVersion << "]" << _id << showbase << capslog.str() << dec << listenPort;

	// create session so disconnects are managed
	shared_ptr<SessionFace> ps = make_shared<Session>(this, nullptr, nullptr, p, PeerSessionInfo({ _id, clientVersion, p->endpoint.address.to_string(), listenPort, chrono::steady_clock::duration(), _rlp[2].toSet<CapDesc>(), 0, map<string, string>(), protocolVersion }));
	if (protocolVersion < dev::p2p::c_protocolVersion - 1)
	{
		ps->disconnect(IncompatibleProtocol);
		return;
	}
	if (caps.empty())
	{
		ps->disconnect(UselessPeer);
		return;
	}

	if (m_netPrefs.pin && !isRequiredPeer(_id))
	{
		cdebug << "Unexpected identity from peer (got" << _id << ", must be one of " << m_requiredPeers << ")";
		ps->disconnect(UnexpectedIdentity);
		return;
	}

	{
		RecursiveGuard l(x_sessions);
		if (m_sessions.count(_id) && !!m_sessions[_id].lock())
			if (auto s = m_sessions[_id].lock())
				if (s->isConnected())
				{
					// Already connected.
					clog(NetWarn) << "Session already exists for peer with id" << _id;
					ps->disconnect(DuplicatePeer);
					return;
				}

		if (!peerSlotsAvailable())
		{
			ps->disconnect(TooManyPeers);
			return;
		}

		unsigned offset = (unsigned)UserPacket;

		// todo: mutex Session::m_capabilities and move for(:caps) out of mutex.
		for (auto const& i : caps)
		{
			auto pcap = m_capabilities[i];
			if (!pcap)
				return ps->disconnect(IncompatibleProtocol);

			pcap->newPeerCapability(ps, offset, i);
			offset += pcap->messageCount();
		}

		ps->start();
		m_sessions[_id] = ps;
	}

	clog(NetP2PNote) << "p2p.host.peer.register" << _id;
}

void FakeHost::CreatePeerSession(Public const& _id, RLP const& _rlp, unique_ptr<RLPXFrameCoder>&& _io, std::shared_ptr<RLPXSocket> const& _s)
{
	// session maybe ingress or egress so m_peers and node table entries may not exist
	shared_ptr<Peer> p;
	DEV_RECURSIVE_GUARDED(x_sessions)
	{
		if (m_peers.count(_id))
			p = m_peers[_id];
		else
		{
			// peer doesn't exist, try to get port info from node table
			if (Node n = nodeFromNodeTable(_id))
				p = make_shared<FakePeer>(n);

			if (!p)
				p = make_shared<FakePeer>(Node(_id, UnspecifiedNodeIPEndpoint));

			m_peers[_id] = p;
		}
	}
	if (p->isOffline())
		p->m_lastConnected = std::chrono::system_clock::now();
    boost::asio::ip::address address(bi::address_v4::from_string("192.168.1.111"));
	p->endpoint.address = address;

	unsigned protocolVersion = 63;
	string clientVersion = "eth/v1.3.0/Linux/g++/Interpreter/RelWithDebInfo/af98af3c*/";
	vector<CapDesc> caps;
	caps.push_back(std::pair<std::string, u256>("eth", u256(63)));
	unsigned short listenPort = 30303;

	//if (pub != _id)
	//{
	//	cdebug << "Wrong ID: " << pub << " vs. " << _id;
	//	return;
	//}

	// clang error (previously: ... << hex << caps ...)
	// "'operator<<' should be declared prior to the call site or in an associated namespace of one of its arguments"
	stringstream capslog;

	// leave only highset mutually supported capability version
	caps.erase(remove_if(caps.begin(), caps.end(), [&](CapDesc const& _r) { return !haveCapability(_r) || any_of(caps.begin(), caps.end(), [&](CapDesc const& _o) { return _r.first == _o.first && _o.second > _r.second && haveCapability(_o); }); }), caps.end());

	for (auto cap : caps)
		capslog << "(" << cap.first << "," << dec << cap.second << ")";

	clog(NetMessageSummary) << "Hello: " << clientVersion << "V[" << protocolVersion << "]" << _id << showbase << capslog.str() << dec << listenPort;
	set<CapDesc> caps_set;
	caps_set.insert(std::pair<std::string, u256>("eth", u256(62)));
	// create session so disconnects are managed
	m_ps = make_shared<FakeSession>(this, p, PeerSessionInfo({ _id, clientVersion, p->endpoint.address.to_string(), listenPort, chrono::steady_clock::duration(), caps_set, 0, map<string, string>(), protocolVersion }));
	if (protocolVersion < dev::p2p::c_protocolVersion - 1)
	{
		m_ps->disconnect(IncompatibleProtocol);
		return;
	}
	if (caps.empty())
	{
		m_ps->disconnect(UselessPeer);
		return;
	}

	if (m_netPrefs.pin && !isRequiredPeer(_id))
	{
		cdebug << "Unexpected identity from peer (got" << _id << ", must be one of " << m_requiredPeers << ")";
		m_ps->disconnect(UnexpectedIdentity);
		return;
	}

	{
		RecursiveGuard l(x_sessions);
		if (m_sessions.count(_id) && !!m_sessions[_id].lock())
			if (auto s = m_sessions[_id].lock())
				if (s->isConnected())
				{
					// Already connected.
					clog(NetWarn) << "Session already exists for peer with id" << _id;
					m_ps->disconnect(DuplicatePeer);
					return;
				}

		if (!peerSlotsAvailable())
		{
			m_ps->disconnect(TooManyPeers);
			return;
		}

		unsigned offset = (unsigned)UserPacket;

		// todo: mutex Session::m_capabilities and move for(:caps) out of mutex.
		for (auto const& i : caps)
		{
			auto pcap = m_capabilities[i];
			if (!pcap)
				return m_ps->disconnect(IncompatibleProtocol);

			pcap->newPeerCapability(m_ps, offset, i);
			offset += pcap->messageCount();
		}

		//ps->start();
		m_sessions[_id] = m_ps;
	}

	clog(NetP2PNote) << "p2p.host.peer.register" << _id;
}

void FakeHost::connectToHost(NodeID const& _id)
{

	NodeID  m_remote("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");

	CreatePeerSession(_id, RLP(), nullptr, nullptr);

}

bool FakeHost::checkPacket(bytesConstRef _msg)
{
	if (_msg[0] > 0x7f || _msg.size() < 2)
		return false;
	if (RLP(_msg.cropped(1)).actualSize() + 1 != _msg.size())
		return false;
	return true;
}

void FakeHost::sendToHost(bytes const& _r)
{
	NodeID  id("8620a3dafd797199dfe24f1378fabc7de62c01569e4b1c4953cc0fef60cf89b6b4bd69fac1462c8c4f549e0c934ce11f5d85f1dfb4e62c4f57779a89d6964fe6");
	uint16_t hProtocolId =63;
	//PacketType type = UserPacket;

	bytesConstRef frame(_r.data(), _r.size());

	if (!checkPacket(frame))
	{
		cerr << "Received " << frame.size() << ": " << toHex(frame) << endl;
		clog(NetWarn) << "INVALID MESSAGE RECEIVED";
		return;
	}
	else
	{
		auto packetType = (PacketType)RLP(frame.cropped(0, 1)).toInt<unsigned>();
		RLP r(frame.cropped(1));
		bool ok = m_sessions[id].lock()->readPacket(hProtocolId, packetType, r);
		if (!ok)
			clog(NetWarn) << "Couldn't interpret packet." << RLP(r);
	}
}
void FakeHost::recvFromHost(NodeID& _id, bytes const & _r)
{
	bytes r(_r);
	m_hostProxy->recvFromHost(r);
}

void FakeHost::onNodeTableEvent(NodeID const& _n, NodeTableEventType const& _e)
{
	if (_e == NodeEntryAdded)
	{
		clog(NetP2PNote) << "p2p.host.nodeTable.events.nodeEntryAdded " << _n;
		if (Node n = nodeFromNodeTable(_n))
		{
			shared_ptr<Peer> p;
			DEV_RECURSIVE_GUARDED(x_sessions)
			{
				if (m_peers.count(_n))
				{
					p = m_peers[_n];
					p->endpoint = n.endpoint;
				}
				else
				{
					p = make_shared<FakePeer>(n);
					m_peers[_n] = p;
					clog(NetP2PNote) << "p2p.host.peers.events.peerAdded " << _n << p->endpoint;
				}
			}
			if (peerSlotsAvailable(Egress))
				connect(p);
		}
	}
	else if (_e == NodeEntryDropped)
	{
		clog(NetP2PNote) << "p2p.host.nodeTable.events.NodeEntryDropped " << _n;
		RecursiveGuard l(x_sessions);
		if (m_peers.count(_n) && m_peers[_n]->peerType == PeerType::Optional)
			m_peers.erase(_n);
	}
}

void FakeHost::determinePublic()
{
	// set m_tcpPublic := listenIP (if public) > public > upnp > unspecified address.

	auto ifAddresses = Network::getInterfaceAddresses();
	auto laddr = m_netPrefs.listenIPAddress.empty() ? bi::address() : bi::address::from_string(m_netPrefs.listenIPAddress);
	auto lset = !laddr.is_unspecified();
	auto paddr = m_netPrefs.publicIPAddress.empty() ? bi::address() : bi::address::from_string(m_netPrefs.publicIPAddress);
	auto pset = !paddr.is_unspecified();

	bool listenIsPublic = lset && isPublicAddress(laddr);
	bool publicIsHost = !lset && pset && ifAddresses.count(paddr);

	bi::tcp::endpoint ep(bi::address(), m_listenPort);
	if (m_netPrefs.traverseNAT && listenIsPublic)
	{
		clog(NetNote) << "Listen address set to Public address:" << laddr << ". UPnP disabled.";
		ep.address(laddr);
	}
	else if (m_netPrefs.traverseNAT && publicIsHost)
	{
		clog(NetNote) << "Public address set to Host configured address:" << paddr << ". UPnP disabled.";
		ep.address(paddr);
	}
	else if (m_netPrefs.traverseNAT)
	{
		bi::address natIFAddr;
		ep = Network::traverseNAT(lset && ifAddresses.count(laddr) ? std::set<bi::address>({ laddr }) : ifAddresses, m_listenPort, natIFAddr);

		if (lset && natIFAddr != laddr)
			// if listen address is set, Host will use it, even if upnp returns different
			clog(NetWarn) << "Listen address" << laddr << "differs from local address" << natIFAddr << "returned by UPnP!";

		if (pset && ep.address() != paddr)
		{
			// if public address is set, Host will advertise it, even if upnp returns different
			clog(NetWarn) << "Specified public address" << paddr << "differs from external address" << ep.address() << "returned by UPnP!";
			ep.address(paddr);
		}
	}
	else if (pset)
		ep.address(paddr);

	m_tcpPublic = ep;
}

void FakeHost::runAcceptor()
{
	assert(m_listenPort > 0);

	if (m_run && !m_accepting)
	{
		clog(NetConnect) << "Listening on local port " << m_listenPort << " (public: " << m_tcpPublic << ")";
		m_accepting = true;

		auto socket = make_shared<RLPXSocket>(m_ioService);
		m_tcp4Acceptor.async_accept(socket->ref(), [=](boost::system::error_code ec)
		{
			m_accepting = false;
			if (ec || !m_run)
			{
				socket->close();
				return;
			}
			if (peerCount() > peerSlots(Ingress))
			{
				clog(NetConnect) << "Dropping incoming connect due to maximum peer count (" << Ingress << " * ideal peer count): " << socket->remoteEndpoint();
				socket->close();
				if (ec.value() < 1)
					runAcceptor();
				return;
			}

			bool success = false;
			try
			{
				// incoming connection; we don't yet know nodeid
				auto handshake = make_shared<RLPXHandshake>(this, socket);
				m_connecting.push_back(handshake);
				handshake->start();
				success = true;
			}
			catch (Exception const& _e)
			{
				clog(NetWarn) << "ERROR: " << diagnostic_information(_e);
			}
			catch (std::exception const& _e)
			{
				clog(NetWarn) << "ERROR: " << _e.what();
			}

			if (!success)
				socket->ref().close();
			runAcceptor();
		});
	}
}

void FakeHost::addPeer(NodeSpec const& _s, PeerType _t)
{
	if (_t == PeerType::Optional)
		addNode(_s.id(), _s.nodeIPEndpoint());
	else
		requirePeer(_s.id(), _s.nodeIPEndpoint());
}

void FakeHost::addNode(NodeID const& _node, NodeIPEndpoint const& _endpoint)
{
	// return if network is stopped while waiting on FakeHost::run() or nodeTable to start
	while (!haveNetwork())
		if (isWorking())
			this_thread::sleep_for(chrono::milliseconds(50));
		else
			return;

	if (_endpoint.tcpPort < 30300 || _endpoint.tcpPort > 30305)
		clog(NetConnect) << "Non-standard port being recorded: " << _endpoint.tcpPort;

	addNodeToNodeTable(Node(_node, _endpoint));
}

void FakeHost::requirePeer(NodeID const& _n, NodeIPEndpoint const& _endpoint)
{
	{
		Guard l(x_requiredPeers);
		m_requiredPeers.insert(_n);
	}

	if (!m_run)
		return;

	Node node(_n, _endpoint, PeerType::Required);
	if (_n)
	{
		// create or update m_peers entry
		shared_ptr<Peer> p;
		DEV_RECURSIVE_GUARDED(x_sessions)
			if (m_peers.count(_n))
			{
				p = m_peers[_n];
				p->endpoint = node.endpoint;
				p->peerType = PeerType::Required;
			}
			else
			{
				p = make_shared<FakePeer>(node);
				m_peers[_n] = p;
			}
		// required for discovery
		addNodeToNodeTable(*p, NodeTable::NodeRelation::Unknown);
	}
	else
	{
		if (!addNodeToNodeTable(node))
			return;
		auto t = make_shared<boost::asio::deadline_timer>(m_ioService);
		t->expires_from_now(boost::posix_time::milliseconds(600));
		t->async_wait([this, _n](boost::system::error_code const& _ec)
		{
			if (!_ec)
				if (auto n = nodeFromNodeTable(_n))
					requirePeer(n.id, n.endpoint);
		});
		DEV_GUARDED(x_timers)
			m_timers.push_back(t);
	}
}

void FakeHost::relinquishPeer(NodeID const& _node)
{
	Guard l(x_requiredPeers);
	if (m_requiredPeers.count(_node))
		m_requiredPeers.erase(_node);
}

void FakeHost::connect(std::shared_ptr<Peer> const& _p)
{
	//if (!m_run)
	//	return;

	if (havePeerSession(_p->id))
	{
		clog(NetConnect) << "Aborted connect. Node already connected.";
		return;
	}

	if (!nodeTableHasNode(_p->id) && _p->peerType == PeerType::Optional)
		return;

	// prevent concurrently connecting to a node
	FakePeer *nptr = dynamic_cast<FakePeer *>(_p.get());
	{
		Guard l(x_pendingNodeConns);
		if (m_pendingPeerConns.count(nptr))
			return;
		m_pendingPeerConns.insert(nptr);
	}

	_p->m_lastAttempted = std::chrono::system_clock::now();

	//bi::tcp::endpoint ep(_p->endpoint);
	//clog(NetConnect) << "Attempting connection to node" << _p->id << "@" << ep << "from" << id();
	//auto socket = make_shared<RLPXSocket>(m_ioService);
	//socket->ref().async_connect(ep, [=](boost::system::error_code const& ec)
	//{
	//	_p->m_lastAttempted = std::chrono::system_clock::now();
	//	_p->m_failedAttempts++;

	//	if (ec)
	//	{
	//		clog(NetConnect) << "Connection refused to node" << _p->id << "@" << ep << "(" << ec.message() << ")";
	//		// Manually set error (session not present)
	//		_p->m_lastDisconnect = TCPError;
	//	}
	//	else
	//	{
	//		clog(NetConnect) << "Connecting to" << _p->id << "@" << ep;
	//		auto handshake = make_shared<RLPXHandshake>(this, socket, _p->id);
	//		{
	//			Guard l(x_connecting);
	//			m_connecting.push_back(handshake);
	//		}

	//		handshake->start();
	//	}

	//	Guard l(x_pendingNodeConns);
	//	m_pendingPeerConns.erase(nptr);
	//});
}

PeerSessionInfos FakeHost::peerSessionInfo() const
{
	if (!m_run)
		return PeerSessionInfos();

	std::vector<PeerSessionInfo> ret;
	RecursiveGuard l(x_sessions);
	for (auto& i : m_sessions)
		if (auto j = i.second.lock())
			if (j->isConnected())
				ret.push_back(j->info());
	return ret;
}

size_t FakeHost::peerCount() const
{
	unsigned retCount = 0;
	RecursiveGuard l(x_sessions);
	for (auto& i : m_sessions)
		if (std::shared_ptr<SessionFace> j = i.second.lock())
			if (j->isConnected())
				retCount++;
	return retCount;
}

void FakeHost::run(boost::system::error_code const&)
{
	if (!m_run)
	{
		// reset NodeTable
		DEV_GUARDED(x_nodeTable)
			m_nodeTable.reset();

		// stopping io service allows running manual network operations for shutdown
		// and also stops blocking worker thread, allowing worker thread to exit
		m_ioService.stop();

		// resetting timer signals network that nothing else can be scheduled to run
		DEV_GUARDED(x_runTimer)
			m_timer.reset();

		m_timerReset.notify_all();
		return;
	}

	if (auto nodeTable = this->nodeTable()) // This again requires x_nodeTable, which is why an additional variable nodeTable is used.
		nodeTable->processEvents();

	// cleanup zombies
	DEV_GUARDED(x_connecting)
		m_connecting.remove_if([](std::weak_ptr<RLPXHandshake> h) { return h.expired(); });
	DEV_GUARDED(x_timers)
		m_timers.remove_if([](std::shared_ptr<boost::asio::deadline_timer> t)
	{
		return t->expires_from_now().total_milliseconds() < 0;
	});

	keepAlivePeers();

	// At this time peers will be disconnected based on natural TCP timeout.
	// disconnectLatePeers needs to be updated for the assumption that Session
	// is always live and to ensure reputation and fallback timers are properly
	// updated. // disconnectLatePeers();

	// todo: update peerSlotsAvailable()

	list<shared_ptr<Peer>> toConnect;
	unsigned reqConn = 0;
	{
		RecursiveGuard l(x_sessions);
		for (auto const& p : m_peers)
		{
			bool haveSession = havePeerSession(p.second->id);
			bool required = p.second->peerType == PeerType::Required;
			if (haveSession && required)
				reqConn++;
			else if (!haveSession && p.second->shouldReconnect() && (!m_netPrefs.pin || required))
				toConnect.push_back(p.second);
		}
	}

	for (auto p : toConnect)
		if (p->peerType == PeerType::Required && reqConn++ < m_idealPeerCount)
			connect(p);

	if (!m_netPrefs.pin)
	{
		unsigned pendingCount = 0;
		DEV_GUARDED(x_pendingNodeConns)
			pendingCount = m_pendingPeerConns.size();
		int openSlots = m_idealPeerCount - peerCount() - pendingCount + reqConn;
		if (openSlots > 0)
			for (auto p : toConnect)
				if (p->peerType == PeerType::Optional && openSlots--)
					connect(p);
	}

	auto runcb = [this](boost::system::error_code const& error) { run(error); };
	m_timer->expires_from_now(boost::posix_time::milliseconds(c_timerInterval));
	m_timer->async_wait(runcb);
}

void FakeHost::startedWorking()
{
	asserts(!m_timer);

	{
		// prevent m_run from being set to true at same time as set to false by stop()
		// don't release mutex until m_timer is set so in case stop() is called at same
		// time, stop will wait on m_timer and graceful network shutdown.
		Guard l(x_runTimer);
		// create deadline timer
		m_timer.reset(new boost::asio::deadline_timer(m_ioService));
		m_run = true;
	}

	// start capability threads (ready for incoming connections)
	for (auto const& h : m_capabilities)
		h.second->onStarting();

	// try to open acceptor (todo: ipv6)
	int port = Network::tcp4Listen(m_tcp4Acceptor, m_netPrefs);
	if (port > 0)
	{
		m_listenPort = port;
		determinePublic();
		runAcceptor();
	}
	else
		clog(NetP2PNote) << "p2p.start.notice id:" << id() << "TCP Listen port is invalid or unavailable.";

	auto nodeTable = make_shared<NodeTable>(
		m_ioService,
		m_alias,
		NodeIPEndpoint(bi::address::from_string(listenAddress()), listenPort(), listenPort()),
		m_netPrefs.discovery
		);
	nodeTable->setEventHandler(new HostNodeTableHandler(*this));
	DEV_GUARDED(x_nodeTable)
		m_nodeTable = nodeTable;
	restoreNetwork(&m_restoreNetwork);

	clog(NetP2PNote) << "p2p.started id:" << id();


	run(boost::system::error_code());
	if (g_P2PUnitTestName.empty())
	{
		m_hostProxy->registerAttackUnitTest();
	}
	else
	{
		m_hostProxy->registerUnitTest(g_P2PUnitTestName);
	}
}


void FakeHost::doWork()
{
	try
	{ 
		m_hostProxy->run(boost::system::error_code());
		m_ioService.run();
	}
	catch (std::exception const& _e)
	{
		clog(NetP2PWarn) << "Exception in Network Thread:" << _e.what();
		clog(NetP2PWarn) << "Network Restart is Recommended.";
	}
}

void FakeHost::keepAlivePeers()
{
	if (chrono::steady_clock::now() - c_keepAliveInterval < m_lastPing)
		return;

	RecursiveGuard l(x_sessions);
	for (auto it = m_sessions.begin(); it != m_sessions.end();)
		if (auto p = it->second.lock())
		{
			p->ping();
			++it;
		}
		else
		{
			m_hostProxy->onSessionClosed(it->first);
			it = m_sessions.erase(it);
		}

	m_lastPing = chrono::steady_clock::now();
}

void FakeHost::disconnectLatePeers()
{
	auto now = chrono::steady_clock::now();
	if (now - c_keepAliveTimeOut < m_lastPing)
		return;

	RecursiveGuard l(x_sessions);
	for (auto p : m_sessions)
		if (auto pp = p.second.lock())
			if (now - c_keepAliveTimeOut > m_lastPing && pp->lastReceived() < m_lastPing)
				pp->disconnect(PingTimeout);
}

bytes FakeHost::saveNetwork() const
{
	std::list<Peer> peers;
	{
		RecursiveGuard l(x_sessions);
		for (auto p : m_peers)
			if (p.second)
				peers.push_back(*p.second);
	}
	peers.sort();

	RLPStream network;
	int count = 0;
	for (auto const& p : peers)
	{
		// todo: ipv6
		if (!p.endpoint.address.is_v4())
			continue;

		// Only save peers which have connected within 2 days, with properly-advertised port and public IP address
		if (chrono::system_clock::now() - p.m_lastConnected < chrono::seconds(3600 * 48) && !!p.endpoint && p.id != id() && (p.peerType == PeerType::Required || p.endpoint.isAllowed()))
		{
			network.appendList(11);
			p.endpoint.streamRLP(network, NodeIPEndpoint::StreamInline);
			network << p.id << (p.peerType == PeerType::Required ? true : false)
				<< chrono::duration_cast<chrono::seconds>(p.m_lastConnected.time_since_epoch()).count()
				<< chrono::duration_cast<chrono::seconds>(p.m_lastAttempted.time_since_epoch()).count()
				<< p.m_failedAttempts.load() << (unsigned)p.m_lastDisconnect << p.m_score.load() << p.m_rating.load();
			count++;
		}
	}

	if (auto nodeTable = this->nodeTable())
	{
		auto state = nodeTable->snapshot();
		state.sort();
		for (auto const& entry : state)
		{
			network.appendList(4);
			entry.endpoint.streamRLP(network, NodeIPEndpoint::StreamInline);
			network << entry.id;
			count++;
		}
	}
	// else: TODO: use previous configuration if available

	RLPStream ret(3);
	ret << dev::p2p::c_protocolVersion << m_alias.secret().ref();
	ret.appendList(count);
	if (!!count)
		ret.appendRaw(network.out(), count);
	return ret.out();
}

void FakeHost::restoreNetwork(bytesConstRef _b)
{
	if (!_b.size())
		return;

	// nodes can only be added if network is added
	if (!isStarted())
		BOOST_THROW_EXCEPTION(NetworkStartRequired());

	if (m_dropPeers)
		return;

	RecursiveGuard l(x_sessions);
	RLP r(_b);
	unsigned fileVersion = r[0].toInt<unsigned>();
	if (r.itemCount() > 0 && r[0].isInt() && fileVersion >= dev::p2p::c_protocolVersion - 1)
	{
		// r[0] = version
		// r[1] = key
		// r[2] = nodes

		for (auto i : r[2])
		{
			// todo: ipv6
			if (i[0].itemCount() != 4 && i[0].size() != 4)
				continue;

			if (i.itemCount() == 4 || i.itemCount() == 11)
			{
				Node n((NodeID)i[3], NodeIPEndpoint(i));
				if (i.itemCount() == 4 && n.endpoint.isAllowed())
				{
					addNodeToNodeTable(n);
				}
				else if (i.itemCount() == 11)
				{
					n.peerType = i[4].toInt<bool>() ? PeerType::Required : PeerType::Optional;
					if (!n.endpoint.isAllowed() && n.peerType == PeerType::Optional)
						continue;
					shared_ptr<FakePeer> p = make_shared<FakePeer>(n);
					p->m_lastConnected = chrono::system_clock::time_point(chrono::seconds(i[5].toInt<unsigned>()));
					p->m_lastAttempted = chrono::system_clock::time_point(chrono::seconds(i[6].toInt<unsigned>()));
					p->m_failedAttempts = i[7].toInt<unsigned>();
					p->m_lastDisconnect = (DisconnectReason)i[8].toInt<unsigned>();
					p->m_score = (int)i[9].toInt<unsigned>();
					p->m_rating = (int)i[10].toInt<unsigned>();
					m_peers[p->id] = p;
					if (p->peerType == PeerType::Required)
						requirePeer(p->id, n.endpoint);
					else
						addNodeToNodeTable(*p.get(), NodeTable::NodeRelation::Known);
				}
			}
			else if (i.itemCount() == 3 || i.itemCount() == 10)
			{
				Node n((NodeID)i[2], NodeIPEndpoint(bi::address_v4(i[0].toArray<byte, 4>()), i[1].toInt<uint16_t>(), i[1].toInt<uint16_t>()));
				if (i.itemCount() == 3 && n.endpoint.isAllowed())
					addNodeToNodeTable(n);
				else if (i.itemCount() == 10)
				{
					n.peerType = i[3].toInt<bool>() ? PeerType::Required : PeerType::Optional;
					if (!n.endpoint.isAllowed() && n.peerType == PeerType::Optional)
						continue;
					shared_ptr<FakePeer> p = make_shared<FakePeer>(n);
					p->m_lastConnected = chrono::system_clock::time_point(chrono::seconds(i[4].toInt<unsigned>()));
					p->m_lastAttempted = chrono::system_clock::time_point(chrono::seconds(i[5].toInt<unsigned>()));
					p->m_failedAttempts = i[6].toInt<unsigned>();
					p->m_lastDisconnect = (DisconnectReason)i[7].toInt<unsigned>();
					p->m_score = (int)i[8].toInt<unsigned>();
					p->m_rating = (int)i[9].toInt<unsigned>();
					m_peers[p->id] = p;
					if (p->peerType == PeerType::Required)
						requirePeer(p->id, n.endpoint);
					else
						addNodeToNodeTable(*p.get(), NodeTable::NodeRelation::Known);
				}
			}
		}
	}
}

bool FakeHost::peerSlotsAvailable(FakeHost::PeerSlotType _type)
{
	size_t peerNodeConns = 0;
	{
		Guard l(x_pendingNodeConns);
		peerNodeConns = m_pendingPeerConns.size();
	}
	return peerCount() + peerNodeConns < peerSlots(_type);
}

bool FakeHost::nodeTableHasNode(Public const& _id) const
{
	auto nodeTable = this->nodeTable();
	return nodeTable && nodeTable->haveNode(_id);
}

Node FakeHost::nodeFromNodeTable(Public const& _id) const
{
	auto nodeTable = this->nodeTable();
	return nodeTable ? nodeTable->node(_id) : Node{};
}

bool FakeHost::addNodeToNodeTable(Node const& _node, NodeTable::NodeRelation _relation /* = NodeTable::NodeRelation::Unknown */)
{
	auto nodeTable = this->nodeTable();
	if (!nodeTable)
		return false;

	nodeTable->addNode(_node, _relation);
	return true;
}
