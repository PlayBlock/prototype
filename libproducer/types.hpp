#pragma once
#include <string>
#include <map>
#include <set>
#include <array>
#include "config.hpp"
#include "libdevcrypto/Common.h"


#define OBJECT_CTOR1(NAME) \
    NAME() = delete; \
    public: \
    template<typename Constructor, typename Allocator> \
    NAME(Constructor&& c, chainbase::allocator<Allocator>) \
    { c(*this); }
#define OBJECT_CTOR2_MACRO(x, y, field) ,field(a)
#define OBJECT_CTOR2(NAME, FIELDS) \
    NAME() = delete; \
    public: \
    template<typename Constructor, typename Allocator> \
    NAME(Constructor&& c, chainbase::allocator<Allocator> a) \
    : id(0) BOOST_PP_SEQ_FOR_EACH(OBJECT_CTOR2_MACRO, _, FIELDS) \
    { c(*this); }
#define OBJECT_CTOR(...) BOOST_PP_OVERLOAD(OBJECT_CTOR, __VA_ARGS__)(__VA_ARGS__)

namespace dev {

namespace types {
	using AccountName = Address;
	using AccountNames = std::set<AccountName>;
	using PublicKey = Public;
	using ShareType = Int64;
	using block_id_type = h256;
}

namespace eth {

namespace chain {
	using types::AccountName;
	using public_key_type = types::PublicKey;

	using ProducerRound = std::vector<AccountName>;
	using RoundChanges = std::map<AccountName, AccountName>;
}
}
}

namespace fc {
	namespace ecc {
		using private_key = dev::Secret;

	}
}

namespace eos { namespace chain {
enum object_type
{
	null_object_type,
	account_object_type,
	permission_object_type,
	permission_link_object_type,
	action_code_object_type,
	key_value_object_type,
	key128x128_value_object_type,
	action_permission_object_type,
	global_property_object_type,
	dynamic_global_property_object_type,
	block_summary_object_type,
	transaction_object_type,
	generated_transaction_object_type,
	producer_object_type,
	chain_property_object_type,
	account_control_history_object_type, ///< Defined by account_history_plugin
	account_transaction_history_object_type, ///< Defined by account_history_plugin
	transaction_history_object_type, ///< Defined by account_history_plugin
	public_key_history_object_type, ///< Defined by account_history_plugin
	balance_object_type, ///< Defined by native_contract library
	staked_balance_object_type, ///< Defined by native_contract library
	producer_votes_object_type, ///< Defined by native_contract library
	producer_schedule_object_type, ///< Defined by native_contract library
	proxy_vote_object_type, ///< Defined by native_contract library
	key64x64x64_value_object_type,
	process_hardfork_object_type, ///<ÓÃÓÚÓ²·Ö²æ
	OBJECT_TYPE_COUNT ///< Sentry value which contains the number of different object types
};
}
}