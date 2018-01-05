
#include "Inline/BasicTypes.h"
#include "Inline/Timing.h"
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"
#include "Emscripten/Emscripten.h"
#include "IR/Module.h"
#include "IR/Operators.h"
#include "IR/Validate.h"
#include "../libevm/WASM_VM.h"
#include "../libevm/Vote.h"

#include <iostream>
#include <map>
#include "register.h"

inline Address asAddress(u256 _item)
{
	return right160(h256(_item));
}

inline u256 fromAddress(Address _a)
{
	return (u160)_a;
}



DEFINE_INTRINSIC_FUNCTION2(env, readMessage, readMessage, i32, i32, destptr, i32, destsize) {
	if (destsize <= 0)
	{
		BOOST_THROW_EXCEPTION(WASMWrongMemory());
	}
	bytes para = WASM_CORE::getParameter();
	auto mem = WASM_CORE::getMemory();
	char* begin = memoryArrayPtr<char>(mem, destptr, uint32_t(destsize));
	int minlen = std::min<int>(para.size(), destsize);
	memcpy(begin, para.data(), minlen);
	return minlen;
}


//void resetVotedTo(dev::Address address)
//{
//	ExtVMFace* _ext = WASM_CORE::getExt();
//
//	auto storageMap = (*(_ext)).getStorage();
//	for (auto iterator = storageMap.begin(); iterator != storageMap.end(); iterator++)
//	{
//		u256 iteratorFirst = iterator->first;
//		dev::Address addressVotedTo = Vote::getVotedTo((*(_ext)), iteratorFirst);
//		if (addressVotedTo == address)
//		{
//			Vote::setVotedTo((*(_ext)), iteratorFirst, dev::Address(0));
//			Vote::setIsVoted((*(_ext)), iteratorFirst, 0);
//		}
//	}
//
//}


//DEFINE_INTRINSIC_FUNCTION2(env, setCandidate, setCandidate, i32, i32, from, i32, isCandidate)
//{
//	ExtVMFace* _ext = WASM_CORE::getExt();
//	auto mem = WASM_CORE::getMemory();
//	Address& _from = memoryRef<Address>(mem, from);
//
//	if (_from == Address(0))
//	{
//		return -3;
//	}
//
//	int _isCandidate;
//	if (isCandidate == 0)
//		_isCandidate = 0;
//	else
//		_isCandidate = 1;
//
//	//Do nothing.
//	if (Vote::getIsCandidate(*(_ext), (u160)_from) == _isCandidate)
//	{
//		return 2;
//	}
//
//	//Candidate deregister.
//	if (_isCandidate == 0)
//	{
//		resetVotedTo(_from);
//	}
//	Vote::setVotedNumber(*(_ext), (u160)_from, 0);
//
//	Vote::setIsCandidate(*(_ext), (u160)_from, _isCandidate);
//
//	return 1;
//}

//DEFINE_INTRINSIC_FUNCTION3(env, setVote, setVote, i32, i32, from, i32, to, i32, isVote)
//{
//	ExtVMFace* _ext = WASM_CORE::getExt();
//	auto mem = WASM_CORE::getMemory();
//	Address& _from = memoryRef<Address>(mem, from);
//	Address& _to = memoryRef<Address>(mem, to);
//
//	u256 a = (u160)_from;
//	u256 b = (u160)_to;
//	std::string s_from = toHex(a);
//	std::string s_to = toHex(b);
//
//	if (_from == Address(0))
//	{
//		return -3;
//	}
//
//	int _isVote;
//	if (isVote == 0)
//		_isVote = 0;
//	else
//		_isVote = 1;
//
//	//Set vote = 0.
//	if (_isVote == 0)
//	{
//		//Do nothing.
//		if (Vote::getIsVoted(*(_ext), (u160)_from) == 0)
//		{
//			return 2;
//		}
//
//		uint64_t voteNumber = Vote::getVotedNumber(*(_ext), (u160)Vote::getVotedTo(*(_ext), (u160)_from));
//		//Remove vote.
//		if (voteNumber - 1 > voteNumber)
//		{
//			return -2;
//		}
//		Vote::setVotedNumber(*(_ext), (u160)Vote::getVotedTo(*(_ext), (u160)_from), voteNumber - 1);
//	}
//	//Set vote = 1.
//	else
//	{
//		//Illegal candidate.
//		if (Vote::getIsCandidate(*(_ext), (u160)_to) == 0)
//		{
//			return 0;
//		}
//
//		//Vote first time.
//		if (Vote::getIsVoted(*(_ext), (u160)_from) == 0)
//		{
//			uint64_t voteNumber = Vote::getVotedNumber(*(_ext), (u160)_to);
//			if (voteNumber + 1 < voteNumber)
//			{
//				return -1;
//			}
//			Vote::setVotedNumber(*(_ext), (u160)_to, voteNumber + 1);
//		}
//		//Revote.(Vote second time.)
//		else
//		{
//			Address voteBeforeAccount = Vote::getVotedTo(*(_ext), (u160)_from);
//			//If voteBefore account equals to voteTo account, do nothing.
//			if (voteBeforeAccount == _to)
//			{
//				return 2;
//			}
//
//			uint64_t voteNumber_before = Vote::getVotedNumber(*(_ext), (u160)voteBeforeAccount);
//			uint64_t voteNumber_after = Vote::getVotedNumber(*(_ext), (u160)_to);
//			if (voteNumber_before - 1 > voteNumber_before)
//			{
//				return -2;
//			}
//			if (voteNumber_after + 1 < voteNumber_after)
//			{
//				return -1;
//			}
//			Vote::setVotedNumber(*(_ext), (u160)voteBeforeAccount, voteNumber_before - 1);
//			Vote::setVotedNumber(*(_ext), (u160)_to, voteNumber_after + 1);
//		}
//	}
//
//	//Set vote infomation finally.
//	Vote::setIsVoted(*(_ext), (u160)_from, _isVote);
//	Vote::setVotedTo(*(_ext), (u160)_from, _to);
//
//	return 1;
//}

