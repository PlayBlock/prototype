
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
#include "../eth/Register.h"

using namespace dev;
using namespace eth;
using namespace IR;
using namespace Runtime;
using namespace std;

DEFINE_INTRINSIC_FUNCTION2(env, readMessage, readMessage, i32, i32, destptr, i32, destsize) {
	WASM_VM::AddUsedGas(basicGas);
	if (destsize <= 0)
	{
		BOOST_THROW_EXCEPTION(WASMWrongMemory());
	}
	WASM_CORE* instance = WASM_CORE::getInstance();
	bytes para = instance->getParameter();
	auto mem = instance->getMemory();
	char* begin = memoryArrayPtr<char>(mem, destptr, uint32_t(destsize));
	int minlen = std::min<int>(para.size(), destsize);
	memcpy(begin, para.data(), minlen);
	return minlen;
}


DEFINE_INTRINSIC_FUNCTION0(env, gascount1, gascount1, none)
{
	//ctrace << "gascount1";
	WASM_VM::AddUsedGas(1);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount2, gascount2, none)
{
	//ctrace << "gascount2";
	WASM_VM::AddUsedGas(2);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount3, gascount3, none)
{
	//ctrace << "gascount3";
	WASM_VM::AddUsedGas(3);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount4, gascount4, none)
{
	//ctrace << "gascount4";
	WASM_VM::AddUsedGas(4);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount5, gascount5, none)
{
	//ctrace << "gascount5";
	WASM_VM::AddUsedGas(5);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount6, gascount6, none)
{
	//ctrace << "gascount6";
	WASM_VM::AddUsedGas(6);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount7, gascount7, none)
{
	//ctrace << "gascount7";
	WASM_VM::AddUsedGas(7);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount8, gascount8, none)
{
	//ctrace << "gascount8";
	WASM_VM::AddUsedGas(8);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount9, gascount9, none)
{
	//ctrace << "gascount9";
	WASM_VM::AddUsedGas(9);
}

DEFINE_INTRINSIC_FUNCTION0(env, gascount10, gascount10, none)
{
	//ctrace << "gascount10";
	WASM_VM::AddUsedGas(10);
}



DEFINE_INTRINSIC_FUNCTION0(env, checktime, checktime, none) {
	//ctrace << "check time" ;
	if (WASM_CORE::IsExecuteExceed())
	{
		BOOST_THROW_EXCEPTION(WASMTimeExceed());
	}
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

	auto mem = WASM_CORE::getInstance()->getMemory();
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

DEFINE_INTRINSIC_FUNCTION3(env, memset, memset, i32, i32, rel_ptr, i32, value, i32, len) {
	WASM_VM::AddUsedGas(2 * len);
	auto mem = WASM_CORE::getInstance()->getMemory();
	char* ptr = memoryArrayPtr<char>(mem, rel_ptr, len);
	memset(ptr, value, len);
	return rel_ptr;
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
	WASM_VM::AddUsedGas(basicGas);
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
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	const char* str = &memoryRef<const char>(mem, charptr);
	std::cerr << std::string(str, strnlen(str, instance->current_state->mem_end - charptr));
}
//
//	DEFINE_INTRINSIC_FUNCTION1(env, free, free, none, i32, ptr) {
//	}
//
//


DEFINE_INTRINSIC_FUNCTION3(env, sha3, sha3, none, i32, sourcePtr, i32, length, i32, dest)
{
	WASM_VM::AddUsedGas(sha3Gas + sha3WordGas*(length + 31) / 32);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	byte* str = memoryArrayPtr<byte>(mem, sourcePtr, length);
	h256& destination = memoryRef<h256>(mem, dest);
	bytesConstRef bcr(str, length);
	h256 result = sha3(bcr);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}



DEFINE_INTRINSIC_FUNCTION1(env, contractaddress, contractaddress, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	Address result = _ext->myAddress;
	Address& destination = memoryRef<Address>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, AddressSize);
}

