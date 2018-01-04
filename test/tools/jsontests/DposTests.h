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
/** @file BlockChainTests.h
 * BlockChainTests functions.
 */

#pragma once
#include "BlockChainTests.h"
#include <libethereum/Block.h>
#include <libethereum/BlockChain.h>
#include <test/tools/libtesteth/TestHelper.h>
#include <libethereum/GenesisInfo.h>
#include <libethashseal/EthashCPUMiner.h>
#include <libproducer/chain_controller.hpp>
#include <libevm/Vote.h>
#include <test/tools/libtestutils/Common.h>
#include <libproducer/producer_plugin.hpp>
#include <test/tools/libtesteth/BlockChainHelper.h>
#include <libethcore/SealEngine.h>

using namespace json_spirit;
namespace dev {
	namespace test {

		struct Account {
			string address;
			string secret;
			uint64_t nonce;
		};

		using Accounts = std::vector<Account>;
		using fc::ecc::private_key;
		using types::AccountName;

		static Account genesisAccount = { "0x110e3e0a01EcE3a91e04a818F840E9E3D17B3C8f", "b81b893df61f227bdc4858012c3bbfab9e695d781911db3f2b12696ade186c56", 0 };

		class DposTestClient
		{
		public:
			DposTestClient();

			void init_accounts();

			const private_key& get_private_key(const AccountName& address) const;

			void produce_blocks(uint32_t count = 1);
			string getWAVMData(string function, Address address = Address());

			void mortgage_eth(Account& _from, uint64_t balance);

			void redeem_eth(Account& _from, uint64_t voteCount);

			void assign(Account& _from, uint64_t voteCount);

			void deAssign(Account& _from, uint64_t voteCount);

			void make_producer(Account& _from);

			void unmake_producer(Account& _from);

			void make_pow_producer(Account& _from);
			void make_pow_transaction(Account& _from, ETIProofOfWork::Solution& _sol);

			void send(Account& _from, const Account& on, uint64_t voteCount);

			void approve_producer(Account& voter, const Account& on);

			void unapprove_producer(Account& voter);

			void transfer_eth(Account& _from, const Account& _to, const u256& _value);

			const chain::ProducerRound& get_active_producers();

			Accounts& get_accounts();

			const std::map<Address, VoteBace> get_votes();

			const std::map<Address, uint64_t> get_all_producers();

			u256 balance(const Address& _address) const;

		    void sendTransaction(const string& gasLimit, const string& gasPrice, const string& to, const string& value, const string& data, Account& _from);

			SealEngineFace* sealEngine() const  { return m_bc.getInterface().sealEngine(); };

		private:

			TestBlockChain m_bc;
			TestBlock m_working;
			std::shared_ptr<class producer_plugin> _producer_plugin;
			eth::chain::chain_controller& _chain;
			Accounts m_accounts;
			std::map<types::AccountName, fc::ecc::private_key> _private_keys;
		};

		class DposBlockTestSuite : public BlockchainTestSuite
		{
		public:
			boost::filesystem::path suiteFolder() const override;
			boost::filesystem::path suiteFillerFolder() const override;
		};
	}
}