//DEFINE_INTRINSIC_FUNCTION3(env, setDelegate, setDelegate, i32, i32, from, i32, to, i32, isDelegate)
//{
//	return 0;
//}
//
//DEFINE_INTRINSIC_FUNCTION1(env, getSender, getSender, none, i32, address)
//{
//	ExtVMFace* _ext = WASM_CORE::getExt();
//	auto mem = WASM_CORE::getMemory();
//	Address& _address = memoryRef<Address>(mem, address);
//
//	dev::Address _caller = _ext->caller;
//	dev::Address _create = _ext->origin;
//
//	if (_caller != _create)
//	{
//		_address = dev::Address(0);
//	}
//
//	_address = _caller;
//}



//our register functions by dz
DEFINE_INTRINSIC_FUNCTION0(env, gascount1, gascount1, none)
{
	std::cout << "gascount1" << std::endl;
	WASM_VM::AddUsedGas(1);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount2, gascount2, none)
{
	std::cout << "gascount2" << std::endl;
	WASM_VM::AddUsedGas(2);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount3, gascount3, none)
{
	std::cout << "gascount3" << std::endl;
	WASM_VM::AddUsedGas(3);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount4, gascount4, none)
{
	std::cout << "gascount4" << std::endl;
	WASM_VM::AddUsedGas(4);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount5, gascount5, none)
{
	std::cout << "gascount5" << std::endl;
	WASM_VM::AddUsedGas(5);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount6, gascount6, none)
{
	std::cout << "gascount6" << std::endl;
	WASM_VM::AddUsedGas(6);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount7, gascount7, none)
{
	std::cout << "gascount7" << std::endl;
	WASM_VM::AddUsedGas(7);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount8, gascount8, none)
{
	std::cout << "gascount8" << std::endl;
	WASM_VM::AddUsedGas(8);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount9, gascount9, none)
{
	std::cout << "gascount9" << std::endl;
	WASM_VM::AddUsedGas(9);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount10, gascount10, none)
{
	std::cout << "gascount10" << std::endl;
	WASM_VM::AddUsedGas(10);
}



DEFINE_INTRINSIC_FUNCTION0(env, checktime, checktime, none) {
	std::cout << "check time" << std::endl;
}

