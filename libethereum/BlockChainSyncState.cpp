#include "BlockChainSyncState.h"
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
#include "libdevcore/Log.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace p2p;


unsigned const c_maxPeerUknownNewBlocks = 1024; /// Max number of unknown new blocks peer can give us
unsigned const c_maxRequestHeaders = 1024;
unsigned const c_maxRequestBodies = 1024;
unsigned const c_blockQueueGap = 1000;

namespace  // Helper functions.
{

	template<typename T> bool haveItem(std::map<unsigned, T>& _container, unsigned _number)
	{
		if (_container.empty())
			return false;
		auto lower = _container.lower_bound(_number);
		if (lower != _container.end() && lower->first == _number)
			return true;
		if (lower == _container.begin())
			return false;
		--lower;
		return lower->first <= _number && (lower->first + lower->second.size()) > _number;
	}

	template<typename T> T const* findItem(std::map<unsigned, std::vector<T>>& _container, unsigned _number)
	{
		if (_container.empty())
			return nullptr;
		auto lower = _container.lower_bound(_number);
		if (lower != _container.end() && lower->first == _number)
			return &(*lower->second.begin());
		if (lower == _container.begin())
			return nullptr;
		--lower;
		if (lower->first <= _number && (lower->first + lower->second.size()) > _number)
			return &lower->second.at(_number - lower->first);
		return nullptr;
	}

	template<typename T> void removeItem(std::map<unsigned, std::vector<T>>& _container, unsigned _number)
	{
		if (_container.empty())
			return;
		auto lower = _container.lower_bound(_number);
		if (lower != _container.end() && lower->first == _number)
		{
			_container.erase(lower);
			return;
		}
		if (lower == _container.begin())
			return;
		--lower;
		if (lower->first <= _number && (lower->first + lower->second.size()) > _number)
			lower->second.erase(lower->second.begin() + (_number - lower->first), lower->second.end());
	}

	template<typename T> void removeAllStartingWith(std::map<unsigned, std::vector<T>>& _container, unsigned _number)
	{
		if (_container.empty())
			return;
		auto lower = _container.lower_bound(_number);
		if (lower != _container.end() && lower->first == _number)
		{
			_container.erase(lower, _container.end());
			return;
		}
		if (lower == _container.begin())
		{
			_container.clear();
			return;
		}
		--lower;
		if (lower->first <= _number && (lower->first + lower->second.size()) > _number)
			lower->second.erase(lower->second.begin() + (_number - lower->first), lower->second.end());
		_container.erase(++lower, _container.end());
	}

	template<typename T> void mergeInto(std::map<unsigned, std::vector<T>>& _container, unsigned _number, T&& _data)
	{
		assert(!haveItem(_container, _number));
		auto lower = _container.lower_bound(_number);
		if (!_container.empty() && lower != _container.begin())
			--lower;
		if (lower != _container.end() && (lower->first + lower->second.size() == _number))
		{
			// extend existing chunk
			lower->second.emplace_back(_data);

			auto next = lower;
			++next;
			if (next != _container.end() && (lower->first + lower->second.size() == next->first))
			{
				// merge with the next chunk
				std::move(next->second.begin(), next->second.end(), std::back_inserter(lower->second));
				_container.erase(next);
			}

		}
		else
		{
			// insert a new chunk
			auto inserted = _container.insert(lower, std::make_pair(_number, std::vector<T> { _data }));
			auto next = inserted;
			++next;
			if (next != _container.end() && next->first == _number + 1)
			{
				std::move(next->second.begin(), next->second.end(), std::back_inserter(inserted->second));
				_container.erase(next);
			}
		}
	}

}  // Anonymous namespace -- helper functions.

namespace dev
{ 
	namespace eth
	{ 
		BlockChainSyncState::BlockChainSyncState(BlockChainSync& _sync):m_sync(_sync){}
		 
		void IdleSyncState::onPeerStatus(std::shared_ptr<EthereumPeer> _peer)
		{
			updateTimeout();

			if (peerLegalCheck(_peer))
			{
				if (_peer->getLastIrrBlock() < m_sync.m_lastIrreversibleBlock)
				{
					cwarn << "peer:" << _peer->id() << "|" << _peer->getLastIrrBlock() << " < " << "m_lastIrreversibleBlock = " << m_sync.m_lastIrreversibleBlock;
					return;
				}

				if (host().bq().knownCount() > 10)
				{
					return;
				}

				auto status = bq().blockStatus(_peer->m_latestHash);
				if (status == QueueStatus::Unknown)
				{//当peer的laststHash为未知时，采取向其request块头  


					//将当前链最后块作为导入起始块
					m_sync.m_syncStartBlock = m_sync.m_lastImportedBlock;
					m_sync.m_syncStartBlockHash = m_sync.m_lastImportedBlockHash;
					m_sync.m_syncLastIrrBlock = m_sync.m_lastIrreversibleBlock;

					m_sync.m_expectBlockHashForFindingCommon = _peer->m_latestHash;
					m_sync.m_expectBlockForFindingCommon = 0;

					//开启不中断同步
					switchState(SyncState::FindingCommonBlock);

					requestPeerLatestBlockHeader(_peer); 

				}
			}
		}

		 

