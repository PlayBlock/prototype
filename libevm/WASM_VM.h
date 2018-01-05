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

#include "VMFace.h"

using namespace dev;
using namespace eth;

class WASM_CORE;

ETH_SIMPLE_EXCEPTION_VM(WASMOutOfGas);
ETH_SIMPLE_EXCEPTION_VM(WASMWrongMemory);
ETH_SIMPLE_EXCEPTION_VM(WASMParameterLengthTooSmall);
ETH_SIMPLE_EXCEPTION_VM(WASMAssertFailed);


//by dz end
class WASM_VM : public VMFace
{
public:
	//virtual bytesConstRef exec(u256& io_gas, ExtVMFace& _ext, OnOpFunc const& _onOp, bool isCreation) override final;
	virtual owning_bytes_ref exec(u256& io_gas, ExtVMFace& _ext, OnOpFunc const& _onOp, bool isCreation = false) override final;

	//by dz
	static void AddUsedGas(u256 useGase)
	{
		if (*m_io_gas < useGase)
			throw WASMOutOfGas();
		*m_io_gas -= useGase;
	}
	//by dz end

private:
	EVMSchedule const* m_schedule = nullptr;
	bytesConstRef m_bytes = bytesConstRef();
	//bytes m_return_r;
	static WASM_CORE* m_core;

	//by dz
	static u256* m_io_gas;
	//by dz end
};

using namespace IR;
using namespace Runtime;

class WASM_CORE
{
private:
	static Runtime::MemoryInstance* current_memory;
	static ExtVMFace* current_ext;
	static bytes current_parameter;
	static int mem_end;

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

	static void setMemoryEnd(int end) { mem_end = end; };

	static int getMemoryEnd() { return mem_end; };
};


