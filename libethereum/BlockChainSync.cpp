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
/** @file BlockChainSync.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "BlockChainSync.h"

#include <chrono>
#include <libdevcore/Common.h>
#include <libdevcore/TrieHash.h>
#include <libp2p/Host.h>
#include <libp2p/Session.h>
#include <libethcore/Exceptions.h>
#include "BlockChain.h"
#include "BlockQueue.h"
#include "EthereumPeer.h"
#include "EthereumHost.h"
#include "BlockChainSyncState.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace p2p;




std::ostream& dev::eth::operator<<(std::ostream& _out, SyncStatus const& _sync)
{
	_out << "protocol: " << _sync.protocolVersion << endl;
	_out << "state: " << EthereumHost::stateName(_sync.state) << " ";
	if (_sync.state == SyncState::Blocks)
		_out << _sync.currentBlockNumber << "/" << _sync.highestBlockNumber;
	return _out;
}



BlockChainSync::BlockChainSync(EthereumHost& _host):
	m_host(_host),
	m_chainStartBlock(_host.chain().chainStartBlockNumber()),
	m_startingBlock(_host.chain().number()),
	m_lastImportedBlock(m_startingBlock),
	m_lastImportedBlockHash(_host.chain().currentHash())
{ 
	m_bqRoomAvailable = host().bq().onRoomAvailable([this]()
	{
		RecursiveGuard l(x_sync);
		ctrace << "onRoomAvaliable state = Blocks";
		m_state = SyncState::Blocks; 
	});

	m_lastImportedBlock = m_startingBlock;
	m_lastImportedBlockHash = _host.chain().currentHash();

	//同步状态机
	m_pNotSyncedState = make_shared<NotSyncedState>(*this);
	m_pIdleSyncState = make_shared<IdleSyncState>(*this);
	m_pWatingSyncState = make_shared<WaitingSyncState>(*this);
	m_pBlockSyncState= make_shared<BlockSyncState>(*this);
	m_pFindingCommonBlockSyncState = make_shared<FindingCommonBlockSyncState>(*this);
	m_pSyncBlocksSyncState = make_shared<SyncBlocksSyncState>(*this);

	//起始时为Idle状态
	m_pCurrState = m_pIdleSyncState; 
	m_state = SyncState::Idle;


}

BlockChainSync::~BlockChainSync()
{
	RecursiveGuard l(x_sync);
	abortSync(); 
}


void BlockChainSync::init(const uint32_t _last_irr_block, const h256& _last_irr_block_hash)
{
	RecursiveGuard l(x_sync);
	m_lastIrreversibleBlock = _last_irr_block;
	m_lastIrreversibleBlockHash = _last_irr_block_hash;
}


void BlockChainSync::onBlockImported(BlockHeader const& _info, const uint32_t _last_irr_block, const h256& _last_irr_block_hash)
{
	//if a block has been added via mining or other block import function
	//through RPC, then we should count it as a last imported block
	RecursiveGuard l(x_sync);   
	m_pCurrState->onBlockImported(_info, _last_irr_block,_last_irr_block_hash); 
}

void BlockChainSync::switchState(SyncState _s)
{    
	if (m_state != _s)
	{
		m_state = _s;
		
		std::shared_ptr<BlockChainSyncState> pNewState;
		switch (_s)
		{
		case SyncState::NotSynced:
			pNewState = m_pNotSyncedState;
			break; 
		case SyncState::Idle:			
			pNewState = m_pIdleSyncState;
			m_lockBlockGen = false;
			break;
		case SyncState::Waiting:
			pNewState = m_pWatingSyncState;
			break;			 
		case SyncState::Blocks:				
			pNewState = m_pBlockSyncState;
			break;
		case SyncState::FindingCommonBlock:
			pNewState = m_pFindingCommonBlockSyncState;
			break;
		case SyncState::SyncBlocks:
			pNewState = m_pSyncBlocksSyncState;
			break;
		default:
			break;
		}

		if (pNewState)
		{
			cwarn << "==============>Switch To " << stateName(_s);
			m_pCurrState->onLeave();
			m_pCurrState = pNewState;
			m_pCurrState->onEnter();
		}
	}
}


void BlockChainSync::abortSync()
{
	RecursiveGuard l(x_sync);
	return;

	resetSync();
	host().foreachPeer([&](std::shared_ptr<EthereumPeer> _p)
	{
		_p->abortSync();
		return true;
	});
}

void BlockChainSync::onPeerStatus(std::shared_ptr<EthereumPeer> _peer)
{
	RecursiveGuard l(x_sync);  
	m_pCurrState->onPeerStatus(_peer); 
	 
}

 

 

 

void BlockChainSync::clearPeerDownload(std::shared_ptr<EthereumPeer> _peer)
{
	auto syncPeer = m_headerSyncPeers.find(_peer);
	if (syncPeer != m_headerSyncPeers.end())
	{
		for (unsigned block : syncPeer->second)
			m_downloadingHeaders.erase(block);
		m_headerSyncPeers.erase(syncPeer);
	}
	syncPeer = m_bodySyncPeers.find(_peer);
	if (syncPeer != m_bodySyncPeers.end())
	{
		for (unsigned block : syncPeer->second)
			m_downloadingBodies.erase(block);
		m_bodySyncPeers.erase(syncPeer);
	} 
}

void BlockChainSync::clearPeerDownload()
{
	for (auto s = m_headerSyncPeers.begin(); s != m_headerSyncPeers.end();)
	{
		if (s->first.expired())
		{
			for (unsigned block : s->second)
				m_downloadingHeaders.erase(block);
			m_headerSyncPeers.erase(s++);
		}
		else
			++s;
	}
	for (auto s = m_bodySyncPeers.begin(); s != m_bodySyncPeers.end();)
	{
		if (s->first.expired())
		{
			for (unsigned block : s->second)
				m_downloadingBodies.erase(block);
			m_bodySyncPeers.erase(s++);
		}
		else
			++s;
	} 
}

void BlockChainSync::logNewBlock(h256 const& _h)
{
	m_knownNewHashes.erase(_h);
}

void BlockChainSync::onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
{
	RecursiveGuard l(x_sync);  
	m_pCurrState->onPeerBlockHeaders(_peer, _r);  
}



void BlockChainSync::onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
{
	RecursiveGuard l(x_sync); 
	m_pCurrState->onPeerBlockBodies(_peer, _r); 
}

void BlockChainSync::collectBlocks()
{ 
}

void BlockChainSync::onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
{
	RecursiveGuard l(x_sync); 
	m_pCurrState->onPeerNewBlock(_peer, _r); 
}

SyncStatus BlockChainSync::status() const
{
	RecursiveGuard l(x_sync);
	SyncStatus res;
	res.state = m_state;
	res.protocolVersion = 62;
	res.startBlockNumber = m_startingBlock;
	res.currentBlockNumber = host().chain().number();
	res.highestBlockNumber = m_highestBlock;
	return res;
}

void BlockChainSync::resetSync()
{
	m_downloadingHeaders.clear();
	m_downloadingBodies.clear();
	m_headers.clear();
	m_bodies.clear();
	m_headerSyncPeers.clear();
	m_bodySyncPeers.clear();
	m_headerIdToNumber.clear(); 
	m_state = SyncState::NotSynced;
}



void BlockChainSync::back2LastIrrBlockAndResync()
{ 
}

void BlockChainSync::keeySyncAlive()
{
 
} 

void BlockChainSync::restartSync()
{
	RecursiveGuard l(x_sync);
	resetSync();
	m_highestBlock = 0; 
	ctrace << "restartSync => m_haveCommonHeader = false";
	m_haveCommonHeader = false;
	host().bq().clear();
	m_startingBlock = host().chain().number();
	m_lastImportedBlock = m_startingBlock;
	m_lastImportedBlockHash = host().chain().currentHash();
}

void BlockChainSync::completeSync()
{
	ctrace << "completeSync!!!";
	RecursiveGuard l(x_sync);
	resetSync();
	m_state = SyncState::Idle;
}

void BlockChainSync::pauseSync()
{
	m_state = SyncState::Waiting;
}

bool BlockChainSync::isSyncing() const
{
	return m_state != SyncState::Idle;
}

void BlockChainSync::onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes)
{
	//目前忽略NewHash
	ctrace << "Ignore new blockhash!!!";
	return;

	RecursiveGuard l(x_sync);
	m_pCurrState->onPeerNewHashes(_peer, _hashes);
}

void BlockChainSync::onPeerAborting()
{
	RecursiveGuard l(x_sync);
	//// Can't check invariants here since the peers is already removed from the list and the state is not updated yet.
	//clearPeerDownload();
	//continueSync();
	//DEV_INVARIANT_CHECK_HERE;
}

bool BlockChainSync::invariants() const
{
	return true;
	if (!isSyncing() && !m_headers.empty())
		BOOST_THROW_EXCEPTION(FailedInvariant() << errinfo_comment("Got headers while not syncing")); 
	if (!isSyncing() && !m_bodies.empty())
		BOOST_THROW_EXCEPTION(FailedInvariant() << errinfo_comment("Got bodies while not syncing"));
	//创世块可以看作含有Common
	//if (isSyncing() && m_host.chain().number() > 0 && m_haveCommonHeader && m_lastImportedBlock == 0)
	//	BOOST_THROW_EXCEPTION(FailedInvariant() << errinfo_comment("Common block not found"));
	if (isSyncing() && !m_headers.empty() &&  m_lastImportedBlock >= m_headers.begin()->first)
		BOOST_THROW_EXCEPTION(FailedInvariant() << errinfo_comment("Header is too old"));
	if (m_headerSyncPeers.empty() != m_downloadingHeaders.empty())
		BOOST_THROW_EXCEPTION(FailedInvariant() << errinfo_comment("Header download map mismatch"));
	if (m_bodySyncPeers.empty() != m_downloadingBodies.empty() && m_downloadingBodies.size() <= m_headerIdToNumber.size())
		BOOST_THROW_EXCEPTION(FailedInvariant() << errinfo_comment("Body download map mismatch"));
	return true;
}

char const* dev::eth::BlockChainSync::stateName(SyncState _s)
{
	return EthereumHost::stateName(_s);
}

bool BlockChainSync::isBlockGenLocked() const
{
	return m_lockBlockGen;
}