		void IdleSyncState::onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{ 
			updateTimeout();

			if (_r.itemCount() != 3)
			{
				_peer->disable("NewBlock without 2 data fields.");
				return;
			}

			BlockHeader info(_r[0][0].data(), HeaderData);
			auto h = info.hash();


			cwarn << "onPeerNewBlock ==>" << h;

			//从数据包中获取此peer的不可逆转块号
			uint32_t lastIrrBlock = _r[1].toInt<u256>().convert_to<uint32_t>();
			h256	lastIrrBlockHash = _r[2].toHash<h256>();

			cwarn << "LastIrr = " << lastIrrBlock;

			//更新peer最新不可逆转块号
			_peer->setLastIrrBlock(lastIrrBlock);
			_peer->setLastIrrBlockHash(lastIrrBlockHash);

			//更新Peer最新的Hash
			_peer->m_latestHash = h;

			if (lastIrrBlock < m_sync.m_lastIrreversibleBlock)
			{//拒绝接收不可逆转小于当前不可逆转块的客户端

				cwarn << _peer->id() << "|" << lastIrrBlock << " < " << "m_lastIrreversibleBlock = " << m_sync.m_lastIrreversibleBlock << "ignore new block!!!!";
				_peer->addRating(-10000);
				return;
			}

			//只处理BlockQueue未知的块
			auto status = host().bq().blockStatus(h);
			if (status != QueueStatus::Unknown)
			{
				return;
			}

			_peer->tryInsertPeerKnownBlockList(h); 

			unsigned blockNumber = static_cast<unsigned>(info.number());
			if (blockNumber > (m_sync.m_lastImportedBlock + 1))
			{
				cwarn << "Received unknown new block";

				if (host().bq().knownCount() < 10)
				{ 
					//将当前链最后块作为导入起始块
					m_sync.m_syncStartBlock = m_sync.m_lastImportedBlock;
					m_sync.m_syncStartBlockHash = m_sync.m_lastImportedBlockHash;
					m_sync.m_syncLastIrrBlock = m_sync.m_lastIrreversibleBlock;

					m_sync.m_expectBlockHashForFindingCommon = _peer->m_latestHash;
					m_sync.m_expectBlockForFindingCommon = 0;
					requestPeerLatestBlockHeader(_peer);

					//尝试查找CommonBlock
					switchState(SyncState::FindingCommonBlock);
				}
				return;
			}
			else if (
				blockNumber < m_sync.m_lastImportedBlock &&
				lastIrrBlock > m_sync.m_lastIrreversibleBlock
				)
			{//当前链不可逆有问题
				cwarn << "blockNumber < m_lastImportedBlock" << blockNumber << " < " << m_sync.m_lastImportedBlock;
				cwarn << "lastIrrBlock > m_lastIrreversibleBlock" << lastIrrBlock << " > " << m_sync.m_lastIrreversibleBlock;
				cwarn << "back2LastIrrBlockAndResync";
				
				m_sync.m_syncStartBlock = m_sync.m_lastIrreversibleBlock;
				m_sync.m_syncStartBlockHash = host().chain().numberHash(m_sync.m_syncStartBlock);
				m_sync.m_syncLastIrrBlock = m_sync.m_lastIrreversibleBlock;

				m_sync.m_expectBlockHashForFindingCommon = _peer->m_latestHash;
				m_sync.m_expectBlockForFindingCommon = 0;
				requestPeerLatestBlockHeader(_peer);

				//此种情况下阻塞产块进程
				m_sync.m_lockBlockGen = true;
				cwarn << "Block Gen Locked!!!!!";
				//尝试查找CommonBlock
				switchState(SyncState::FindingCommonBlock);
				return;
			}


			switch (host().bq().import(_r[0].data()))
			{
				case ImportResult::Success:
					_peer->addRating(100);  

					if (blockNumber > m_sync.m_lastImportedBlock)
					{
						m_sync.m_lastImportedBlock = max(m_sync.m_lastImportedBlock, blockNumber);
						m_sync.m_lastImportedBlockHash = h;
					}

					//接到传来的块，做简单的检验则直接广播出去
					host().pushDeliverBlock(h, _r[0].data().toBytes(), lastIrrBlock, lastIrrBlockHash);
					break;
				case ImportResult::FutureTimeKnown:
				 
					//接到传来的块，做简单的检验则直接广播出去
					host().pushDeliverBlock(h, _r[0].data().toBytes(), lastIrrBlock, lastIrrBlockHash);
					break;

				case ImportResult::Malformed:
				case ImportResult::BadChain: 
					_peer->disable("Malformed block received.");
					return;

				case ImportResult::AlreadyInChain:
				case ImportResult::AlreadyKnown:
					break;

				case ImportResult::FutureTimeUnknown:
				case ImportResult::UnknownParent:
				{
					_peer->m_unknownNewBlocks++;
					if (_peer->m_unknownNewBlocks > c_maxPeerUknownNewBlocks)
					{
						_peer->disable("Too many uknown new blocks");
						resetAllSyncData();
					}
					//将当前链最后块作为导入起始块
					m_sync.m_syncStartBlock = m_sync.m_lastImportedBlock;
					m_sync.m_syncStartBlockHash = m_sync.m_lastImportedBlockHash;
					m_sync.m_syncLastIrrBlock = m_sync.m_lastIrreversibleBlock;

					m_sync.m_expectBlockHashForFindingCommon = _peer->m_latestHash;
					m_sync.m_expectBlockForFindingCommon = 0;
					requestPeerLatestBlockHeader(_peer);

					//尝试查找CommonBlock
					switchState(SyncState::FindingCommonBlock);
					break;
				}
				case ImportResult::Irreversible: //遇到了未知的不可逆转块，说明某客户端与当前客户端链严重偏离
					cwarn << "Unknown irreversible block founded!!! ignore and restart sync!";
					_peer->addRating(-10000);
					_peer->disable("Unknown irreversible block founded!!!");
					resetAllSyncData(); 
					break;
				default:;
			}
		}

