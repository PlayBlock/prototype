#pragma once

#include <mutex>
#include <unordered_map>

#include <libdevcore/Guards.h>
#include <libethcore/Common.h>
#include <libethcore/BlockHeader.h>
#include <libp2p/Common.h>
#include "CommonNet.h"

namespace dev
{ 
	class RLPStream;

	namespace eth
	{
		class EthereumHost;
		class BlockQueue;
		class EthereumPeer; 
		class BlockChainSync;

		class BlockChainSyncState
		{
		public:
			BlockChainSyncState(BlockChainSync& _sync);
			 
			virtual void onPeerStatus(std::shared_ptr<EthereumPeer> _peer) = 0; 
			virtual void onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r) = 0; 
			virtual void onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r) = 0; 
			virtual void onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r) = 0; 
			virtual void onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes) = 0; 
			virtual void onPeerAborting() = 0; 
			virtual void onBlockImported(BlockHeader const& _info, const uint32_t _last_irr_block) = 0;

			virtual void onEnter() = 0;
			virtual void onLeave() = 0;

		protected:
			BlockChainSync& m_sync; 
		};


		class DefaultSyncState : public BlockChainSyncState
		{
		public:
			DefaultSyncState(BlockChainSync& _sync) :BlockChainSyncState(_sync) { m_lastUpdateTime = fc::time_point::now(); }
			virtual void onPeerStatus(std::shared_ptr<EthereumPeer> _peer);
			virtual void onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes);
			virtual void onPeerAborting();
			virtual void onBlockImported(BlockHeader const& _info, const uint32_t _last_irr_block);

			virtual void onEnter() { updateLastUpdateTime(); }
			virtual void onLeave() {}

		protected:
			/*==============超时处理==================*/

			void updateLastUpdateTime() 
			{ 
				m_lastUpdateTime = fc::time_point::now(); 
			}

			//返回true表示出现timeout
			bool updateTimeout() 
			{  
				if (elapseTime() > timeoutElapseSec())
				{//超时
					ctrace << "TIME OUT!!! elapse secs > " << timeoutElapseSec();
					this->onTimeout();
					return true;
				}
				m_lastUpdateTime = fc::time_point::now();
				return false;
			}

			double elapseTime() { return ((double)((fc::time_point::now() - m_lastUpdateTime).to_milliseconds())) / 1000.0; }

			virtual double timeoutElapseSec() const { return DBL_MAX; }

			virtual void onTimeout() {}
			 

			/*==============工具函数==================*/ 
			 
			EthereumHost& host();
			EthereumHost const& host() const;
			BlockQueue& bq();
			BlockQueue const& bq() const;

			bool haveBlockHeader(uint32_t _num);  

			void switchState(SyncState _s);

			//清空当前Peer的下载记录
			void clearPeerDownloadMarks(std::shared_ptr<EthereumPeer> _peer);
			
			bool peerLegalCheck(std::shared_ptr<EthereumPeer> _peer);

			//检查m_headers是否为空，若为空则返回true
			bool checkSyncComplete() const;

			//清除临时数据
			void resetSyncTempData();

			//在重大错误时调用，除了清除同步用临时数据还会清bq
			void resetAllSyncData();

			void printBlockHeadersInfo(RLP const& _r); 

			/*==============Peer同步相关函数==================*/

			virtual void requestPeerLatestBlockHeader(std::shared_ptr<EthereumPeer> _peer);

			void requestBlocks(std::shared_ptr<EthereumPeer> _peer);

			virtual void continueSync() {}

		protected:

			fc::time_point m_lastUpdateTime;
		};



		class IdleSyncState : public DefaultSyncState
		{
		public:
			IdleSyncState(BlockChainSync& _sync):DefaultSyncState(_sync) {} 
			virtual void onPeerStatus(std::shared_ptr<EthereumPeer> _peer); 
			virtual void onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes); 

			virtual void onEnter() 
			{ 
				updateLastUpdateTime();
				resetSyncTempData(); 
			}
		};
		
		
		class FindingCommonBlockSyncState : public DefaultSyncState
		{
		public:
			FindingCommonBlockSyncState(BlockChainSync& _sync) :DefaultSyncState(_sync) {}
			virtual void onPeerStatus(std::shared_ptr<EthereumPeer> _peer);
			virtual void onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes);
			virtual void onPeerAborting();
			virtual void onEnter();
		protected: 

			//十秒超时
			virtual double timeoutElapseSec() const { return 10.0; }

			virtual void onTimeout();

			//返回下一个尝试查找的Common块
			unsigned int nextTryCommonBlock() const; 
			//导入接收到的块头，返回是否找到Common块
			bool importBlockHeader(RLP const& _r);
			void moveToNextCommonBlock(); 
			void requestNextCommonHeader(); 
			void requestExpectHashHeader(); 
			bool isExpectBlockHeader(const BlockHeader& _h) const; 

			virtual void continueSync();

			//每隔一段时间发送请求BlockHeader命令，保持FindingCommon状态的活性
			void keepAlive();
		private: 

			int	m_unexpectTimes = 0;					    ///<收到非预期次数
			fc::time_point m_lastBlockHeaderTimePoint; ///<最近一次接到BlockHeader的时间点
		};

		class SyncBlocksSyncState : public DefaultSyncState
		{
		public:
			SyncBlocksSyncState(BlockChainSync& _sync) :DefaultSyncState(_sync) {}
			virtual void onPeerStatus(std::shared_ptr<EthereumPeer> _peer);
			virtual void onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);
			virtual void onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes);
			virtual void onPeerAborting();
			virtual void onEnter();

		protected: 

			virtual void continueSync();

			void keepAlive();

			//十秒超时
			virtual double timeoutElapseSec() const { return 10.0; }

			virtual void onTimeout();

			//检查块头的前驱或后继是否合法
			//当前驱不合法时返回false，否则返回true
			//当后继不合法时会清理此不合法Header及后续所有Header
			bool checkHeader(const BlockHeader& _h);

			//尝试合并块头块体并导入
			//返回值为是否成功，若返回false则需要切换回Idle
			bool collectBlocks();

			void syncHeadersAndBodies();

			void syncHeadersAndBodies(std::shared_ptr<EthereumPeer> _peer);

		private: 
			fc::time_point m_lastBlockMsgTimePoint; ///<最近一次接到BlockHeader的时间点
		};


		class NotSyncedState : public DefaultSyncState
		{
		public:
			NotSyncedState(BlockChainSync& _sync) :DefaultSyncState(_sync) {}
		};

		class WaitingSyncState : public DefaultSyncState
		{
		public:
			WaitingSyncState(BlockChainSync& _sync) :DefaultSyncState(_sync) {}
		};



		class BlockSyncState : public DefaultSyncState
		{
		public:
			BlockSyncState(BlockChainSync& _sync) :DefaultSyncState(_sync) {}
		};

	}
}