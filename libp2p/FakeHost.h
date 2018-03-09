#pragma once

#include<libp2p/Host.h>



namespace dev
{
namespace p2p
{

class FakeHost :public Host
{
	friend class HostNodeTableHandler;
	friend class RLPXHandshake;

	friend class Session;
	friend class HostCapabilityFace;

public:
	/// Start server, listening for connections on the given port.
	FakeHost(
		std::string const& _clientVersion,
		NetworkPreferences const& _n = NetworkPreferences(),
		bytesConstRef _restoreNetwork = bytesConstRef()
	);

	/// Alternative constructor that allows providing the node key directly
	/// without restoring the network.
	FakeHost(
		std::string const& _clientVersion,
		KeyPair const& _alias,
		NetworkPreferences const& _n = NetworkPreferences()
	);

	/// Will block on network process events.
	virtual ~FakeHost();

	/// Register a peer-capability; all new peer connections will have this capability.
	template <class T> std::shared_ptr<T> registerCapability(std::shared_ptr<T> const& _t) { _t->m_host = this; m_capabilities[std::make_pair(T::staticName(), T::staticVersion())] = _t; return _t; }
	template <class T> void addCapability(std::shared_ptr<T> const & _p, std::string const& _name, u256 const& _version) { m_capabilities[std::make_pair(_name, _version)] = _p; }

	bool haveCapability(CapDesc const& _name) const { return m_capabilities.count(_name) != 0; }
	CapDescs caps() const { CapDescs ret; for (auto const& i : m_capabilities) ret.push_back(i.first); return ret; }
	template <class T> std::shared_ptr<T> cap() const { try { return std::static_pointer_cast<T>(m_capabilities.at(std::make_pair(T::staticName(), T::staticVersion()))); } catch (...) { return nullptr; } }

	/// Add a potential peer.
	void addPeer(NodeSpec const& _s, PeerType _t);

	/// Add node as a peer candidate. Node is added if discovery ping is successful and table has capacity.
	void addNode(NodeID const& _node, NodeIPEndpoint const& _endpoint);

	/// Create Peer and attempt keeping peer connected.
	void requirePeer(NodeID const& _node, NodeIPEndpoint const& _endpoint);

	/// Create Peer and attempt keeping peer connected.
	void requirePeer(NodeID const& _node, bi::address const& _addr, unsigned short _udpPort, unsigned short _tcpPort) { requirePeer(_node, NodeIPEndpoint(_addr, _udpPort, _tcpPort)); }

	/// Note peer as no longer being required.
	void relinquishPeer(NodeID const& _node);

	/// Set ideal number of peers.
	void setIdealPeerCount(unsigned _n) { m_idealPeerCount = _n; }

	/// Set multipier for max accepted connections.
	void setPeerStretch(unsigned _n) { m_stretchPeers = _n; }

	/// Get peer information.
	PeerSessionInfos peerSessionInfo() const;

	/// Get number of peers connected.
	size_t peerCount() const;

	/// Get the address we're listening on currently.
	std::string listenAddress() const { return m_tcpPublic.address().is_unspecified() ? "0.0.0.0" : m_tcpPublic.address().to_string(); }

	/// Get the port we're listening on currently.
	unsigned short listenPort() const { return std::max(0, m_listenPort.load()); }

	/// Serialise the set of known peers.
	bytes saveNetwork() const;

	// TODO: P2P this should be combined with peers into a HostStat object of some kind; coalesce data, as it's only used for status information.
	Peers getPeers() const { RecursiveGuard l(x_sessions); Peers ret; for (auto const& i : m_peers) ret.push_back(*i.second); return ret; }

	NetworkPreferences const& networkPreferences() const { return m_netPrefs; }

	void setNetworkPreferences(NetworkPreferences const& _p, bool _dropPeers = false) { m_dropPeers = _dropPeers; auto had = isStarted(); if (had) stop(); m_netPrefs = _p; if (had) start(); }

