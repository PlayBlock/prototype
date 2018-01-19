#include "DPwTestsHelper.h"
#include <boost/range/algorithm/find.hpp>
using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

namespace dev {
	namespace test {

		DposTestClient::DposTestClient() :
			m_bc(TestBlockChain::defaultDposGenesisBlock()),
			_producer_plugin(make_shared<class producer_plugin>(m_bc.getInterface())),
			_chain(_producer_plugin->get_chain_controller())
		{

			_producer_plugin->get_chain_controller().setStateDB(m_bc.testGenesis().state().db());
			m_bc.interfaceUnsafe().setProducer(_producer_plugin);

			init_accounts();
		}

		void DposTestClient::init_accounts()
		{
			// init genesis account
			genesisAccount.nonce = 0;

			// read all prikey-address pair
			// transfer 1 eth from genesis account to all these addresses
			json_spirit::mValue v;
			string keys_file_path = dev::test::getFolder(__FILE__) + "/DposTests/";
			string s = contentsString(keys_file_path + "address-keys.json");
			json_spirit::read_string(s, v);
			json_spirit::mObject keys = v.get_obj();
			int i = 0;
			for (const auto& key : keys)
			{
				//if (i++ > config::TotalProducersPerRound +5)
				//	break;

				Account account{ key.first , key.second.get_str() , 0 };

				//add key into accounts
				m_accounts.push_back(std::move(account));

				//add address-privatekey pair into _private_keys
				_private_keys[AccountName(key.first)] = private_key(key.second.get_str());
			}

			auto& params = m_bc.getInterface().chainParams();
			for (auto& key : params.privateKeys)
			{
				_private_keys.insert(key);
			}

			//give 1 eth to accounts
			for (const auto& i : m_accounts)
			{
				transfer_eth(genesisAccount, i, u256(1000000000000000000));
			}

			produce_blocks(config::TotalProducersPerRound);

			//test balance
			//Block block(m_bc.interface(), m_bc.testGenesis().state().db());
			//block.populateFromChain(m_bc.interface(),m_bc.interface().currentHash());
			//for (auto i : m_accounts)
			//{
			//	cout<< block.balance(Address(i.second.address)) <<endl;
			//}
		}

		const private_key& DposTestClient::get_private_key(const AccountName& address) const
		{
			return _private_keys.at(address);
		}

		void DposTestClient::produce_blocks(uint32_t count)
		{
			for (int i = 0; i < count; i++)
			{
				auto slot = 1;
				auto producer = _chain.get_scheduled_producer(slot);
				while (producer == AccountName())
					producer = _chain.get_scheduled_producer(++slot);

				//auto producer =  _chain.get_scheduled_producer(slot);
				auto& private_key = get_private_key(producer);
				m_working.dposMine(m_bc, _chain.get_slot_time(slot), producer, private_key);
				m_bc.addBlock(m_working);
				m_working = TestBlock();
			}
		}

		void DposTestClient::produce_pow_blocks(const AccountName& _address, uint32_t count)
		{
			for (int i = 0; i < count; i++)
			{
				auto slot = 1;
				auto producer = _chain.get_scheduled_producer(slot);
				while (producer == AccountName())
					producer = _chain.get_scheduled_producer(++slot);

				BOOST_REQUIRE(producer == _address);
				//auto producer =  _chain.get_scheduled_producer(slot);
				auto& private_key = get_private_key(producer);
				m_working.dposMine(m_bc, _chain.get_slot_time(slot), producer, private_key);
				m_bc.addBlock(m_working);
				m_working = TestBlock();
			}
		}

		void DposTestClient::produce_blocks_Number(uint32_t count, std::map<AccountName, int>& _accountblock)
		{

			for (int i = 0; i < count; i++)
			{
				auto slot = 1;
				auto producer = _chain.get_scheduled_producer(slot);
				while (producer == AccountName())
					producer = _chain.get_scheduled_producer(++slot);
				//计算race生产者的产块数
				if (_accountblock.find(producer) != _accountblock.end())
				{
					_accountblock[producer]++;
				}
				else
				{
					_accountblock.emplace(producer,1);
				}

				//auto producer =  _chain.get_scheduled_producer(slot);
				auto& private_key = get_private_key(producer);
				m_working.dposMine(m_bc, _chain.get_slot_time(slot), producer, private_key);
				m_bc.addBlock(m_working);
				m_working = TestBlock();
			}

		}


