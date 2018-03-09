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
/** @file Peer.h
 * @author Alex Leverington <nessence@gmail.com>
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include "Common.h"

namespace dev
{

namespace p2p
{

/**
 * @brief Representation of connectivity state and all other pertinent Peer metadata.
 * A Peer represents connectivity between two nodes, which in this case, are the host
 * and remote nodes.
 *
 * State information necessary for loading network topology is maintained by NodeTable.
 *
 * @todo Implement 'bool required'
 * @todo reputation: Move score, rating to capability-specific map (&& remove friend class)
 * @todo reputation: implement via origin-tagged events
 * @todo Populate metadata upon construction; save when destroyed.
 * @todo Metadata for peers needs to be handled via a storage backend. 
 * Specifically, peers can be utilized in a variety of
 * many-to-many relationships while also needing to modify shared instances of
 * those peers. Modifying these properties via a storage backend alleviates
 * Host of the responsibility. (&& remove save/restoreNetwork)
 * @todo reimplement recording of historical session information on per-transport basis
 * @todo move attributes into protected
 */
class Peer: public Node
{
	friend class Session;		/// Allows Session to update score and rating.
	friend class Host;		/// For Host: saveNetwork(), restoreNetwork()
	friend class FakeHost;
	friend class RLPXHandshake;

public:
	/// Construct Peer from Node.
	Peer(Node const& _node): Node(_node) {}

	Peer(Peer const&);
	
	bool isOffline() const { return !m_session.lock(); }

	virtual bool operator<(Peer const& _p) const;
	
	/// WIP: Returns current peer rating.
	int rating() const { return m_rating; }
	
	/// Return true if connection attempt should be made to this peer or false if
	bool shouldReconnect() const;
	
	/// Number of times connection has been attempted to peer.
	int failedAttempts() const { return m_failedAttempts; }

	/// Reason peer was previously disconnected.
	DisconnectReason lastDisconnect() const { return m_lastDisconnect; }
	
	/// Peer session is noted as useful.
	void noteSessionGood() { m_failedAttempts = 0; }
	
protected:
	/// Returns number of seconds to wait until attempting connection, based on attempted connection history.
	unsigned fallbackSeconds() const;

	std::atomic<int> m_score{0};									///< All time cumulative.
	std::atomic<int> m_rating{0};									///< Trending.
	
	/// Network Availability
	
	std::chrono::system_clock::time_point m_lastConnected;
	std::chrono::system_clock::time_point m_lastAttempted;
	std::atomic<unsigned> m_failedAttempts{0};
	DisconnectReason m_lastDisconnect = NoDisconnect;	///< Reason for disconnect that happened last.

	/// Used by isOffline() and (todo) for peer to emit session information.
	std::weak_ptr<Session> m_session;
};

//用于记录根据Peer进行排序的信息
class PeerSortObject
{
	public: 
		PeerSortObject(const NodeID& _id , const int _rating, const std::chrono::steady_clock::time_point& _connectTime) 
			:m_id(_id),m_rating(_rating), m_connectTime(_connectTime){}

		PeerSortObject(const PeerSortObject& _obj)
			:m_id(id()),m_rating(_obj.rating()),m_connectTime(_obj.connectTime()){}

		NodeID id() const { return m_id; }
		int rating() const { return m_rating; }
		std::chrono::steady_clock::time_point connectTime() const { return m_connectTime; }

	private:
		NodeID m_id;
		int m_rating;
		std::chrono::steady_clock::time_point m_connectTime;
};

using Peers = std::vector<Peer>;

}
}