		void IdleSyncState::onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes)
		{
			updateTimeout();

			if (_peer->isConversing())
			{
				clog(NetMessageDetail) << "Ignoring new hashes since we're already downloading.";
				return;
			}
			clog(NetMessageDetail) << "Not syncing and new block hash discovered: syncing.";
			unsigned knowns = 0;
			unsigned unknowns = 0;
			unsigned maxHeight = 0;

			for (auto const& p : _hashes)
			{
				h256 const& h = p.first;
				_peer->addRating(1);
				_peer->tryInsertPeerKnownBlockList(h);
				auto status = host().bq().blockStatus(h);
				if (status == QueueStatus::Importing || status == QueueStatus::Ready || host().chain().isKnown(h)) {
					cwarn << "---" << (unsigned)p.second << ":" << h << " known";
					knowns++;
				}
				else if (status == QueueStatus::Bad)
				{
					cwarn << "block hash bad!" << h << ". Bailing...";
					return;
				}
				else if (status == QueueStatus::Future) { //此处若为Future，则认为已知
					cwarn << "have same future!";
					knowns++;
				}
				else if (status == QueueStatus::Unknown)
				{
					cwarn << "---" << (unsigned)p.second << ":" << h << " unknown";
					unknowns++;
					if (p.second > maxHeight)
					{
						maxHeight = (unsigned)p.second;
						_peer->m_latestHash = h;
					}
				}
				else
					knowns++;
			}
			clog(NetMessageSummary) << knowns << "knowns," << unknowns << "unknowns";
			if (unknowns > 0)
			{
				cwarn << "Not syncing and new block hash discovered: syncing.";


				if ( host().bq().knownCount() < 10 )
				{ 
					//将当前链最后块作为导入起始块
					m_sync.m_syncStartBlock = m_sync.m_lastImportedBlock;
					m_sync.m_syncStartBlockHash = m_sync.m_lastImportedBlockHash;
					m_sync.m_syncLastIrrBlock = m_sync.m_lastIrreversibleBlock;

					m_sync.m_expectBlockHashForFindingCommon = _peer->m_latestHash;
					m_sync.m_expectBlockForFindingCommon = 0;
					requestPeerLatestBlockHeader(_peer);

					//开启不中断同步
					switchState(SyncState::FindingCommonBlock);
				}
			}
		}
		 
 
		 



		EthereumHost& DefaultSyncState::host()
		{
			return m_sync.host();
		}

		EthereumHost const& DefaultSyncState::host() const
		{
			return m_sync.host();
		}

		dev::eth::BlockQueue& DefaultSyncState::bq()
		{
			return m_sync.host().bq();
		}

		dev::eth::BlockQueue const& DefaultSyncState::bq() const
		{
			return m_sync.host().bq();
		}

		bool DefaultSyncState::haveBlockHeader(uint32_t _num)
		{
			return haveItem(m_sync.m_headers, _num);
		}

		void DefaultSyncState::switchState(SyncState _s)
		{
			m_sync.switchState(_s);
		}

		void DefaultSyncState::clearPeerDownloadMarks(std::shared_ptr<EthereumPeer> _peer)
		{
			m_sync.clearPeerDownload(_peer);
		}

		bool DefaultSyncState::peerLegalCheck(std::shared_ptr<EthereumPeer> _peer)
		{
			std::shared_ptr<SessionFace> session = _peer->session();
			if (!session)
				return false; // Expired 

			_peer->setLlegal(false);

			if (_peer->m_genesisHash != host().chain().genesisHash())
				_peer->disable("Invalid genesis hash");
			else if (_peer->m_protocolVersion != host().protocolVersion() && _peer->m_protocolVersion != EthereumHost::c_oldProtocolVersion)
				_peer->disable("Invalid protocol version.");
			else if (_peer->m_networkId != host().networkId())
				_peer->disable("Invalid network identifier.");
			else if (session->info().clientVersion.find("/v0.7.0/") != string::npos)
				_peer->disable("Blacklisted client version.");
			else if (host().isBanned(session->id()))
				_peer->disable("Peer banned for previous bad behaviour.");
			else if (_peer->m_asking != Asking::State && _peer->m_asking != Asking::Nothing)
				_peer->disable("Peer banned for unexpected status message.");
			else
			{
				if (_peer->m_lastIrrBlock <= m_sync.m_lastIrreversibleBlock)
				{
					if( _peer->m_lastIrrBlockHash != host().chain().numberHash(_peer->m_lastIrrBlock) )
					{//发现此Peer运行的链与当前链不同，直接断开链接
						cwarn<< _peer->id()<<"|" << "Peer Last Irr Block Illegal!!!";
						_peer->disable("Peer Last Irr Block Illegal!!!");
						return false;
					}
				}

				_peer->setLlegal(true);
				return true;
			}

			return false;
		}

		bool DefaultSyncState::checkSyncComplete() const
		{ 
			if (m_sync.m_headers.empty())
			{
				assert(m_sync.m_bodies.empty()); 
				return true;
			}
			return false; 
		}

		void DefaultSyncState::resetSyncTempData()
		{
			m_sync.m_downloadingHeaders.clear();
			m_sync.m_downloadingBodies.clear();
			m_sync.m_headers.clear();
			m_sync.m_bodies.clear();
			m_sync.m_headerSyncPeers.clear();
			m_sync.m_bodySyncPeers.clear();
			m_sync.m_headerIdToNumber.clear();
		}


		void DefaultSyncState::resetAllSyncData()
		{
			resetSyncTempData();
			bq().clear(); 
		}

		void DefaultSyncState::printBlockHeadersInfo(RLP const& _r)
		{
			//获取header数量
			size_t itemCount = _r.itemCount();

			h256s itemHashes;
			vector<unsigned> itemNums;
			for (unsigned i = 0; i < itemCount; i++)
			{
				BlockHeader info(_r[i].data(), HeaderData);
				itemNums.push_back(static_cast<unsigned>(info.number()));
				itemHashes.push_back(info.hash());
			}

			if (itemNums.size() > 0)
			{
				cwarn << "Header Nums:" << itemNums;
				cwarn << "Header Hashes:" << itemHashes;
			}
		}