		string DposTestClient::getWAVMData(string function, Address address)
		{
			if (address == Address())
			{
				bytes func = bytes(function.begin(), function.end());
				return toHex(func);
			}
			bytes func = bytes(function.begin(), function.end());
			bytes addr = address.asBytes();
			bytes total = func;
			total.push_back('\0');
			total += addr;
			return toHex(total);
		}

		int precompiledCost(int base, int word, const string& data)
		{
			bytes _data = fromHex(data);
			int baseGas = 21000;
			for (auto i : _data)
				baseGas += i ? 68 : 4;

			int s = _data.size();
			int pGas = base + (s + 31) / 32 * word;

			return baseGas + pGas;
		}

		int DposTestClient::mortgage_eth(Account& _from, uint64_t balance)
		{
			string gasLimit = "0x325aa0";
			string gasPrice = "0x77359400";
			string to = "0000000000000000000000000000000000000022";
			string value = "0x0";
			string nonce = boost::lexical_cast<string>(_from.nonce);
			string data = toHex(boost::lexical_cast<string>(balance));
			string secretKey = _from.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			_from.nonce++;

			return precompiledCost(210000, 7, data);
		}

		int DposTestClient::redeem_eth(Account& _from, uint64_t voteCount)
		{
			string gasLimit = "0x325aa0";
			string gasPrice = "0x77359400";
			string to = "0000000000000000000000000000000000000023";
			string value = "0x0";
			string nonce = boost::lexical_cast<string>(_from.nonce);
			string data = toHex(boost::lexical_cast<string>(voteCount));
			string secretKey = _from.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			_from.nonce++;

			return precompiledCost(210000, 8, data);
		}


		int DposTestClient::make_producer(Account& _from, const string& name, const string& url)
		{
			string gasLimit = "0x325aa0";
			string gasPrice = "0x77359400";
			string to = "0000000000000000000000000000000000000024";
			string value = "0x0";
			string nonce = boost::lexical_cast<string>(_from.nonce);
			string data = toHex(name) + "00" + toHex(url);

			string secretKey = _from.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			_from.nonce++;

			return precompiledCost(2100000, 9, data);
		}

		int DposTestClient::unmake_producer(Account& _from)
		{
			string gasLimit = "0x325aa0";
			string gasPrice = "0x77359400";
			string to = "0000000000000000000000000000000000000025";
			string value = "0x0";
			string nonce = boost::lexical_cast<string>(_from.nonce);
			string data = "";
			string secretKey = _from.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			_from.nonce++;

			return precompiledCost(210000, 10, data);
		}
		void DposTestClient::make_pow_transaction(Account& _from, ETIProofOfWork::Solution& _sol)
		{
			string gasLimit = "0xc350";
			string gasPrice = "0x04a817c800";
			string to = "000000000000000000000000000000000000002c";
			string value = "0x0";
			string nonce = boost::lexical_cast<string>(_from.nonce);
			string data = toString(_sol.op._saveImpl());
			string secretKey = _from.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			_from.nonce++;
		}
		void DposTestClient::make_pow_producer(Account& _from, enum class setPowTest tests)
		{
			BlockHeader bh = m_bc.getInterface().info();

			static Secret priviteKey = Secret(_from.secret);
			static AccountName workerAccount(_from.address);
			// 注册回调函数，等待miners找到解后调用
			Notified<bool> sealed(false);

			std::map<dev::h256, std::pair<dev::u256, dev::u256>> map;
			std::unordered_map<dev::u256, dev::u256> mapChange;
			POW_Operation op(map, mapChange, Address());
			ETIProofOfWork::Solution sol = { op };

			sealEngine()->onETISealGenerated([&](const ETIProofOfWork::Solution& _sol) {
				//sol.op. = _sol.op;
				sol.op.worker_pubkey = _sol.op.worker_pubkey;
				sol.op.block_id = _sol.op.block_id;
				sol.op.nonce = _sol.op.nonce;
				sol.op.input = _sol.op.input;
				sol.op.work = _sol.op.work;
				sol.op.signature = _sol.op.signature;

				sealed = true;
			});

			auto tid = std::this_thread::get_id();
			static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

			uint64_t tryNonce = s_eng();
			uint64_t start = tryNonce;
			uint64_t nonce = start;// +thread_num;
			auto target = _producer_plugin->get_chain_controller().get_pow_target();

			ETIProofOfWork::WorkPackage newWork{ bh.hash(), priviteKey, workerAccount, nonce, target };

			// 给miners发送新的任务
			sealEngine()->newETIWork(newWork);
			sealed.waitNot(false);
			sealEngine()->onETISealGenerated([](const ETIProofOfWork::Solution&) {});

			if (setPowTest::errorSignature == tests)
			{
				sol.op.signature = SignatureStruct();
			}
			else if (setPowTest::errorTarget == tests)
			{
				for (; ; tryNonce++)
				{
					op.nonce = tryNonce;
					op.create(priviteKey, op.work_input());
					// 计算结果小于等于target的时候退出，报告找到的这个解
					if (op.work > target)
						break;

				}
				sol.op.work = op.work;
			}
			else if (setPowTest::errorBlockid == tests)
			{    
				//延迟报告
				//produce_blocks(1);

				//一个未知的block_id
				sol.op.block_id = dev::h256("0xca35f4998735926f7ce09bd86e259ca02ea4ef0ab15dbdfdc560b0fad5e1c1b4");
			}
			make_pow_transaction(_from, sol);

		}