	/// Start network. @threadsafe
	void start();

	/// Stop network. @threadsafe
	/// Resets acceptor, socket, and IO service. Called by deallocator.
	void stop();

	/// @returns if network has been started.
	bool isStarted() const { return isWorking(); }

	/// @returns our reputation manager.
	ReputationManager& repMan() { return m_repMan; }

	/// @returns if network is started and interactive.
	bool haveNetwork() const { Guard l(x_runTimer); Guard ll(x_nodeTable); return m_run && !!m_nodeTable; }

	/// Validates and starts peer session, taking ownership of _io. Disconnects and returns false upon error.
	void startPeerSession(Public const& _id, RLP const& _hello, std::unique_ptr<RLPXFrameCoder>&& _io, std::shared_ptr<RLPXSocket> const& _s);

	/// Get session by id
	std::shared_ptr<SessionFace> peerSession(NodeID const& _id) { RecursiveGuard l(x_sessions); return m_sessions.count(_id) ? m_sessions[_id].lock() : std::shared_ptr<SessionFace>(); }

	/// Get our current node ID.
	NodeID id() const { return m_alias.pub(); }

	/// Get the public TCP endpoint.
	bi::tcp::endpoint const& tcpPublic() const { return m_tcpPublic; }

	/// Get the public endpoint information.
	std::string enode() const { return "enode://" + id().hex() + "@" + (networkPreferences().publicIPAddress.empty() ? m_tcpPublic.address().to_string() : networkPreferences().publicIPAddress) + ":" + toString(m_tcpPublic.port()); }

	/// Get the node information.
	p2p::NodeInfo nodeInfo() const { return NodeInfo(id(), (networkPreferences().publicIPAddress.empty() ? m_tcpPublic.address().to_string() : networkPreferences().publicIPAddress), m_tcpPublic.port(), m_clientVersion); }

protected:
	void onNodeTableEvent(NodeID const& _n, NodeTableEventType const& _e);

	/// Deserialise the data and populate the set of known peers.
	void restoreNetwork(bytesConstRef _b);

private:
	enum PeerSlotType { Egress, Ingress };

	unsigned peerSlots(PeerSlotType _type) { return _type == Egress ? m_idealPeerCount : m_idealPeerCount * m_stretchPeers; }

	virtual bool havePeerSession(NodeID const& _id) { return !!peerSession(_id); }

	/// Determines and sets m_tcpPublic to publicly advertised address.
	virtual void determinePublic();

	virtual void connect(std::shared_ptr<Peer> const& _p);

	/// Returns true if pending and connected peer count is less than maximum
	virtual bool peerSlotsAvailable(PeerSlotType _type = Ingress);

	/// Ping the peers to update the latency information and disconnect peers which have timed out.
	virtual void keepAlivePeers();

	/// Disconnect peers which didn't respond to keepAlivePeers ping prior to c_keepAliveTimeOut.
	virtual void disconnectLatePeers();

	/// Called only from startedWorking().
	virtual void runAcceptor();

	/// Called by Worker. Not thread-safe; to be called only by worker.
	virtual void startedWorking();
	/// Called by startedWorking. Not thread-safe; to be called only be Worker.
	virtual void run(boost::system::error_code const& error);			///< Run network. Called serially via ASIO deadline timer. Manages connection state transitions.

																/// Run network. Not thread-safe; to be called only by worker.
	virtual void doWork();

	/// Shutdown network. Not thread-safe; to be called only by worker.
	virtual void doneWorking();

	/// returns true if a member of m_requiredPeers
	virtual bool isRequiredPeer(NodeID const&) const;

	virtual bool nodeTableHasNode(Public const& _id) const;
	virtual Node nodeFromNodeTable(Public const& _id) const;
	virtual bool addNodeToNodeTable(Node const& _node, NodeTable::NodeRelation _relation = NodeTable::NodeRelation::Unknown);
	///< Set by constructor and used to set Host key and restore network peers & nodes.

};
}
}




