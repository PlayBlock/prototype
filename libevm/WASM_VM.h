#pragma once

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

#include <map>
#include <vector>
#include "VMFace.h"

namespace dev
{
namespace eth
{

	class WASM_CORE;

	ETH_SIMPLE_EXCEPTION_VM(WASMOutOfGas);
	ETH_SIMPLE_EXCEPTION_VM(WASMWrongMemory);
	ETH_SIMPLE_EXCEPTION_VM(WASMParameterLengthTooSmall);
	ETH_SIMPLE_EXCEPTION_VM(WASMAssertFailed);
	ETH_SIMPLE_EXCEPTION_VM(WASMTimeExceed);

	const U32 CT256Size = sizeof(h256);
	const U32 AddressSize = sizeof(Address);
	
	
	struct ModuleState {
		Runtime::ModuleInstance* instance = nullptr;
		IR::Module*              module = nullptr;
		int                      mem_start = 0;
		int                      mem_end = 1 << 16;
		std::vector<char>             init_memory;
		//fc::sha256               code_version;
		//TableMap                 table_key_types;
		//bool                     tables_fixed = false;
	};



	class wasm_memory;

	class WASM_CORE
	{
	public:
		WASM_CORE();
		~WASM_CORE();

		void init();
		void destory();

		bytes run(const std::string& string, const char* functionName, uint64_t applyFunction, const bytes& args, ExtVMFace& _ext, bool isCreation);

		static Runtime::MemoryInstance* getMemory() { return current_memory; }

		static ExtVMFace* getExt() { return current_ext; }

		static bytes getParameter() { return current_parameter; }

		static u256 u256_temp;
		//static void setMemoryEnd(int end) { mem_end = end; };

		//static int getMemoryEnd() { return mem_end; };

		static void ResetTime() {
			execute_time.restart();
		}
		static bool IsExecuteExceed()
		{
			return false;
			//return execute_time.duration().count() > 1000000000;
		}

		static ModuleState *current_state;
		static wasm_memory *current_memory_management;

		static Runtime::ModuleInstance *current_module;
		static Runtime::MemoryInstance *current_memory;
		static std::map<Address, ModuleState> module_cache;
	private:

		static void LoadModuleState(Address const& _address, const std::string& _string);

		//static Runtime::MemoryInstance* current_memory;
		static ExtVMFace* current_ext;
		static bytes current_parameter;
		//static int mem_end;
		static Timer execute_time;


	};


	class wasm_memory
	{
	public:
		explicit wasm_memory(WASM_CORE& core);
		wasm_memory(const wasm_memory&) = delete;
		wasm_memory(wasm_memory&&) = delete;
		~wasm_memory();
		U32 sbrk(I32 num_bytes);
	private:
		static U32 limit_32bit_address(Uptr address);

		static const U32 _max_memory = 1024 * 1024;
		WASM_CORE& _wasm_core;
		Uptr _num_pages;
		const U32 _min_bytes;
		U32 _num_bytes;
	};
	
	
	
	//by dz end
	class WASM_VM : public VMFace
	{
	public:
		//virtual bytesConstRef exec(u256& io_gas, ExtVMFace& _ext, OnOpFunc const& _onOp, bool isCreation) override final;
		virtual owning_bytes_ref exec(u256& io_gas, ExtVMFace& _ext, OnOpFunc const& _onOp, bool isCreation = false) override final;
		virtual void clearCodeCache(Address const& _address)override
		{
			if (WASM_CORE::module_cache.count(_address))
			{
				auto const& moduleState = WASM_CORE::module_cache[_address];
				if (moduleState.module)
				{
					delete moduleState.module;
				}
				//if (moduleState.instance)
				//{
				//	delete moduleState.instance;
				//}
				WASM_CORE::module_cache.erase(_address);
			}
		}

		static void AddUsedGas(u256 useGase)
		{
			if (*m_io_gas < useGase)
				throw WASMOutOfGas();
			*m_io_gas -= useGase;
		}
		static void ClearAllCacheCode();


	private:
		bytesConstRef m_bytes = bytesConstRef();
		//bytes m_return_r;
		static WASM_CORE* m_core;

		//by dz
		static u256* m_io_gas;
		//by dz end
	};




}
}

