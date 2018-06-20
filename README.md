# prototype - A PlayBlock Chain Prototype Project



### Build


```shell
mkdir build

./scripts/install_deps.bat

cd build

cmake .. -G "Visual Studio 14 2015 Win64"

```



### Note: This project is a local git repository, so the number of contributors displayed on github is less than the actual contributors.

### Next Verion Plan:
- **Develop the whole new blockchain framework use GO language**
- **Replace the DPOS consensus algorithm Use Gosig**
- **Delete the POW**

 
#### Optimization work based on ETH
- **Consensus algorithm optimization:**
	- Replace ETH's POW consensus with DPOS+POW. The specific algorithm is as follows:
		- The process of generating a block consists of rounds, with 21 nodes participating in each round. Each node is rewarded with system rewards and transaction fees if it produces a block.
		- At the end of each round, the system selects the next 21 nodes.
		- 21 nodes in each round consist of 4 POW nodes and 17 DPOS nodes
		- This ratio will change over time.
		- In the first 30 days of Genesis, the chain prefers POW nodes.
		- After 30 days of Genesis, since then, there are only 4 POW nodes in the chain
		- DPOS election algorithm：
			- Select 16 DPOS nodes from highest to lowest according to the number of votes.
			- The last one DPOS node is selected according to an algorithm.
			- This algorithm guarantees that the remaining DPOS nodes (not the first 16 DPOS nodes) have the same probability of being selected by the system and the weight of the votes of each other.
		- POW election
			- The node that has calculated the hash problem will broadcast the solution result (also a transaction) to the entire network. If the block is generated in time (needs to be solved before the next block output or otherwise invalid), the node is recorded in the POW node list.
			- The system selects the POW node of the next round as follows: The oldest POW node recorded in the list will be selected and deleted from the list.
	- POW calculation algorithm uses MLGB's hash hopping algorithm.

- **Contract VM Optimization:** use EOS WAVM
- **P2P synchronization logic optimization:**
	- Rewritten ETH's P2P synchronization module
	- Solved the ETH P2P synchronization logic in the weak network environment easily forked bugs and various synchronization bugs
	- Block producers broadcast blocks directly after producing blocks to speed up synchronization
- **Transaction verification speed optimization:**
	- Transaction inspection algorithm changed from ETH's single-thread inspection algorithm to multi-thread inspection algorithm
- **Main production block flow optimization**
	- In the DPOS algorithm of EOS, the block producer checks whether a block is generated every 1 second. In ETI, this interval is changed to millisecond polling to prevent frequent missed block time.
- **Stability optimization:**
	- The synchronization logic is decoupled from the block production flow and does not affect the current node production block when synchronizing the external block
- **RLP structure optimization:**
	- Fixed a bug in ETH's RLP accessing slowly under tens of thousands of transaction cases.
- **State tree build optimization:**
	- Optimize the original single-threaded tree construction algorithm as a multi-threaded tree construction algorithm
- **Others：**
	- Removed Uncle block reward from ETH
	- Removes the fork processing logic from the ETH
	- Added a large number of test cases to fix bugs and new features to ensure system stability

#### Need to do optimization work later:
- **Question: As the number of accounts increases, the block takes longer to import**
	- **Test environment:**
		- Two ETI nodes with SSD disks, one node sending huge transactions to another node
		- The process of sending the transaction is to create 10,240 accounts and make 10,240 transactions in each block interval. Each transaction is sent from a Genesis rich account to a new account.
		- This test environment will maximize the number of transactions per block.
		- Note: This pressure test environment is very strict and may exceed the actual operating conditions. In this test environment, 280 million accounts will be generated each day, while Ethereum is currently only about 31 million accounts.
	- **Result:** The time for importing blocks will increase as the number of accounts grows. With the maximum number of transactions, after the 1,000th block is produced (that is, the number of system accounts has reached 10 million, and the total number of current Ethereum accounts is only 31 million), A block time will reach about 3 seconds and the import time will continue to rise.
	- **Reason:** Each block has a state tree, and the leaves node stores the state of each account. This tree is reconstructed for every block that is produced, and the tree is also queried when the transaction is executed. As the number of accounts increases, the depth of the tree increases and the time for importing blocks increases. That is, the import block time is positively correlated with the total number of accounts in the system and the number of transactions in the block. For example, when the chain full block status has yielded 1,000 blocks and the lead time has changed to 3 seconds, even if the subsequent transaction is transferred to a known account in the system, In each full block state is still 3 seconds.
	- **Solution:** There are currently two options:
		- Scenario 1: Continue to use ETH's state tree, use caching to mitigate slow access problems
		- Scenario 2: Full use of map + memory-mapped file storage in EOS solutions
#### Some technical parameters:
- **TPS:** 3413tx/s
- **Block size:** 10240 txs/block, one block size equal to approximately 1MByte
- **gas limit：**
	- Every simple transaction: 21000wei
	- Maximum number of transactions in a block：10240
	- Block gaslimit = 10240 * 21000 wei
- **Transaction DATA field data volume limit:**
	- Calculate gas according to transaction data volume. The larger the data amount is, the higher the gas is, and the upper limit is only affected by the block gas limit.

#### Hardware requirements
- **Test environment:** Two nodes one production block and one receiving block
- **Disk usage:** The initial disk usage of the node is about 1GB (used to save the EOS database memory mapping file). When each block is full, the node consumes about 28GB of disk space a day.
- **Memory usage:** About 1GB
- **CPU  usage:** 36% ( Intel(R) Xeon(R) CPU E5-26xx v4 8Core )
- **Network bandwidth usage:** 2Mbps/Node

---------------------------------------------------------------------------

### DAPP On Chain

#### Block Dungeon

- The first H5 dungeon battle game based on blockchain technology. The Block Dungeon game rules are based on cryptography and deployed on the PlayBlock Chain in the form of smart contracts. Because the data of the blockchain is unchangeable and transparent, it ensures the fairness of the game rules and the security of asset transactions among players. Relying on the high TPS performance and scalability of PlayBlock Chain, Block Dungeon theoretically has no upper limit on the number of online players.

![](https://github.com/walden01/prototype/raw/master/app_screenshots/boot.png)
![](https://github.com/walden01/prototype/raw/master/app_screenshots/maze_battle1.jpg)
![](https://github.com/walden01/prototype/raw/master/app_screenshots/wheel.jpg)
![](https://github.com/walden01/prototype/raw/master/app_screenshots/frontpage.jpg)
![](https://github.com/walden01/prototype/raw/master/app_screenshots/eq.jpg)