//
//#ifdef NDEBUG
//	const int CHECKTIME_LIMIT = 3000;
//#else
//	const int CHECKTIME_LIMIT = 18000;
//#endif
//
//	DEFINE_INTRINSIC_FUNCTION0(env, checktime, checktime, none) {
//		auto dur = wasm_interface::get().current_execution_time();
//		if (dur > CHECKTIME_LIMIT) {
//			wlog("checktime called ${d}", ("d", dur));
//			throw checktime_exceeded();
//		}
//	}
//	template <typename Function, typename KeyType, int numberOfKeys>
//	int32_t validate(int32_t valueptr, int32_t valuelen, Function func) {
//
//		static const uint32_t keylen = numberOfKeys * sizeof(KeyType);
//
//		FC_ASSERT(valuelen >= keylen, "insufficient data passed");
//
//		auto& wasm = wasm_interface::get();
//		FC_ASSERT(wasm.current_apply_context, "no apply context found");
//
//		char* value = memoryArrayPtr<char>(wasm.current_memory, valueptr, valuelen);
//		KeyType*  keys = reinterpret_cast<KeyType*>(value);
//
//		valuelen -= keylen;
//		value += keylen;
//
//		return func(wasm.current_apply_context, keys, value, valuelen);
//	}
//
//#define READ_RECORD(READFUNC, INDEX, SCOPE) \
//   auto lambda = [&](apply_context* ctx, INDEX::value_type::key_type* keys, char *data, uint32_t datalen) -> int32_t { \
//      auto res = ctx->READFUNC<INDEX, SCOPE>( Name(scope), Name(code), Name(table), keys, data, datalen); \
//      if (res >= 0) res += INDEX::value_type::number_of_keys*sizeof(INDEX::value_type::key_type); \
//      return res; \
//   }; \
//   return validate<decltype(lambda), INDEX::value_type::key_type, INDEX::value_type::number_of_keys>(valueptr, valuelen, lambda);
//
//#define UPDATE_RECORD(UPDATEFUNC, INDEX, DATASIZE) \
//   auto lambda = [&](apply_context* ctx, INDEX::value_type::key_type* keys, char *data, uint32_t datalen) -> int32_t { \
//      return ctx->UPDATEFUNC<INDEX::value_type>( Name(scope), Name(ctx->code.value), Name(table), keys, data, datalen); \
//   }; \
//   return validate<decltype(lambda), INDEX::value_type::key_type, INDEX::value_type::number_of_keys>(valueptr, DATASIZE, lambda);
//
//#define DEFINE_RECORD_UPDATE_FUNCTIONS(OBJTYPE, INDEX) \
//   DEFINE_INTRINSIC_FUNCTION4(env,store_##OBJTYPE,store_##OBJTYPE,i32,i64,scope,i64,table,i32,valueptr,i32,valuelen) { \
//      UPDATE_RECORD(store_record, INDEX, valuelen); \
//   } \
//   DEFINE_INTRINSIC_FUNCTION4(env,update_##OBJTYPE,update_##OBJTYPE,i32,i64,scope,i64,table,i32,valueptr,i32,valuelen) { \
//      UPDATE_RECORD(update_record, INDEX, valuelen); \
//   } \
//   DEFINE_INTRINSIC_FUNCTION3(env,remove_##OBJTYPE,remove_##OBJTYPE,i32,i64,scope,i64,table,i32,valueptr) { \
//      UPDATE_RECORD(remove_record, INDEX, sizeof(typename INDEX::value_type::key_type)*INDEX::value_type::number_of_keys); \
//   }
//
//#define DEFINE_RECORD_READ_FUNCTIONS(OBJTYPE, FUNCPREFIX, INDEX, SCOPE) \
//   DEFINE_INTRINSIC_FUNCTION5(env,load_##FUNCPREFIX##OBJTYPE,load_##FUNCPREFIX##OBJTYPE,i32,i64,scope,i64,code,i64,table,i32,valueptr,i32,valuelen) { \
//      READ_RECORD(load_record, INDEX, SCOPE); \
//   } \
//   DEFINE_INTRINSIC_FUNCTION5(env,front_##FUNCPREFIX##OBJTYPE,front_##FUNCPREFIX##OBJTYPE,i32,i64,scope,i64,code,i64,table,i32,valueptr,i32,valuelen) { \
//      READ_RECORD(front_record, INDEX, SCOPE); \
//   } \
//   DEFINE_INTRINSIC_FUNCTION5(env,back_##FUNCPREFIX##OBJTYPE,back_##FUNCPREFIX##OBJTYPE,i32,i64,scope,i64,code,i64,table,i32,valueptr,i32,valuelen) { \
//      READ_RECORD(back_record, INDEX, SCOPE); \
//   } \
//   DEFINE_INTRINSIC_FUNCTION5(env,next_##FUNCPREFIX##OBJTYPE,next_##FUNCPREFIX##OBJTYPE,i32,i64,scope,i64,code,i64,table,i32,valueptr,i32,valuelen) { \
//      READ_RECORD(next_record, INDEX, SCOPE); \
//   } \
//   DEFINE_INTRINSIC_FUNCTION5(env,previous_##FUNCPREFIX##OBJTYPE,previous_##FUNCPREFIX##OBJTYPE,i32,i64,scope,i64,code,i64,table,i32,valueptr,i32,valuelen) { \
//      READ_RECORD(previous_record, INDEX, SCOPE); \
//   } \
//   DEFINE_INTRINSIC_FUNCTION5(env,lower_bound_##FUNCPREFIX##OBJTYPE,lower_bound_##FUNCPREFIX##OBJTYPE,i32,i64,scope,i64,code,i64,table,i32,valueptr,i32,valuelen) { \
//      READ_RECORD(lower_bound_record, INDEX, SCOPE); \
//   } \
//   DEFINE_INTRINSIC_FUNCTION5(env,upper_bound_##FUNCPREFIX##OBJTYPE,upper_bound_##FUNCPREFIX##OBJTYPE,i32,i64,scope,i64,code,i64,table,i32,valueptr,i32,valuelen) { \
//      READ_RECORD(upper_bound_record, INDEX, SCOPE); \
//   }
//
//	DEFINE_RECORD_UPDATE_FUNCTIONS(i64, key_value_index);
//	DEFINE_RECORD_READ_FUNCTIONS(i64, , key_value_index, by_scope_primary);
//
//	DEFINE_RECORD_UPDATE_FUNCTIONS(i128i128, key128x128_value_index);
//	DEFINE_RECORD_READ_FUNCTIONS(i128i128, primary_, key128x128_value_index, by_scope_primary);
//	DEFINE_RECORD_READ_FUNCTIONS(i128i128, secondary_, key128x128_value_index, by_scope_secondary);
//
//	DEFINE_RECORD_UPDATE_FUNCTIONS(i64i64i64, key64x64x64_value_index);
//	DEFINE_RECORD_READ_FUNCTIONS(i64i64i64, primary_, key64x64x64_value_index, by_scope_primary);
//	DEFINE_RECORD_READ_FUNCTIONS(i64i64i64, secondary_, key64x64x64_value_index, by_scope_secondary);
//	DEFINE_RECORD_READ_FUNCTIONS(i64i64i64, tertiary_, key64x64x64_value_index, by_scope_tertiary);
//
//	DEFINE_INTRINSIC_FUNCTION3(env, assert_sha256, assert_sha256, none, i32, dataptr, i32, datalen, i32, hash) {
//		FC_ASSERT(datalen > 0);
//
//		auto& wasm = wasm_interface::get();
//		auto  mem = wasm.current_memory;
//
//		char* data = memoryArrayPtr<char>(mem, dataptr, datalen);
//		const auto& v = memoryRef<fc::sha256>(mem, hash);
//
//		auto result = fc::sha256::hash(data, datalen);
//		FC_ASSERT(result == v, "hash miss match");
//	}
//
//	DEFINE_INTRINSIC_FUNCTION3(env, sha256, sha256, none, i32, dataptr, i32, datalen, i32, hash) {
//		FC_ASSERT(datalen > 0);
//
//		auto& wasm = wasm_interface::get();
//		auto  mem = wasm.current_memory;
//
//		char* data = memoryArrayPtr<char>(mem, dataptr, datalen);
//		auto& v = memoryRef<fc::sha256>(mem, hash);
//		v = fc::sha256::hash(data, datalen);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, multeq_i128, multeq_i128, none, i32, self, i32, other) {
//		auto& wasm = wasm_interface::get();
//		auto  mem = wasm.current_memory;
//		auto& v = memoryRef<unsigned __int128>(mem, self);
//		const auto& o = memoryRef<const unsigned __int128>(mem, other);
//		v *= o;
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, diveq_i128, diveq_i128, none, i32, self, i32, other) {
//		auto& wasm = wasm_interface::get();
//		auto  mem = wasm.current_memory;
//		auto& v = memoryRef<unsigned __int128>(mem, self);
//		const auto& o = memoryRef<const unsigned __int128>(mem, other);
//		FC_ASSERT(o != 0, "divide by zero");
//		v /= o;
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, double_add, double_add, i64, i64, a, i64, b) {
//		DOUBLE c = DOUBLE(*reinterpret_cast<double *>(&a))
//			+ DOUBLE(*reinterpret_cast<double *>(&b));
//		double res = c.convert_to<double>();
//		return *reinterpret_cast<uint64_t *>(&res);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, double_mult, double_mult, i64, i64, a, i64, b) {
//		DOUBLE c = DOUBLE(*reinterpret_cast<double *>(&a))
//			* DOUBLE(*reinterpret_cast<double *>(&b));
//		double res = c.convert_to<double>();
//		return *reinterpret_cast<uint64_t *>(&res);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, double_div, double_div, i64, i64, a, i64, b) {
//		auto divisor = DOUBLE(*reinterpret_cast<double *>(&b));
//		FC_ASSERT(divisor != 0, "divide by zero");
//
//		DOUBLE c = DOUBLE(*reinterpret_cast<double *>(&a)) / divisor;
//		double res = c.convert_to<double>();
//		return *reinterpret_cast<uint64_t *>(&res);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, double_lt, double_lt, i32, i64, a, i64, b) {
//		return DOUBLE(*reinterpret_cast<double *>(&a))
//			< DOUBLE(*reinterpret_cast<double *>(&b));
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, double_eq, double_eq, i32, i64, a, i64, b) {
//		return DOUBLE(*reinterpret_cast<double *>(&a))
//			== DOUBLE(*reinterpret_cast<double *>(&b));
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, double_gt, double_gt, i32, i64, a, i64, b) {
//		return DOUBLE(*reinterpret_cast<double *>(&a))
//			> DOUBLE(*reinterpret_cast<double *>(&b));
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, double_to_i64, double_to_i64, i64, i64, a) {
//		return DOUBLE(*reinterpret_cast<double *>(&a))
//			.convert_to<uint64_t>();
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, i64_to_double, i64_to_double, i64, i64, a) {
//		double res = DOUBLE(a).convert_to<double>();
//		return *reinterpret_cast<uint64_t *>(&res);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION0(env, now, now, i32) {
//		return wasm_interface::get().current_validate_context->controller.head_block_time().sec_since_epoch();
//	}
//
//	DEFINE_INTRINSIC_FUNCTION0(env, currentCode, currentCode, i64) {
//		auto& wasm = wasm_interface::get();
//		return wasm.current_validate_context->code.value;
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, requireAuth, requireAuth, none, i64, account) {
//		wasm_interface::get().current_validate_context->require_authorization(Name(account));
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, requireNotice, requireNotice, none, i64, account) {
//		wasm_interface::get().current_apply_context->require_recipient(account);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, requireScope, requireScope, none, i64, scope) {
//		wasm_interface::get().current_validate_context->require_scope(scope);
//	}
//
DEFINE_INTRINSIC_FUNCTION3(env, memcpy, memcpy, i32, i32, dstp, i32, srcp, i32, len) {

	WASM_VM::AddUsedGas(2 * len);
	//auto& wasm = wasm_interface::get();
	//auto  mem = wasm.current_memory;
	//char* dst = memoryArrayPtr<char>(mem, dstp, len);
	//const char* src = memoryArrayPtr<const char>(mem, srcp, len);
	//FC_ASSERT(len > 0);

	//if (dst > src)
	//	FC_ASSERT(dst >= (src + len), "overlap of memory range is undefined", ("d", dstp)("s", srcp)("l", len));
	//else
	//	FC_ASSERT(src >= (dst + len), "overlap of memory range is undefined", ("d", dstp)("s", srcp)("l", len));

	//memcpy(dst, src, uint32_t(len));
	//return dstp;

	auto mem = WASM_CORE::getMemory();
	char* dst = memoryArrayPtr<char>(mem, dstp, len);
	const char* src = memoryArrayPtr<const char>(mem, srcp, len);
	if (len <= 0)
		BOOST_THROW_EXCEPTION(WASMWrongMemory());

	if (dst > src)
	{
		if (dst < (src + len))
		{
			BOOST_THROW_EXCEPTION(WASMWrongMemory());
		}
	}
	else {
		if (src < (dst + len))
		{
			BOOST_THROW_EXCEPTION(WASMWrongMemory());
		}
	}


	memcpy(dst, src, uint32_t(len));
	return dstp;
}

//DEFINE_INTRINSIC_FUNCTION3(env, memset, memset, i32, i32, rel_ptr, i32, value, i32, len) {
//	auto& wasm = wasm_interface::get();
//	auto  mem = wasm.current_memory;
//	char* ptr = memoryArrayPtr<char>(mem, rel_ptr, len);
//	FC_ASSERT(len > 0);

//	memset(ptr, value, len);
//	return rel_ptr;
//}
//
//
//	/**
//	* Transaction C API implementation
//	* @{
//	*/
//
//	DEFINE_INTRINSIC_FUNCTION0(env, transactionCreate, transactionCreate, i32) {
//		auto& ptrx = wasm_interface::get().current_apply_context->create_pending_transaction();
//		return ptrx.handle;
//	}
//
//	static void emplace_scope(const Name& scope, std::vector<Name>& scopes) {
//		auto i = std::upper_bound(scopes.begin(), scopes.end(), scope);
//		if (i == scopes.begin() || *(i - 1) != scope) {
//			scopes.insert(i, scope);
//		}
//	}
//
//	DEFINE_INTRINSIC_FUNCTION3(env, transactionRequireScope, transactionRequireScope, none, i32, handle, i64, scope, i32, readOnly) {
//		auto& ptrx = wasm_interface::get().current_apply_context->get_pending_transaction(handle);
//		if (readOnly == 0) {
//			emplace_scope(scope, ptrx.scope);
//		}
//		else {
//			emplace_scope(scope, ptrx.readscope);
//		}
//
//		ptrx.check_size();
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, transactionAddMessage, transactionAddMessage, none, i32, handle, i32, msg_handle) {
//		auto apply_context = wasm_interface::get().current_apply_context;
//		auto& ptrx = apply_context->get_pending_transaction(handle);
//		auto& pmsg = apply_context->get_pending_message(msg_handle);
//		ptrx.messages.emplace_back(pmsg);
//		ptrx.check_size();
//		apply_context->release_pending_message(msg_handle);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, transactionSend, transactionSend, none, i32, handle) {
//		auto apply_context = wasm_interface::get().current_apply_context;
//		auto& ptrx = apply_context->get_pending_transaction(handle);
//
//		EOS_ASSERT(ptrx.messages.size() > 0, tx_unknown_argument,
//			"Attempting to send a transaction with no messages");
//
//		apply_context->deferred_transactions.emplace_back(ptrx);
//		apply_context->release_pending_transaction(handle);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, transactionDrop, transactionDrop, none, i32, handle) {
//		wasm_interface::get().current_apply_context->release_pending_transaction(handle);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION4(env, messageCreate, messageCreate, i32, i64, code, i64, type, i32, data, i32, length) {
//		auto& wasm = wasm_interface::get();
//		auto  mem = wasm.current_memory;
//
//		EOS_ASSERT(length >= 0, tx_unknown_argument,
//			"Pushing a message with a negative length");
//
//		Bytes payload;
//		if (length > 0) {
//			try {
//				// memoryArrayPtr checks that the entire array of bytes is valid and
//				// within the bounds of the memory segment so that transactions cannot pass
//				// bad values in attempts to read improper memory
//				const char* buffer = memoryArrayPtr<const char>(mem, uint32_t(data), uint32_t(length));
//				payload.insert(payload.end(), buffer, buffer + length);
//			}
//			catch (const Runtime::Exception& e) {
//				FC_THROW_EXCEPTION(tx_unknown_argument, "Message data is not a valid memory range");
//			}
//		}
//
//		auto& pmsg = wasm.current_apply_context->create_pending_message(Name(code), Name(type), payload);
//		return pmsg.handle;
//	}
//
//	DEFINE_INTRINSIC_FUNCTION3(env, messageRequirePermission, messageRequirePermission, none, i32, handle, i64, account, i64, permission) {
//		auto apply_context = wasm_interface::get().current_apply_context;
//		// if this is not sent from the code account with the permission of "code" then we must
//		// presently have the permission to add it, otherwise its a failure
//		if (!(account == apply_context->code.value && Name(permission) == Name("code"))) {
//			apply_context->require_authorization(Name(account), Name(permission));
//		}
//		auto& pmsg = apply_context->get_pending_message(handle);
//		pmsg.authorization.emplace_back(Name(account), Name(permission));
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, messageSend, messageSend, none, i32, handle) {
//		auto apply_context = wasm_interface::get().current_apply_context;
//		auto& pmsg = apply_context->get_pending_message(handle);
//
//		apply_context->inline_messages.emplace_back(pmsg);
//		apply_context->release_pending_message(handle);
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, messageDrop, messageDrop, none, i32, handle) {
//		wasm_interface::get().current_apply_context->release_pending_message(handle);
//	}
//
//	/**
//	* @} Transaction C API implementation
//	*/
//
//
//
//	DEFINE_INTRINSIC_FUNCTION2(env, readMessage, readMessage, i32, i32, destptr, i32, destsize) {
//		FC_ASSERT(destsize > 0);
//
//		wasm_interface& wasm = wasm_interface::get();
//		auto  mem = wasm.current_memory;
//		char* begin = memoryArrayPtr<char>(mem, destptr, uint32_t(destsize));
//
//		int minlen = std::min<int>(wasm.current_validate_context->msg.data.size(), destsize);
//
//		//   wdump((destsize)(wasm.current_validate_context->msg.data.size()));
//		memcpy(begin, wasm.current_validate_context->msg.data.data(), minlen);
//		return minlen;
//	}
//
//	DEFINE_INTRINSIC_FUNCTION2(env, assert, assert, none, i32, test, i32, msg) {
//		const char* m = &Runtime::memoryRef<char>(wasm_interface::get().current_memory, msg);
//		std::string message(m);
//		if (!test) edump((message));
//		FC_ASSERT(test, "assertion failed: ${s}", ("s", message)("ptr", msg));
//	}
//
//	DEFINE_INTRINSIC_FUNCTION0(env, messageSize, messageSize, i32) {
//		return wasm_interface::get().current_validate_context->msg.data.size();
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, malloc, malloc, i32, i32, size) {
//		FC_ASSERT(size > 0);
//		int32_t& end = Runtime::memoryRef<int32_t>(Runtime::getDefaultMemory(wasm_interface::get().current_module), 0);
//		int32_t old_end = end;
//		end += 8 * ((size + 7) / 8);
//		FC_ASSERT(end > old_end);
//		return old_end;
//	}
//
DEFINE_INTRINSIC_FUNCTION1(env, printi, printi, none, i64, val) {
	std::cerr << uint64_t(val);
}
//	DEFINE_INTRINSIC_FUNCTION1(env, printd, printd, none, i64, val) {
//		std::cerr << DOUBLE(*reinterpret_cast<double *>(&val));
//	}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, printi128, printi128, none, i32, val) {
//		auto& wasm = wasm_interface::get();
//		auto  mem = wasm.current_memory;
//		auto& value = memoryRef<unsigned __int128>(mem, val);
//		fc::uint128_t v(value >> 64, uint64_t(value));
//		std::cerr << fc::variant(v).get_string();
//	}
//	DEFINE_INTRINSIC_FUNCTION1(env, printn, printn, none, i64, val) {
//		std::cerr << Name(val).toString();
//	}
//
DEFINE_INTRINSIC_FUNCTION1(env, prints, prints, none, i32, charptr) {
	//auto& wasm = wasm_interface::get();
	//auto  mem = wasm.current_memory;

	//const char* str = &memoryRef<const char>(mem, charptr);

	//std::cerr << std::string(str, strnlen(str, wasm.current_state->mem_end - charptr));

	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	const char* str = &memoryRef<const char>(mem, charptr);
	std::cerr << std::string(str, strnlen(str, WASM_CORE::getMemoryEnd() - charptr));
}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, free, free, none, i32, ptr) {
//	}
//
//

//////////

#define CT256Size 40

DEFINE_INTRINSIC_FUNCTION3(env, sha3, sha3, none, i32, sourcePtr, i32, length, i32, dest)
{
	WASM_VM::AddUsedGas(sha3Gas + sha3WordGas*(length + 31) / 32);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	byte* str = memoryArrayPtr<byte>(mem, sourcePtr, length);
	u256& destination = memoryRef<u256>(mem, dest);
	bytesConstRef bcr(str, length);
	u256 result = sha3(bcr);
	memcpy(&destination, &result, CT256Size);
}



DEFINE_INTRINSIC_FUNCTION1(env, address, address, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = fromAddress(_ext->myAddress);
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, caller, caller, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = fromAddress(_ext->caller);
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION2(env, balance, balance, none, i32, address, i32, dest)
{
	WASM_VM::AddUsedGas(balanceGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = _ext->balance(asAddress(memoryRef<u256>(mem, address)));
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION2(env, blockhash, blockhash, none, i32, number, i32, dest)
{
	WASM_VM::AddUsedGas(blockhashGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = _ext->blockHash(memoryRef<u256>(mem, number));
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, coinbase, coinbase, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = fromAddress(_ext->envInfo().author());
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);

}

DEFINE_INTRINSIC_FUNCTION1(env, timestamp, timestamp, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = _ext->envInfo().timestamp();
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, number, number, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = _ext->envInfo().number();
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, difficulty, difficulty, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = _ext->envInfo().difficulty();
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, gaslimit, gaslimit, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = _ext->envInfo().gasLimit();
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, gasprice, gasprice, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = _ext->gasPrice;
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, printu256, printu256, none, i32, source)
{
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256& result = memoryRef<u256>(mem, source);
	std::cout << "DEC: " << result.str() << std::endl;
	//std::cout << "Hex: "<<toHex(result) << std::endl;
}

DEFINE_INTRINSIC_FUNCTION2(env, getstore, getstore, none, i32, location, i32, data)
{
	WASM_VM::AddUsedGas(sloadGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256& address = memoryRef<u256>(mem, location);
	u256 result = _ext->store(address);
	u256& destination = memoryRef<u256>(mem, data);
	memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION2(env, setstore, setstore, none, i32, location, i32, data)
{
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256& address = memoryRef<u256>(mem, location);
	u256& savedata = memoryRef<u256>(mem, data);


	if (!_ext->store(address) && savedata)
		WASM_VM::AddUsedGas(sstoreSetGas);
	else if (_ext->store(address) && !savedata)
	{
		WASM_VM::AddUsedGas(sstoreResetGas);
		_ext->sub.refunds += sstoreRefundGas;
	}
	else
		WASM_VM::AddUsedGas(sstoreResetGas);


	_ext->setStore(address, savedata);
}


DEFINE_INTRINSIC_FUNCTION2(env, transferbalance, transferbalance, none, i32, address, i32, data)
{
	WASM_VM::AddUsedGas(callGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256& balance = memoryRef<u256>(mem, data);
	_ext->transferBalance(asAddress(memoryRef<u256>(mem, address)), balance);
}


DEFINE_INTRINSIC_FUNCTION1(env, callvalue, callvalue, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	u256 result = _ext->value;
	u256& destination = memoryRef<u256>(mem, dest);
	memcpy(&destination, &result, CT256Size);
}


DEFINE_INTRINSIC_FUNCTION2(env, setu256value, setu256value, none, i32, u256address, i32, charptr)
{
	WASM_VM::AddUsedGas(u256op1);
	ExtVMFace* _ext = WASM_CORE::getExt();
	auto mem = WASM_CORE::getMemory();
	const char* str = &memoryRef<const char>(mem, charptr);
	u256& destination = memoryRef<u256>(mem, u256address);
	std::string numberStr(str, strnlen(str, WASM_CORE::getMemoryEnd() - charptr));
	u256 number(numberStr);
	//destination.assign(numberStr);
	destination = number;
}


#define U256OperatorDelarationAndImp(name,operator,usedGas)\
DEFINE_INTRINSIC_FUNCTION3(env,name, name, none, i32, operand_1, i32, operand_2, i32, result)\
{\
	WASM_VM::AddUsedGas(usedGas);\
	auto mem = WASM_CORE::getMemory();\
	u256& op1 = memoryRef<u256>(mem, operand_1);\
	u256& op2 = memoryRef<u256>(mem, operand_2);\
	u256& v = memoryRef<u256>(mem, result);\
	v = op1 operator op2;\
}\

U256OperatorDelarationAndImp(add_u256, +, u256op1)
U256OperatorDelarationAndImp(sub_u256, -, u256op1)
U256OperatorDelarationAndImp(mul_u256, *, u256op2)
U256OperatorDelarationAndImp(div_u256, / , u256op2)
U256OperatorDelarationAndImp(mod_u256, %, u256op2)


#define U256ComparatorDelarationAndImp(name,operator)\
DEFINE_INTRINSIC_FUNCTION2(env,name, name, i32, i32, operand_1, i32, operand_2)\
{\
	WASM_VM::AddUsedGas(u256op1);\
	auto mem = WASM_CORE::getMemory();\
	u256& op1 = memoryRef<u256>(mem, operand_1);\
	u256& op2 = memoryRef<u256>(mem, operand_2);\
	return op1 operator op2;\
}\

U256ComparatorDelarationAndImp(lt_u256, <)
	U256ComparatorDelarationAndImp(gt_u256, >)
	U256ComparatorDelarationAndImp(eq_u256, == )




	DEFINE_INTRINSIC_FUNCTION2(env, WAVMAssert, WAVMAssert, none, i32, test, i32, msg) {
	if (test == 0)
	{
		auto mem = WASM_CORE::getMemory();
		const char* str = &memoryRef<const char>(mem, msg);
		std::cout << std::string(str, strnlen(str, WASM_CORE::getMemoryEnd() - msg)) << std::endl;
		BOOST_THROW_EXCEPTION(WASMAssertFailed());
	}
}

DEFINE_INTRINSIC_FUNCTION1(env, test1, test1, none, i32, a) {
	std::cout << "test1 " << "a: " << a << std::endl;
}

DEFINE_INTRINSIC_FUNCTION2(env, test2, test2, none, i32, a, i32, b) {
	std::cout << "test2 " << "a: " << a << " b:" << b << std::endl;
}
