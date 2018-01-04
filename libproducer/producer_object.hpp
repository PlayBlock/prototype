#pragma once
 
#include "types.hpp"
#include "chainbase.hpp"
#include "multi_index_includes.hpp"
#include "fc/time.hpp"
#include "version.hpp"

using dev::types::AccountName;
using dev::eth::chain::public_key_type; 
using fc::time_point_sec;

namespace eos { namespace chain {
class producer_object : public chainbase::object<producer_object_type, producer_object> {
   OBJECT_CTOR(producer_object)

   id_type          id;
   AccountName      owner;
   uint64_t         last_aslot = 0;
   //public_key_type  signing_key;
   int64_t          total_missed = 0;
   uint32_t         last_confirmed_block_num = 0;


   //移植steemit pow worker在队列中的排位，越小越靠前
   uint64_t        pow_worker = 0;
   /// The blockchain configuration values this producer recommends
   //BlockchainConfiguration configuration;

   //Producer跑的客户端版本
   dev::eth::chain::version running_version;

   //Hardfork 投票相关
   dev::eth::chain::hardfork_version  hardfork_ver_vote;
   time_point_sec    hardfork_time_vote = dev::eth::config::ETI_GenesisTime;
};

struct by_key;
struct by_owner;
//用来排序POW worker队列
struct by_pow;
using producer_multi_index = chainbase::shared_multi_index_container<
   producer_object,
   indexed_by<
      ordered_unique<tag<by_id>, member<producer_object, producer_object::id_type, &producer_object::id>>,
      ordered_unique<tag<by_owner>, member<producer_object, AccountName, &producer_object::owner>>,
      ordered_unique<tag<by_key>, member<producer_object, producer_object::id_type, &producer_object::id>>,
	  ordered_non_unique<tag<by_pow>, member<producer_object, uint64_t, &producer_object::pow_worker>>
   >
>;

} } // eos::chain

CHAINBASE_SET_INDEX_TYPE(eos::chain::producer_object, eos::chain::producer_multi_index)



#include <fc/reflect/reflect.hpp>
FC_REFLECT(eos::chain::producer_object::id_type, (_id))
FC_REFLECT(eos::chain::producer_object, (id)(owner)(last_aslot)(total_missed)(last_confirmed_block_num)(pow_worker)(running_version)(hardfork_ver_vote)(hardfork_time_vote))