		void DposTestClient::sendNewWork(Account& _from, ETIProofOfWork::Solution& _sol, Notified<bool> &_sealed)
		{
			BlockHeader bh = m_bc.getInterface().info();

			static Secret priviteKey = Secret(_from.secret);
			static AccountName workerAccount(_from.address);

			sealEngine()->onETISealGenerated([&](const ETIProofOfWork::Solution& m_sol) {
				//sol.op. = _sol.op;
				_sol.op.worker_pubkey = m_sol.op.worker_pubkey;
				_sol.op.block_id = m_sol.op.block_id;
				_sol.op.nonce = m_sol.op.nonce;
				_sol.op.input = m_sol.op.input;
				_sol.op.work = m_sol.op.work;
				_sol.op.signature = m_sol.op.signature;

				_sealed = true;
			});

			auto tid = std::this_thread::get_id();
			static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));

			uint64_t tryNonce = s_eng();
			uint64_t start = tryNonce;
			uint64_t nonce = start;// +thread_num;
			auto target = _producer_plugin->get_chain_controller().get_pow_target();

			ETIProofOfWork::WorkPackage newWork{ bh.hash(), priviteKey, workerAccount, nonce, target };

			// 给miners发送新的任务
			sealEngine()->newETIWork(newWork);
		
		}
		
		void DposTestClient::add_new_Work(Account& _from)
		{
			
			// 注册回调函数，等待miners找到解后调用
			Notified<bool> sealed(false);
			Notified<bool> sealed2(false);
			std::map<dev::h256, std::pair<dev::u256, dev::u256>> map;
			std::unordered_map<dev::u256, dev::u256> mapChange;
			POW_Operation op(map, mapChange, Address());
			ETIProofOfWork::Solution sol = { op };

			sendNewWork(_from,sol, sealed2);//不需要wait

            produce_blocks(1);

			sendNewWork(_from, sol, sealed);
			sealed.waitNot(false);
			
			sealEngine()->onETISealGenerated([](const ETIProofOfWork::Solution&) {});

			make_pow_transaction(_from, sol);

		}


		dev::h256 DposTestClient::get_ownpow_target()
		{
			const auto& dgp = _producer_plugin->get_chain_controller().get_dynamic_global_properties();
			dev::u256 target;
			target = -1;
			target >>= ((dgp.num_pow_witnesses / 4) + 4);
			return dev::h256(target);
		}

		int DposTestClient::send(Account& _from, const Account& on, uint64_t voteCount)
		{
			string gasLimit = "0x325aa0";
			string gasPrice = "0x77359400";
			string to = "000000000000000000000000000000000000002a";
			string value = "0x0";
			string nonce = boost::lexical_cast<string>(_from.nonce);
			string data = on.address + toHex(boost::lexical_cast<string>(voteCount));
			string secretKey = _from.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			_from.nonce++;

			return precompiledCost(210000, 6, data);
		}

		int DposTestClient::approve_producer(Account& voter, const Account& on, uint64_t voteCount)
		{
			string gasLimit = "0x325aa0";
			string gasPrice = "0x77359400";
			string to = "0000000000000000000000000000000000000026";
			string value = "0x0";
			string nonce = boost::lexical_cast<string>(voter.nonce);
			string data = on.address + toHex(boost::lexical_cast<string>(voteCount));
			string secretKey = voter.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			voter.nonce++;

			return precompiledCost(210000, 6, data);
		}

		int DposTestClient::unapprove_producer(Account& voter, const Account& on, uint64_t voteCount)
		{
			string gasLimit = "0x325aa0";
			string gasPrice = "0x77359400";
			string to = "0000000000000000000000000000000000000027";
			string value = "0x0";
			string nonce = boost::lexical_cast<string>(voter.nonce);
			string data = on.address + toHex(boost::lexical_cast<string>(voteCount));
			string secretKey = voter.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			voter.nonce++;

			return precompiledCost(210000, 6, data);
		}

		void DposTestClient::transfer_eth(Account& _from, const Account& _to, const u256& _value)
		{
			string gasLimit = "0xc350";
			string gasPrice = "0x77359400";
			string to = _to.address;
			string value = _value.str();
			string nonce = boost::lexical_cast<string>(_from.nonce);
			string data = "0x";
			string secretKey = _from.secret;

			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", nonce));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", secretKey));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			_from.nonce++;
		}

		const chain::ProducerRound DposTestClient::get_active_producers() {
			if (_producer_plugin == nullptr)
			{
				throw std::runtime_error("uninit producer plugin");
			}

			chain::ProducerRound round;
			for (int i = 0; i < _producer_plugin->get_chain_controller().get_global_properties().active_producers.size(); i++)
			{
				round.push_back(_producer_plugin->get_chain_controller().get_global_properties().active_producers[i]);
			}

			return round;
		}

		Accounts& DposTestClient::get_accounts()
		{
			return m_accounts;
		}

		const std::map<Address, VoteInfo> DposTestClient::get_votes()
		{
			return _producer_plugin->get_chain_controller().get_votes();
		}

		const std::map<Address, uint64_t> DposTestClient::get_all_producers()
		{
			return _producer_plugin->get_chain_controller().get_producers();
			//std::map<Address, VoteInfo> voteInfo = _producer_plugin->get_chain_controller().get_votes();
			//std::map<Address, uint64_t> ret;
			//for (auto i : voteInfo)
			//{
			//	ret[i.first] = i.second.getReceivedVotedNumber();
			//}
			//return ret;
		}

		u256 DposTestClient::balance(const Address& _address) const
		{
			Block block(m_bc.getInterface(), m_bc.testGenesis().state().db());
			block.populateFromChain(m_bc.getInterface(), m_bc.getInterface().currentHash());
			return block.balance(_address);
		}

		bytes DposTestClient::code(const Address& _address) const
		{
			Block block(m_bc.getInterface(), m_bc.testGenesis().state().db());
			block.populateFromChain(m_bc.getInterface(), m_bc.getInterface().currentHash());
			return block.code(_address);
		}

		u256 DposTestClient::storage(Address const & _contract, u256 const & _memory) const
		{
			Block block(m_bc.getInterface(), m_bc.testGenesis().state().db());
			block.populateFromChain(m_bc.getInterface(), m_bc.getInterface().currentHash());
			return block.storage(_contract, _memory);
		}

		void DposTestClient::sendTransaction(const string & gasLimit, const string & gasPrice, const string & to, const string & value, const string & data, Account& _from)
		{
			json_spirit::mObject obj;
			obj.emplace(make_pair("gasLimit", gasLimit));
			obj.emplace(make_pair("gasPrice", gasPrice));
			obj.emplace(make_pair("to", to));
			obj.emplace(make_pair("value", value));
			obj.emplace(make_pair("nonce", boost::lexical_cast<string>(_from.nonce)));
			obj.emplace(make_pair("data", data));
			obj.emplace(make_pair("secretKey", _from.secret));
			TestTransaction tx(obj);
			m_working.addTransaction(tx);

			_from.nonce++;
		}

		void DposTestClient::sendTransaction(TransactionSkeleton const & _ts, Secret const & _s)
		{
			Transaction t(_ts, _s);

			TestTransaction tx(t);
			m_working.addTransaction(tx);
		}

		void DposTestClient::setGasLimit(u256 const & _v)
		{
			cout << "_v.str(): " << _v.str() << endl;
			BlockHeader bh(m_working.blockHeader());
			bh.setGasLimit(_v);
			m_working.setBlockHeader(bh);
		}
	}
}