#pragma once
#include "fc/time.hpp"
#include "config.hpp"
#include "global_property_object.hpp"
#include "types.hpp"
#include "producer_object.hpp"
#include "chainbase.hpp"
#include <libethereum/Block.h>
#include <map>
//#include "producer_objects.hpp"
//#include <libevm/Vote.h>
#include <libethereum/BlockChain.h>

namespace dev {
namespace eth {

namespace chain {
	using namespace eos::chain;

class chain_controller {
public:

	chain_controller(const dev::eth::BlockChain& bc, chainbase::database& db);

	chain_controller(chain_controller&&) = default;
	~chain_controller();

	void chain_controller::initialize_indexes();
	void chain_controller::initialize_chain(const dev::eth::BlockChain& bc);


	fc::time_point_sec get_slot_time(const uint32_t slot_num) const;

	uint32_t get_slot_at_time(const fc::time_point_sec when)const;

	uint32_t block_interval()const { return config::BlockIntervalSeconds; }


	void push_block(const BlockHeader& b);



	//void init_global_property();


	const global_property_object&          get_global_properties()const;
	const dynamic_global_property_object&  get_dynamic_global_properties()const;
	const producer_object&                 get_producer(const AccountName& ownerName)const;

	fc::time_point_sec   head_block_time()const;
	uint32_t         head_block_num()const;


	types::AccountName get_scheduled_producer(uint32_t slot_num)const;
//	const producer_object& get_producer(const types::AccountName& ownerName) const;
//
//	//signed_block generate_block(
//	//	fc::time_point_sec when,
//	//	const AccountName& producer,
//	//	const fc::ecc::private_key& block_signing_private_key
//	//);
//
	ProducerRound calculate_next_round(const BlockHeader& next_block);
//
//	void setStateDB(const OverlayDB& db) { _db.setStateDB(db); }
//	const chainbase::database& db() { return _db; }
//
//
	void setStateDB(const OverlayDB& db) { _stateDB = &db; }

	void databaseReversion(uint32_t _firstvalid);

	//std::map<Address, VoteBace> chain_controller::get_votes(h256 const& _hash = h256()) const;

private:
	void update_global_dynamic_data(const BlockHeader& b);
	void update_global_properties(const BlockHeader& b);
	void update_signing_producer(const producer_object& signing_producer, const BlockHeader& new_block);

	void update_pvomi_perblock();

	void update_last_irreversible_block();

	void apply_block(const BlockHeader& b);
	const producer_object& validate_block_header(const BlockHeader& bh)const;

private:
	chainbase::database& _db;
	const BlockChain& _bc;
	const OverlayDB* _stateDB;
	std::unordered_map<Address, uint64_t> _all_votes;
};

}
}
}