DEFINE_INTRINSIC_FUNCTION1(env, caller, caller, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	Address result = _ext->caller;
	Address& destination = memoryRef<Address>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION2(env, balance, balance, none, i32, address, i32, dest)
{
	WASM_VM::AddUsedGas(balanceGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	h256 result = _ext->balance(memoryRef<Address>(mem, address));
	h256& destination = memoryRef<h256>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION2(env, blockhash, blockhash, none, i32, number, i32, dest)
{
	WASM_VM::AddUsedGas(blockhashGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	h256 result = _ext->blockHash(memoryRef<h256>(mem, number));
	h256& destination = memoryRef<h256>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, coinbase, coinbase, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	Address result = _ext->envInfo().author();
	Address& destination = memoryRef<Address>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, timestamp, timestamp, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	h256 result = _ext->envInfo().prevTimestamp();
	h256& destination = memoryRef<h256>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, number, number, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	h256 result = _ext->envInfo().number();
	h256& destination = memoryRef<h256>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, getcallvalue, getcallvalue, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	h256 result = _ext->value;
	h256& destination = memoryRef<h256>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}


DEFINE_INTRINSIC_FUNCTION1(env, gaslimit, gaslimit, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	h256 result = _ext->envInfo().gasLimit();
	h256& destination = memoryRef<h256>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, gasprice, gasprice, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	h256 result = _ext->gasPrice;
	h256& destination = memoryRef<h256>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION1(env, printu256, printu256, none, i32, source)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	u256 result = memoryRef<h256>(mem, source);
	std::cout << "DEC: " << result.str() << std::endl;
	//ctrace << "Hex: "<<toHex(result) ;
}


DEFINE_INTRINSIC_FUNCTION1(env, printaddress, printaddress, none, i32, source)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	Address& result = memoryRef<Address>(mem, source);
	ctrace << "Address: " << result.hex();
	//ctrace << "Hex: "<<toHex(result);
}

DEFINE_INTRINSIC_FUNCTION2(env, getstore, getstore, none, i32, location, i32, data)
{
	WASM_VM::AddUsedGas(sloadGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	u256 address = memoryRef<h256>(mem, location);
	h256 result = _ext->store(address);
	h256& destination = memoryRef<h256>(mem, data);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}

DEFINE_INTRINSIC_FUNCTION2(env, setstore, setstore, none, i32, location, i32, data)
{
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	u256 address = memoryRef<h256>(mem, location);
	u256 savedata = memoryRef<h256>(mem, data);

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
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	u256 balance = memoryRef<h256>(mem, data);

	Address tmp = memoryRef<Address>(mem, address);
	ctrace << "address: " << memoryRef<Address>(mem, address).hex();
	ctrace << "balance: " << balance.str();
	_ext->transferBalance(memoryRef<Address>(mem, address), balance);
}


DEFINE_INTRINSIC_FUNCTION1(env, callvalue, callvalue, none, i32, dest)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	h256 result = _ext->value;
	h256& destination = memoryRef<h256>(mem, dest);
	destination = result;
	//memcpy(&destination, &result, CT256Size);
}


DEFINE_INTRINSIC_FUNCTION2(env, setu256value, setu256value, none, i32, u256address, i32, charptr)
{
	WASM_VM::AddUsedGas(u256op1);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	const char* str = &memoryRef<const char>(mem, charptr);
	h256& destination = memoryRef<h256>(mem, u256address);
	std::string numberStr(str, strnlen(str, 66));
	u256 number(numberStr);
	//destination.assign(numberStr);
	destination = number;
}

DEFINE_INTRINSIC_FUNCTION2(env, setaddressvalue, setaddressvalue, none, i32, addressptr, i32, charptr)
{
	WASM_VM::AddUsedGas(u256op1);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	const char* str = &memoryRef<const char>(mem, charptr);
	Address& destination = memoryRef<Address>(mem, addressptr);
	std::string numberStr(str, strnlen(str, 66));

	Address address(numberStr);
	//destination.assign(numberStr);
	destination = address;
}

DEFINE_INTRINSIC_FUNCTION2(env, u256toaddress, u256toaddress, none, i32, input, i32, result)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();

	h256& value = memoryRef<h256>(mem, input);
	Address& destination = memoryRef<Address>(mem, result);

	memcpy(&destination, &value, sizeof(Address));
}

DEFINE_INTRINSIC_FUNCTION2(env, addresstou256, addresstou256, none, i32, input, i32, result)
{
	WASM_VM::AddUsedGas(basicGas);
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();

	Address& value = memoryRef<Address>(mem, input);
	h256& destination = memoryRef<h256>(mem, result);
	destination = h256(0);
	memcpy(&destination, &value, sizeof(Address));
}



#define U256OperatorDelarationAndImp(name,operator,usedGas)\
DEFINE_INTRINSIC_FUNCTION3(env,name, name, none, i32, operand_1, i32, operand_2, i32, result)\
{\
	WASM_VM::AddUsedGas(usedGas);\
	WASM_CORE* instance = WASM_CORE::getInstance();\
	auto mem = instance->getMemory();\
	u256 op1 = memoryRef<h256>(mem, operand_1);\
	u256 op2 = memoryRef<h256>(mem, operand_2);\
	h256& v = memoryRef<h256>(mem, result);\
	v = op1 operator op2;\
}\

#define U256OperatorDivDelarationAndImp(name,operator,usedGas)\
DEFINE_INTRINSIC_FUNCTION3(env,name, name, none, i32, operand_1, i32, operand_2, i32, result)\
{\
	WASM_VM::AddUsedGas(usedGas);\
	WASM_CORE* instance = WASM_CORE::getInstance();\
	auto mem = instance->getMemory();\
	u256 op1 = memoryRef<h256>(mem, operand_1);\
	u256 op2 = memoryRef<h256>(mem, operand_2);\
	if(op2 == u256(0))\
		BOOST_THROW_EXCEPTION(BadInstruction()); \
	h256& v = memoryRef<h256>(mem, result);\
	v = op1 operator op2;\
}\


U256OperatorDelarationAndImp(add_u256, +, u256op1)
U256OperatorDelarationAndImp(sub_u256, -, u256op1)
U256OperatorDelarationAndImp(mul_u256, *, u256op2)
U256OperatorDivDelarationAndImp(div_u256, / , u256op2)
U256OperatorDivDelarationAndImp(mod_u256, %, u256op2)


#define U256ComparatorDelarationAndImp(name,operator)\
DEFINE_INTRINSIC_FUNCTION2(env,name, name, i32, i32, operand_1, i32, operand_2)\
{\
	WASM_VM::AddUsedGas(u256op1);\
	WASM_CORE* instance = WASM_CORE::getInstance();\
	auto mem = instance->getMemory();\
	u256 op1 = memoryRef<h256>(mem, operand_1);\
	u256 op2 = memoryRef<h256>(mem, operand_2);\
	return op1 operator op2;\
}\

U256ComparatorDelarationAndImp(lt_u256, <)
	U256ComparatorDelarationAndImp(gt_u256, >)
	U256ComparatorDelarationAndImp(eq_u256, == )



	DEFINE_INTRINSIC_FUNCTION2(env, WAVMAssert, WAVMAssert, none, i32, test, i32, msg)
{
	WASM_VM::AddUsedGas(basicGas);
	if (test == 0)
	{
		WASM_CORE* instance = WASM_CORE::getInstance();
		auto mem = instance->getMemory();
		const char* str = &memoryRef<const char>(mem, msg);
		ctrace << std::string(str, strnlen(str, instance->current_state->mem_end - msg));
		BOOST_THROW_EXCEPTION(WASMAssertFailed());
	}
}
//´ýÉ¾³ý
DEFINE_INTRINSIC_FUNCTION1(env, test1, test1, none, i32, a) {
	WASM_VM::AddUsedGas(sstoreSetGas);
	ctrace << "test1 " << "a: " << a;
	auto mem = WASM_CORE::getInstance()->getMemory();
}
//´ýÉ¾³ý
DEFINE_INTRINSIC_FUNCTION2(env, test2, test2, none, i32, a, i32, b) {
	WASM_VM::AddUsedGas(sstoreSetGas);
	ctrace << "test2 " << "a: " << a << " b:" << b;
}
#define TOTAL_PAGES 1024 
#define GLOBAL_ENV_VAR_OFFSET 4
DEFINE_INTRINSIC_FUNCTION1(env, malloc, malloc, i32, i32, size) {
	if (size <= 0)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}  
	
	auto mem = WASM_CORE::getInstance()->getMemory();

	int32_t pageSize = 1<<Platform::getPageSizeLog2();

	if (getMemoryNumPages(mem) < TOTAL_PAGES)
	{
		growMemory(mem, TOTAL_PAGES - getMemoryNumPages(mem));
	}

	int32_t& end = memoryRef<int32_t>(mem, TOTAL_PAGES * pageSize - GLOBAL_ENV_VAR_OFFSET);
	int32_t old_end = end;
	end += 8 * ((size + 7) / 8);
	if (end <= old_end)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}
	if (TOTAL_PAGES * pageSize - GLOBAL_ENV_VAR_OFFSET - end < 0)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}
	return TOTAL_PAGES * pageSize - GLOBAL_ENV_VAR_OFFSET - end;
}

DEFINE_INTRINSIC_FUNCTION1(env, free, free, none, i32, ptr) {
	ctrace << "####free#####";
}

DEFINE_INTRINSIC_FUNCTION2(env, WAVMReturn, WAVMReturn, none, i32, sourcePtr, i32, length)
{
	WASM_VM::AddUsedGas(sstoreSetGas); //gasÖµÓÐ´ýÈ·¶¨
	WASM_CORE* instance = WASM_CORE::getInstance();
	ExtVMFace* _ext = instance->getExt();
	auto mem = instance->getMemory();
	byte* str = memoryArrayPtr<byte>(mem, sourcePtr, length);
	bytesConstRef bcr(str, length);
	bytes ss = bcr.toBytes();
	instance->getReturn() = ss;
}