		void DefaultSyncState::requestPeerLatestBlockHeader(std::shared_ptr<EthereumPeer> _peer)
		{
			if (_peer->m_asking != Asking::Nothing)
			{
				clog(NetAllDetail) << "Can't sync with this peer - outstanding asks.";
				return;
			}

			//拒绝已被判定为非法的Peer
			if (!_peer->isLlegal())
				return; 

			_peer->requestBlockHeaders(_peer->m_latestHash, 1, 0, false);
			_peer->m_requireTransactions = true; 
 
		}
		 

		void DefaultSyncState::onPeerStatus(std::shared_ptr<EthereumPeer> _peer)
		{
			//一般情况下只对Peer做合法性检查
			peerLegalCheck(_peer); 

			updateTimeout();

		}

		void DefaultSyncState::onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{
			updateTimeout();
		}

		void DefaultSyncState::onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{
			updateTimeout();
		}

		void DefaultSyncState::onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{ 
			updateTimeout();

			if (_r.itemCount() != 3)
			{
				_peer->disable("NewBlock without 2 data fields.");
				return;
			}

			BlockHeader header(_r[0][0].data(), HeaderData);
			auto h = header.hash();  
			cwarn << "onPeerNewBlock ==>" << h; 
			//从数据包中获取此peer的不可逆转块号
			uint32_t lastIrrBlock = _r[1].toInt<u256>().convert_to<uint32_t>();
			h256 lastIrrBlockHash = _r[2].toHash<h256>();

			cwarn << "LastIrr = " << lastIrrBlock;  
			//更新peer最新不可逆转块号
			_peer->setLastIrrBlock(lastIrrBlock);
			_peer->setLastIrrBlockHash(lastIrrBlockHash);

			//更新Peer最新的Hash
			_peer->m_latestHash = h;

			if (lastIrrBlock < m_sync.m_syncLastIrrBlock)
			{//拒绝接收不可逆转小于当前客户端的块

				cwarn << _peer->id() << "|" << lastIrrBlock << " < " << "m_lastIrreversibleBlock = " << m_sync.m_lastIrreversibleBlock << "ignore new block!!!!";
				_peer->addRating(-10000); 
				return;
			}

			//只处理BlockQueue未知的块
			auto status = host().bq().blockStatus(h);
			if (status != QueueStatus::Unknown)
			{
				return;
			}

			_peer->tryInsertPeerKnownBlockList(h);  
			unsigned blockNumber = static_cast<unsigned>(header.number()); 
			if (blockNumber <= m_sync.m_syncLastIrrBlock)
			{//只导入块号大于不可逆的块
				return;
			}

			switch (host().bq().import(_r[0].data()))
			{
				case ImportResult::Success:
					_peer->addRating(100); 
					//接到传来的块，做简单的检验则直接广播出去
					host().pushDeliverBlock(h, _r[0].data().toBytes(), lastIrrBlock, lastIrrBlockHash);
					break;
				case ImportResult::FutureTimeKnown: 
					//接到传来的块，做简单的检验则直接广播出去
					host().pushDeliverBlock(h, _r[0].data().toBytes(), lastIrrBlock, lastIrrBlockHash);
					break;

				case ImportResult::Malformed:
				case ImportResult::BadChain: 
					_peer->disable("Malformed block received.");
					return;

				case ImportResult::AlreadyInChain:
				case ImportResult::AlreadyKnown:
					break;

				case ImportResult::FutureTimeUnknown:
				case ImportResult::UnknownParent: 
					break; 
				case ImportResult::Irreversible: //遇到了未知的不可逆转块，说明某客户端与当前客户端链严重偏离
					cwarn << "Unknown irreversible block founded!!!"; 
					_peer->addRating(-10000);
					_peer->disable("Unknown irreversible block founded!!!");
					break;
				default:;
			}
		}

		void DefaultSyncState::onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes)
		{
			updateTimeout();
		}

		void DefaultSyncState::onPeerAborting()
		{
			updateTimeout();
		}

		void DefaultSyncState::onBlockImported(BlockHeader const& _info, const uint32_t _last_irr_block, const h256& _last_irr_block_hash)
		{
			if (_info.number() > m_sync.m_lastImportedBlock)
			{
				m_sync.m_lastImportedBlock = static_cast<unsigned>(_info.number());
				m_sync.m_lastImportedBlockHash = _info.hash();
				m_sync.m_highestBlock = max(m_sync.m_lastImportedBlock, m_sync.m_highestBlock);
			}

			if (_last_irr_block > m_sync.m_lastIrreversibleBlock)
			{
				m_sync.m_lastIrreversibleBlock = _last_irr_block;
				m_sync.m_lastImportedBlockHash = _last_irr_block_hash;
			}
		}  

		void SyncBlocksSyncState::onPeerStatus(std::shared_ptr<EthereumPeer> _peer)
		{
			DefaultSyncState::onPeerStatus(_peer);
			keepAlive();
		}

		void SyncBlocksSyncState::onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{
			m_lastBlockMsgTimePoint = fc::time_point::now();

			if (updateTimeout())
			{
				return;
			}

			cwarn << "onPeerBlockHeaders==============================";
			//获取header数量
			size_t itemCount = _r.itemCount();
			cwarn << "BlocksHeaders (" << dec << itemCount << "entries)" << (itemCount ? "" : ": NoMoreHeaders");


			clearPeerDownloadMarks(_peer); 
			 
			if (itemCount == 0)
			{
				cwarn << "Peer does not have the blocks requested !";
				_peer->addRating(-100);
			}

			printBlockHeadersInfo(_r);

			for (unsigned i = 0; i < itemCount; i++)
			{//对于每一个header

				BlockHeader header(_r[i].data(), HeaderData);
				unsigned blockNumber = static_cast<unsigned>(header.number());

				if (blockNumber <= m_sync.m_syncStartBlock)
				{
					clog(NetMessageSummary) << "Skipping header : blockNumber <= m_syncStartBlock " << blockNumber;
					continue;
				}

				if (blockNumber < m_sync.m_syncLastIrrBlock)
				{
					clog(NetMessageSummary) << "Skipping too old header " << blockNumber;
					continue;
				}

				if (haveItem(m_sync.m_headers, blockNumber))
				{//header已存在
					clog(NetMessageSummary) << "Skipping header : already haveItem" << blockNumber;
					continue;
				}  
					
				if (!checkHeader(header))
				{//前驱header出问题，切换回Idle模式
					switchState(SyncState::Idle); 
					return;
				}
				 
				BlockChainSync::Header hdr{ _r[i].data().toBytes(), header.hash(), header.parentHash() };
				BlockChainSync::HeaderId headerId{ header.transactionsRoot(), header.sha3Uncles() };

				mergeInto(m_sync.m_headers, blockNumber, std::move(hdr));
				if (headerId.transactionsRoot == EmptyTrie && headerId.uncles == EmptyListSHA3)
				{//空交易体，则直接制造一个空块体即可
					RLPStream r(2);
					r.appendRaw(RLPEmptyList);
					r.appendRaw(RLPEmptyList);
					bytes body;
					r.swapOut(body);
					mergeInto(m_sync.m_bodies, blockNumber, std::move(body));
				}
				else
					m_sync.m_headerIdToNumber[headerId] = blockNumber; 
				 
			}//end of for 
			

			if (checkSyncComplete())
			{
				switchState(SyncState::Idle);
				return;
			}

			if (!collectBlocks())
			{//拼接导入块时出现错误，重新同步
				switchState(SyncState::Idle);
				return;
			}

			if (checkSyncComplete())
			{
				switchState(SyncState::Idle);
				return;
			}

			//继续同步
			continueSync();
		}

		void SyncBlocksSyncState::syncHeadersAndBodies()
		{
			if (host().bq().knownCount() > c_blockQueueGap)
			{
				clog(NetAllDetail) << "host().bq().knownCount() > c_blockQueueGap Waiting for block queue before downloading blocks";
				resetSyncTempData();
				switchState(SyncState::Idle);
				return;
			}

			//按最新不可逆高度排序
			host().foreachPeerByLastIrr([this](std::shared_ptr<EthereumPeer> _peer)
			{
				if (_peer->m_asking != Asking::Nothing)
				{
					clog(NetAllDetail) << "Can't sync with this peer - outstanding asks.";
					return true;
				}

				//拒绝已被判定为非法的Peer
				if (!_peer->isLlegal())
					return true;

				clearPeerDownloadMarks(_peer);

				if (host().bq().knownFull())
				{
					clog(NetAllDetail) << "Waiting for block queue before downloading blocks";
					resetSyncTempData();
					switchState(SyncState::Idle);
					return false;
				}

				syncHeadersAndBodies(_peer);
				
				return false; //只访问最高不可逆高度的节点
			});
		}



		void SyncBlocksSyncState::syncHeadersAndBodies(std::shared_ptr<EthereumPeer> _peer)
		{ 
			// check to see if we need to download any block bodies first
			auto header = m_sync.m_headers.begin();
			h256s neededBodies;
			vector<unsigned> neededNumbers;
			vector<BlockChainSync::HeaderId> neededHeaderIds;

			unsigned index = 0;

			if (!m_sync.m_headers.empty() && m_sync.m_headers.begin()->first == m_sync.m_syncStartBlock + 1)
			{//存在相同块，且header不为空，与m_syncStartBlock接上

				while (header != m_sync.m_headers.end() && neededBodies.size() < c_maxRequestBodies && index < header->second.size())
				{
					unsigned block = header->first + index;
					if (m_sync.m_downloadingBodies.count(block) == 0 && !haveItem(m_sync.m_bodies, block))
					{

						neededBodies.push_back(header->second[index].hash);
						neededNumbers.push_back(block);

						BlockHeader h(header->second[index].data, HeaderData); 
						neededHeaderIds.push_back(BlockChainSync::HeaderId{ h.transactionsRoot(), h.sha3Uncles() });

						m_sync.m_downloadingBodies.insert(block);
					}

					++index;
					if (index >= header->second.size())
						break; // Download bodies only for validated header chain
				}
			}
			if (neededBodies.size() > 0)
			{
				cwarn << "request Block Nums:" << neededNumbers;
				cwarn << "request Block Hashes:" << neededBodies;

				for (int i = 0; i < neededHeaderIds.size(); i++)
				{
					if (m_sync.m_headerIdToNumber.find(neededHeaderIds[i]) == m_sync.m_headerIdToNumber.end())
					{
						m_sync.m_headerIdToNumber[neededHeaderIds[i]] = neededNumbers[i];
					}
				}

				m_sync.m_bodySyncPeers[_peer] = neededNumbers;
				_peer->requestBlockBodies(neededBodies);
			}
			else
			{
				// check if need to download headers
				unsigned start = m_sync.m_syncStartBlock + 1;
				auto next = m_sync.m_headers.begin();
				unsigned count = 0;
				if (!m_sync.m_headers.empty() && start >= m_sync.m_headers.begin()->first)
				{
					start = m_sync.m_headers.begin()->first + m_sync.m_headers.begin()->second.size();
					++next;
				}

				while (count == 0 && next != m_sync.m_headers.end())
				{
					count = std::min(c_maxRequestHeaders, next->first - start);
					while (count > 0 && m_sync.m_downloadingHeaders.count(start) != 0)
					{
						start++;
						count--;
					}
					std::vector<unsigned> headers;
					for (unsigned block = start; block < start + count; block++)
						if (m_sync.m_downloadingHeaders.count(block) == 0)
						{
							headers.push_back(block);
							m_sync.m_downloadingHeaders.insert(block);
						}
					count = headers.size();
					if (count > 0)
					{
						m_sync.m_headerSyncPeers[_peer] = headers;
						assert(!haveItem(m_sync.m_headers, start));
						_peer->requestBlockHeaders(start, count, 0, false);
					}
					else if (start >= next->first)
					{
						start = next->first + next->second.size();
						++next;
					}
				} 
			} 
		}

		void SyncBlocksSyncState::onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{ 
			m_lastBlockMsgTimePoint = fc::time_point::now();

			if (updateTimeout())
			{
				return;
			}

			size_t itemCount = _r.itemCount();
			cwarn << "BlocksBodies (" << dec << itemCount << "entries)" << (itemCount ? "" : ": NoMoreBodies");

			clearPeerDownloadMarks(_peer); 

			if (itemCount == 0)
			{
				cwarn << "Peer does not have the blocks requested";
				_peer->addRating(-100);
			}

			//用于打印日志
			std::vector<unsigned> bodies;

			for (unsigned i = 0; i < itemCount; i++)
			{
				RLP body(_r[i]);

				auto txList = body[0];
				h256 transactionRoot = trieRootOver(txList.itemCount(), [&](unsigned i) { return rlp(i); }, [&](unsigned i) { return txList[i].data().toBytes(); });
				h256 uncles = sha3(body[1].data());
				BlockChainSync::HeaderId id{ transactionRoot, uncles };
				auto iter = m_sync.m_headerIdToNumber.find(id);
				if (iter == m_sync.m_headerIdToNumber.end() || !haveItem(m_sync.m_headers, iter->second))
				{   
					cwarn << "Ignored unknown block body";
					continue;
				}
				unsigned blockNumber = iter->second;

				bodies.push_back(blockNumber);

				if (haveItem(m_sync.m_bodies, blockNumber))
				{
					cwarn << "Skipping already downloaded block body " << blockNumber;
					continue;
				}
				m_sync.m_headerIdToNumber.erase(id); 
				mergeInto(m_sync.m_bodies, blockNumber, body.data().toBytes());
			}

			cwarn << "BlockBodies: " << bodies;

			if (checkSyncComplete())
			{
				switchState(SyncState::Idle);
				return;
			}

			if (!collectBlocks())
			{//拼接导入块时出现错误，重新同步
				switchState(SyncState::Idle);
				return;
			}

			if (checkSyncComplete())
			{
				switchState(SyncState::Idle);
				return;
			}
			 
			continueSync();
		}

 


		void SyncBlocksSyncState::onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{
			DefaultSyncState::onPeerNewBlock(_peer, _r);
			keepAlive();
		}

		void SyncBlocksSyncState::onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes)
		{
			DefaultSyncState::onPeerNewHashes(_peer, _hashes);
			keepAlive();
		}

		void SyncBlocksSyncState::onPeerAborting()
		{
			DefaultSyncState::onPeerAborting();
			keepAlive();
		}

		void SyncBlocksSyncState::onEnter()
		{
			updateLastUpdateTime(); 
			//此状态进入时需要触发同步
			continueSync();
		}

		void SyncBlocksSyncState::continueSync()
		{ 
			syncHeadersAndBodies();
		}

		void SyncBlocksSyncState::keepAlive()
		{
			double dt = ((double)((fc::time_point::now() - m_lastBlockMsgTimePoint).to_milliseconds())) / 1000.0;
			if (dt > 5.0)
			{
				continueSync();
				m_lastBlockMsgTimePoint = fc::time_point::now();
			}
		}

		void SyncBlocksSyncState::onTimeout()
		{
			switchState(SyncState::Idle);
		}

		bool SyncBlocksSyncState::checkHeader(const BlockHeader& _h)
		{ 
			unsigned int blockNumber = _h.number().convert_to<unsigned int>();

			// validate chain
			BlockChainSync::HeaderId headerId{ _h.transactionsRoot(), _h.sha3Uncles() }; 
			BlockChainSync::Header const* prevBlock = findItem(m_sync.m_headers, blockNumber - 1);

			if (
				(prevBlock && prevBlock->hash != _h.parentHash()) ||
				(blockNumber == m_sync.m_syncStartBlock + 1 && _h.parentHash() != m_sync.m_syncStartBlockHash))
			{
				cwarn << "Block header mismatching parent!!!";
				if (prevBlock == NULL)
				{
					cwarn << "prevBlock == NULL";
				}
				else {
					cwarn << "prevBlock->hash = " << prevBlock->hash << " info.parentHash() = " << _h.parentHash();
				}

				cwarn << "blockNumber = " << blockNumber << " m_syncStartBlock = " << m_sync.m_syncStartBlock << " m_syncStartBlockHash = " << m_sync.m_syncStartBlockHash;
				// mismatching parent id, delete the previous block and don't add this one
				clog(NetImpolite) << "Unknown block header " << blockNumber << " " << _h.hash() << " (Restart syncing)";

				
				resetAllSyncData();
				return false;
			}

			BlockChainSync::Header const* nextBlock = findItem(m_sync.m_headers, blockNumber + 1);
			if (nextBlock && nextBlock->parent != _h.hash())
			{
				cwarn << "Block header mismatching next block!!!"; 
				clog(NetImpolite) << "Unknown block header " << blockNumber + 1 << " " << nextBlock->hash;
				// clear following headers
				unsigned n = blockNumber + 1;
				auto headers = m_sync.m_headers.at(n);
				for (auto const& h : headers)
				{
					BlockHeader deletingInfo(h.data, HeaderData);
					m_sync.m_headerIdToNumber.erase(headerId);
					m_sync.m_downloadingBodies.erase(n);
					m_sync.m_downloadingHeaders.erase(n);
					++n;
				}
				removeAllStartingWith(m_sync.m_headers, blockNumber + 1);
				removeAllStartingWith(m_sync.m_bodies, blockNumber + 1);
			}
			return true;
		}

		bool SyncBlocksSyncState::collectBlocks()
		{ 
			// merge headers and bodies
			auto& headers = *m_sync.m_headers.begin();
			auto& bodies = *m_sync.m_bodies.begin();
			if (headers.first != bodies.first || headers.first != m_sync.m_syncStartBlock + 1)
				return true;

			unsigned success = 0;
			unsigned future = 0;
			unsigned got = 0;
			unsigned unknown = 0;
			size_t i = 0;
			for (; i < headers.second.size() && i < bodies.second.size(); i++)
			{
				cwarn << "try block import : num = "<< headers.first + i << " hash = "<<headers.second[i].hash;
				RLPStream blockStream(3);
				blockStream.appendRaw(headers.second[i].data);
				RLP body(bodies.second[i]);
				blockStream.appendRaw(body[0].data());
				blockStream.appendRaw(body[1].data());
				bytes block;
				blockStream.swapOut(block);
				switch (host().bq().import(&block))
				{
				case ImportResult::Success:
					success++;
					if (headers.first + i > m_sync.m_syncStartBlock)
					{
						m_sync.m_syncStartBlock = headers.first + (unsigned)i;
						m_sync.m_syncStartBlockHash = headers.second[i].hash;
					}
					break;
				case ImportResult::Malformed:
				case ImportResult::BadChain: 
					resetAllSyncData();
					return false;

				case ImportResult::FutureTimeKnown:
					future++; 
					if (headers.first + i > m_sync.m_syncStartBlock)
					{
						m_sync.m_syncStartBlock = headers.first + (unsigned)i;
						m_sync.m_syncStartBlockHash = headers.second[i].hash;
					}
					break;
				case ImportResult::AlreadyInChain: 
				case ImportResult::AlreadyKnown: 
					if (headers.first + i > m_sync.m_syncStartBlock)
					{
						m_sync.m_syncStartBlock = headers.first + (unsigned)i;
						m_sync.m_syncStartBlockHash = headers.second[i].hash;
					}
					break;
				case ImportResult::FutureTimeUnknown:
				case ImportResult::UnknownParent:
					if (headers.first + i > m_sync.m_syncStartBlock)
					{
						resetAllSyncData();
						return false;
					}
					return true;

				case ImportResult::Irreversible: //遇到了未知的不可逆转块，说明某客户端与当前客户端链严重偏离
					cwarn << "Unknown irreversible block founded!!! ignore and restart sync!"; 
					resetAllSyncData();
					return false;
				default:;
				}
			}

			clog(NetMessageSummary) << dec << success << "imported OK," << unknown << "with unknown parents," << future << "with future timestamps," << got << " already known received.";

			if (host().bq().unknownFull())
			{//太多未知块，需要重新同步 
				clog(NetWarn) << "Too many unknown blocks, restarting sync"; 
				resetAllSyncData();
				return false;
			}

			auto newHeaders = std::move(headers.second);
			newHeaders.erase(newHeaders.begin(), newHeaders.begin() + i);
			unsigned newHeaderHead = headers.first + i;
			auto newBodies = std::move(bodies.second);
			newBodies.erase(newBodies.begin(), newBodies.begin() + i);
			unsigned newBodiesHead = bodies.first + i;
			m_sync.m_headers.erase(m_sync.m_headers.begin());
			m_sync.m_bodies.erase(m_sync.m_bodies.begin());
			if (!newHeaders.empty())
				m_sync.m_headers[newHeaderHead] = newHeaders;
			if (!newBodies.empty())
				m_sync.m_bodies[newBodiesHead] = newBodies;

			return true;  
		}

		void FindingCommonBlockSyncState::onEnter()
		{
			m_unexpectTimes = 0;
			m_lastBlockHeaderTimePoint = fc::time_point::now();
			updateLastUpdateTime();
		}

		void FindingCommonBlockSyncState::onPeerStatus(std::shared_ptr<EthereumPeer> _peer)
		{ 
			DefaultSyncState::onPeerStatus(_peer);
			keepAlive();
		}

		void FindingCommonBlockSyncState::onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{
			m_lastBlockHeaderTimePoint = fc::time_point::now();

			if (updateTimeout())
			{//发生超时
				return;
			}

			cwarn << "onPeerBlockHeaders==============================";
			//获取header数量
			size_t itemCount = _r.itemCount();

			clog(NetMessageSummary) << "BlocksHeaders (" << dec << itemCount << "entries)" << (itemCount ? "" : ": NoMoreHeaders"); 

			printBlockHeadersInfo(_r); 

			clearPeerDownloadMarks(_peer); 

			bool commonHeaderFounded = false;

			if (itemCount == 0)
			{
				cwarn << "Peer does not have the blocks requested !";
				_peer->addRating(-100);
				return;
			}
			else if( itemCount == 1)
			{//只对单个块头的消息进行处理,因为在查找相同块阶段只会有单个块消息出现

				BlockHeader header(_r[0].data(), HeaderData); 
				unsigned blockNumber = static_cast<unsigned>(header.number()); 
				
				if (isExpectBlockHeader(header))
				{
					if (blockNumber < m_sync.m_syncLastIrrBlock )
					{//预期块高度小于不可逆，直接退回Idle状态  
						switchState(SyncState::Idle);
						return;
					}

					if (!haveItem(m_sync.m_headers, blockNumber))
					{
						if (importBlockHeader(_r))
						{//找到CommonHeader

							if (checkSyncComplete())
							{//说明申请的第一个块即为已知块
								switchState(SyncState::Idle);
								return;
							}

							//切换到SyncBlocks进行进一步同步 
							switchState(SyncState::SyncBlocks); 
							return;
						}
						else {//块头已导入，但不是Common，继续向前找

							if (blockNumber == m_sync.m_syncLastIrrBlock)
							{//说明此时peer传入的是不相同的不可逆转块
								_peer->addRating(-10000); 
								switchState(SyncState::Idle);
								return;
							}

							moveToNextCommonBlock();
						}
					}

				} else {//接到非预期块头 
					cwarn << "recv unexpected block header!!!";
					m_unexpectTimes++;
					if (m_unexpectTimes > 20)
					{//有可能对方已切换分叉
						m_unexpectTimes = 0;
						_peer->addRating(-10000);
						switchState(SyncState::Idle);
						return;
					}
				}  
			}else {//回传的Header数量超预期
				cwarn << "Ignore peer unexpected block header !";
			} 
			
			continueSync();
		}
		 

		void FindingCommonBlockSyncState::onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{
			DefaultSyncState::onPeerBlockBodies(_peer, _r);
			keepAlive();
		}

		void FindingCommonBlockSyncState::onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r)
		{
			DefaultSyncState::onPeerNewBlock(_peer, _r);
			keepAlive();
		}

		void FindingCommonBlockSyncState::onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes)
		{
			DefaultSyncState::onPeerNewHashes(_peer, _hashes);
			keepAlive();
		}

		void FindingCommonBlockSyncState::onPeerAborting()
		{
			DefaultSyncState::onPeerAborting(); 
		}

		void FindingCommonBlockSyncState::onTimeout()
		{
			switchState(SyncState::Idle);
		}

		unsigned int FindingCommonBlockSyncState::nextTryCommonBlock() const
		{
			unsigned start = m_sync.m_syncStartBlock;
			if (!m_sync.m_headers.empty())
				start = std::min(start, m_sync.m_headers.begin()->first - 1);
			return start;
		}

		void FindingCommonBlockSyncState::moveToNextCommonBlock()
		{
			//download backwards until common block is found 1 header at a time
			unsigned start = nextTryCommonBlock(); 
			m_sync.m_syncStartBlock = start;
			m_sync.m_syncStartBlockHash = host().chain().numberHash(start);

			m_sync.m_expectBlockForFindingCommon = start;
			m_sync.m_expectBlockHashForFindingCommon = h256();
		}

		bool FindingCommonBlockSyncState::isExpectBlockHeader(const BlockHeader& _h) const
		{
			return 
				m_sync.m_expectBlockHashForFindingCommon == _h.hash() || 
				m_sync.m_expectBlockForFindingCommon == static_cast<unsigned>(_h.number());
		}

		void FindingCommonBlockSyncState::continueSync()
		{
			if (m_sync.m_expectBlockHashForFindingCommon != h256())
			{
				requestExpectHashHeader();
			}
			else {
				requestNextCommonHeader();
			}
		}

		void FindingCommonBlockSyncState::keepAlive()
		{
			double dt = ((double)((fc::time_point::now() - m_lastBlockHeaderTimePoint).to_milliseconds())) / 1000.0;
			if (dt > 5.0)
			{
				continueSync();
				m_lastBlockHeaderTimePoint = fc::time_point::now();
			}
		}

		bool FindingCommonBlockSyncState::importBlockHeader(RLP const& _r)
		{
			BlockHeader header(_r[0].data(), HeaderData);
			unsigned blockNumber = static_cast<unsigned>(header.number());

			auto status = host().bq().blockStatus(header.hash());
			if (status == QueueStatus::Importing || status == QueueStatus::Ready || host().chain().isKnown(header.hash()))
			{//发现相同块
				cwarn << "Block: " << header.hash() << "|" << blockNumber << " => common block header founded!";
				m_sync.m_syncStartBlock = (unsigned)header.number();
				m_sync.m_syncStartBlockHash = header.hash();
				return true;
			}
			else
			{//未找到Common块则将块头并入已块头中 
				BlockChainSync::Header hdr{ _r[0].data().toBytes(), header.hash(), header.parentHash() };
				BlockChainSync::HeaderId headerId{ header.transactionsRoot(), header.sha3Uncles() };

				mergeInto(m_sync.m_headers, blockNumber, std::move(hdr));
				if (headerId.transactionsRoot == EmptyTrie && headerId.uncles == EmptyListSHA3)
				{//空交易体，则直接制造一个空块体即可
					RLPStream r(2);
					r.appendRaw(RLPEmptyList);
					r.appendRaw(RLPEmptyList);
					bytes body;
					r.swapOut(body);
					mergeInto(m_sync.m_bodies, blockNumber, std::move(body));
				}
				else
					m_sync.m_headerIdToNumber[headerId] = blockNumber;
			}
			return false;
		}

		

		void FindingCommonBlockSyncState::requestNextCommonHeader()
		{
			host().foreachPeerByLastIrr([this](std::shared_ptr<EthereumPeer> _p)
			{
				if (_p->m_asking != Asking::Nothing)
				{
					clog(NetAllDetail) << "Can't sync with this peer - outstanding asks.";
					return true;
				}

				//拒绝已被判定为非法的Peer
				if (!_p->isLlegal())
					return true;

				clearPeerDownloadMarks(_p);


				_p->requestBlockHeaders(nextTryCommonBlock(), 1, 0, false);
				return false;
			});
		}

		void FindingCommonBlockSyncState::requestExpectHashHeader()
		{
			host().foreachPeerByLastIrr([this](std::shared_ptr<EthereumPeer> _p)
			{
				if (_p->m_asking != Asking::Nothing)
				{
					clog(NetAllDetail) << "Can't sync with this peer - outstanding asks.";
					return true;
				}

				//拒绝已被判定为非法的Peer
				if (!_p->isLlegal())
					return true; 
				clearPeerDownloadMarks(_p); 

				_p->requestBlockHeaders(m_sync.m_expectBlockHashForFindingCommon, 1, 0, false);
				return false;
			});
		}

 

	}
}