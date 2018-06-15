
#include "WASM_VM.h"
#include <libethereum/ExtVM.h>
#include <iostream>

using namespace IR;
using namespace Runtime;
using namespace std;

using namespace dev;
using namespace eth;


WASM_CORE* WASM_CORE::s_core = nullptr;
u256* WASM_VM::m_io_gas = nullptr;


void strMemCopy(std::string)
{
	Address add;
	size_t len = sizeof(add);
	bytes data(len, 0);
	memcpy(data.data(), &add, len);
}


owning_bytes_ref WASM_VM::exec(u256& io_gas, ExtVMFace& _ext, OnOpFunc const& _onOp, bool isCreation)
{
	Log::setCategoryEnabled(Log::Category::debug, false);
	m_io_gas = &io_gas;
	bytesConstRef data = _ext.data;
	//ctrace << "data:" << (data.count() ? (const char*)(data.data()) : "");

	bytes retbytes;
	WASM_CORE* core = WASM_CORE::getInstance();

	std::string string_temp((_ext).code.begin(), (_ext).code.end());
	bytes _totalBytes((_ext).data.begin(), (_ext).data.end());
	//by hjx 20171213
	if (isCreation)
	{
		//Timer _time;
		retbytes = core->run(string_temp, "init", 0, bytes(), _ext, true);
		//std::cout << "exetime:" << _time.elapsed() << std::endl;
		return owning_bytes_ref{ std::move(_ext.code), 0, _ext.code.size() };
	}
	else
	{
		if (_totalBytes.size() == 0)
			retbytes = core->run(string_temp, "_default", 0, bytes(), _ext, false);
		else if (_totalBytes.size() >= 8)
		{
			bytes applyFunction(_totalBytes.begin(), _totalBytes.begin() + 8);
			uint64_t applyFunctionUint;
			memcpy(&applyFunctionUint, applyFunction.data(), sizeof(applyFunctionUint));
			bytes paraString_(_totalBytes.begin() + 8, _totalBytes.end());

			bytes logs = asBytes("abc logs");
			_ext.log({}, bytesConstRef(logs.data(), logs.size()));

			retbytes = core->run(string_temp, "apply", applyFunctionUint, paraString_, _ext, false);
		}
		else
			BOOST_THROW_EXCEPTION(WASMParameterLengthTooSmall());
		return owning_bytes_ref{ std::move(retbytes), 0, retbytes.size() };
	}

}


void WASM_VM::clearCodeCache(Address const& _address)
{
	WASM_CORE* core = WASM_CORE::getInstance();
	
	if (core->module_cache.count(_address))
	{
		auto const& moduleState = core->module_cache[_address];
		if (moduleState.module)
		{
			delete moduleState.module;
		}
		//if (moduleState.instance)
		//{
		//	delete moduleState.instance;
		//}
		core->module_cache.erase(_address);
	}
}


struct RootResolver : Resolver
{
	std::map<std::string, Resolver*> moduleNameToResolverMap;

	bool resolve(const std::string& moduleName, const std::string& exportName, ObjectType type, ObjectInstance*& outObject) override
	{
		// Try to resolve an intrinsic first.
		if (IntrinsicResolver::singleton.resolve(moduleName, exportName, type, outObject)) { return true; }

		// Then look for a named module.
		auto namedResolverIt = moduleNameToResolverMap.find(moduleName);
		if (namedResolverIt != moduleNameToResolverMap.end())
		{
			return namedResolverIt->second->resolve(moduleName, exportName, type, outObject);
		}

		// Finally, stub in missing function imports.
		if (type.kind == ObjectKind::function)
		{
			// Generate a function body that just uses the unreachable op to fault if called.
			Serialization::ArrayOutputStream codeStream;
			OperatorEncoderStream encoder(codeStream);
			encoder.unreachable();
			encoder.end();

			// Generate a module for the stub function.
			Module stubModule;
			DisassemblyNames stubModuleNames;
			stubModule.types.push_back(asFunctionType(type));
			stubModule.functions.defs.push_back({ { 0 },{},std::move(codeStream.getBytes()),{} });
			stubModule.exports.push_back({ "importStub",ObjectKind::function,0 });
			stubModuleNames.functions.push_back({ std::string(moduleName) + "." + exportName,{} });
			IR::setDisassemblyNames(stubModule, stubModuleNames);
			IR::validateDefinitions(stubModule);

			// Instantiate the module and return the stub function instance.
			auto stubModuleInstance = instantiateModule(stubModule, {});
			outObject = getInstanceExport(stubModuleInstance, "importStub");
			Log::printf(Log::Category::error, "Generated stub for missing function import %s.%s : %s\n", moduleName.c_str(), exportName.c_str(), asString(type).c_str());
			BOOST_THROW_EXCEPTION(BadInstruction());
			return true;
		}
		else if (type.kind == ObjectKind::memory)
		{
			outObject = asObject(Runtime::createMemory(asMemoryType(type)));
			Log::printf(Log::Category::error, "Generated stub for missing memory import %s.%s : %s\n", moduleName.c_str(), exportName.c_str(), asString(type).c_str());
			BOOST_THROW_EXCEPTION(BadInstruction()); 
			return true;
		}
		else if (type.kind == ObjectKind::table)
		{
			outObject = asObject(Runtime::createTable(asTableType(type)));
			Log::printf(Log::Category::error, "Generated stub for missing table import %s.%s : %s\n", moduleName.c_str(), exportName.c_str(), asString(type).c_str());
			BOOST_THROW_EXCEPTION(BadInstruction()); 
			return true;
		}
		else if (type.kind == ObjectKind::global)
		{
			outObject = asObject(Runtime::createGlobal(asGlobalType(type), Runtime::Value(asGlobalType(type).valueType, Runtime::UntaggedValue())));
			Log::printf(Log::Category::error, "Generated stub for missing global import %s.%s : %s\n", moduleName.c_str(), exportName.c_str(), asString(type).c_str());
			BOOST_THROW_EXCEPTION(BadInstruction());
			return true;
		}

		return false;
	}
};

inline bool loadBinaryModule(const std::string& wasmBytes, IR::Module& outModule)
{
	Timing::Timer loadTimer;

	// Load the module from a binary WebAssembly file.
	try
	{
		Serialization::MemoryInputStream stream((const U8*)wasmBytes.data(), wasmBytes.size());
		WASM::serialize(stream, outModule);
	}
	catch (Serialization::FatalSerializationException exception)
	{
		ctrace << "Error deserializing WebAssembly binary file:";
		ctrace << exception.message;
		return false;
	}
	catch (IR::ValidationException exception)
	{
		ctrace << "Error validating WebAssembly binary file:";
		ctrace << exception.message;
		return false;
	}
	catch (std::bad_alloc)
	{
		ctrace << "Memory allocation failed: input is likely malformed";
		return false;
	}

	Timing::logRatePerSecond("Loaded WASM", loadTimer, wasmBytes.size() / 1024.0 / 1024.0, "MB");
	return true;
}

WASM_CORE::WASM_CORE()
{

}

WASM_CORE::~WASM_CORE()
{

}

void WASM_CORE::init()
{
	WASM_CORE::current_memory = NULL;
	WASM_CORE::current_ext = NULL;
	WASM_CORE::current_parameter = bytes();
	WASM_CORE::current_return = bytes();
	Runtime::init();
}

void WASM_CORE::destory()
{
	for (auto const& p : module_cache)
	{
		if (p.second.module)
		{
			delete p.second.module;
		}
		//if (moduleState.instance)
		//{
		//	delete moduleState.instance;
		//}
	}
	module_cache.clear();
	Runtime::freeUnreferencedObjects({});
}

WASM_CORE* WASM_CORE::getInstance()
{
	if (s_core == nullptr)
	{
		s_core = new WASM_CORE();
		s_core->init();
	}
	return s_core;
}

void WASM_CORE::destoryInstance()
{
	if (s_core)
	{
		s_core->destory();
		delete s_core;
		s_core = nullptr;
	}
}

void WASM_CORE::LoadModuleState(Address const& _address, const std::string& _string)
{

	//Module* module = nullptr;
	//ModuleInstance* moduleInstance = nullptr;

	if (module_cache.count(_address))
	{
		ModuleState& state = module_cache[_address];
		current_module = state.instance;
		current_memory = getDefaultMemory(current_module);
		current_state = &state;
		ClearMemory(current_memory);
		return;
	}

	module_cache.insert(make_pair(_address, ModuleState()));
	ModuleState& state = module_cache[_address];
	state.module = new Module();
	Serialization::MemoryInputStream inputStream((const U8*)_string.data(), _string.size());
	WASM::serializeWithInjection(inputStream, *state.module);

	RootResolver rootResolver;
	LinkResult linkResult = linkModule(*state.module, rootResolver);

	if (!linkResult.success)
	{
		//return;
		ctrace << "Failed to link module:";
		for (auto& missingImport : linkResult.missingImports)
		{
			ctrace << "Missing import: module=\"" << missingImport.moduleName
				<< "\" export=\"" << missingImport.exportName
				<< "\" type=\"" << asString(missingImport.type) << "\"";
		}
		throw;
	}
	state.instance = instantiateModule(*state.module, std::move(linkResult.resolvedImports));
	if (!state.instance) { throw; }
	current_memory = Runtime::getDefaultMemory(state.instance);


	char* memstart = &memoryRef<char>(current_memory, 0);

	const auto allocated_memory = Runtime::getDefaultMemorySize(state.instance);
	for (uint64_t i = 0; i < allocated_memory; ++i)
	{
		if (memstart[i])
		{
			state.mem_end = i + 1;
		}
	}
	state.init_memory.resize(state.mem_end);
	memcpy(state.init_memory.data(), memstart, state.mem_end);

	current_module = state.instance;
	current_memory = getDefaultMemory(current_module);
	current_state = &state;
	ClearMemory(current_memory);
}





bytes WASM_CORE::run(const std::string& _string, const char* functionName, uint64_t applyFunction, const bytes& args, ExtVMFace& _ext, bool isCreation)
{
	if (*(U32*)_string.data() != 0x6d736100)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}
	try
	{
		LoadModuleState(_ext.myAddress, _string);
	}
	catch (const Serialization::FatalSerializationException& exception)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}
	catch (IR::ValidationException exception)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}
	catch (std::bad_alloc)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}
	catch (...)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}

	if (!current_memory) {
		return bytes(0);
	}

	FunctionInstance* call = nullptr;

	call = asFunctionNullable(getInstanceExport(current_module, functionName));
	if (!call)
	{
		ctrace << "Module does not export '" << functionName << "'";
		if (strcmp(functionName, "apply") == 0)
			BOOST_THROW_EXCEPTION(BadInstruction());
		return bytes(0);
	}

	//Timing::Timer executionTimer;
	Runtime::Result functionResult;

	ModuleState& state = module_cache[_ext.myAddress];
	char* memstart = &memoryRef<char>(current_memory, 0);
	memset(memstart + state.mem_end, 0, ((1 << 16) - state.mem_end));
	memcpy(memstart, state.init_memory.data(), state.mem_end);

	try
	{
		std::unique_ptr<wasm_memory> wasm_memory_mgmt;
		const FunctionType* functionType = getFunctionType(call);

		// Set up the arguments for the invoke.
		std::vector<Value> invokeArgs;
		current_ext = &_ext;
		current_parameter = args;
		if (strcmp(functionName, "apply") == 0)
		{
			Value value = (U64)applyFunction;
			invokeArgs.push_back(value);
		}
		wasm_memory_mgmt.reset(new wasm_memory(*this));
		// 运行虚拟机之前，重置返回结果
		current_return = bytes();
		functionResult = invokeFunction(call, invokeArgs);
		wasm_memory_mgmt.reset();
		current_ext = nullptr;
	}
	catch (const Runtime::Exception& e)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}
	catch (VMException const& _e)
	{
		BOOST_THROW_EXCEPTION(BadInstruction());
	}
	catch (...)
	{
		ctrace << "catch (...)";
		BOOST_THROW_EXCEPTION(BadInstruction());
	}

	//Timing::logTimer("Invoked function", executionTimer);
	//std::string stringResult;
	//if (functionResult.type == ResultType::i32)
	//	stringResult = std::to_string(functionResult.i32);
	//else if (functionResult.type == ResultType::i64)
	//	stringResult = std::to_string(functionResult.i64);
	//else if (functionResult.type == ResultType::f32)
	//	stringResult = Floats::asString(functionResult.f32);
	//else if (functionResult.type == ResultType::f64)
	//	stringResult = Floats::asString(functionResult.f64);
	//else
	//	stringResult = asString(functionResult);
	//return asBytes(stringResult.c_str());

	return current_return;
}

void dev::eth::WASM_CORE::ClearMemory(Runtime::MemoryInstance * mem)
{
	int32_t pageSize = 1 << Platform::getPageSizeLog2(); 
	int32_t totalSize = getMemoryNumPages(mem) * pageSize;
	byte* pBuf = &memoryRef<byte>(mem,0);
	memset(pBuf, 0, totalSize);
}

wasm_memory::wasm_memory(WASM_CORE& core)
	: _wasm_core(core)
	, _num_pages(Runtime::getMemoryNumPages(core.current_memory))
	, _min_bytes(limit_32bit_address(_num_pages << numBytesPerPageLog2))
{
	_wasm_core.current_memory_management = this;
	_num_bytes = _min_bytes;
}

wasm_memory::~wasm_memory()
{
	if (_num_bytes > _min_bytes)
		sbrk((I32)_min_bytes - (I32)_num_bytes);
	_wasm_core.current_memory_management = nullptr;
}

U32 wasm_memory::sbrk(I32 num_bytes)
{
	const U32 previous_num_bytes = _num_bytes;
	if (Runtime::getMemoryNumPages(_wasm_core.current_memory) != _num_pages)
		throw;

	// Round the absolute value of num_bytes to an alignment boundary, and ensure it won't allocate too much or too little memory.
	num_bytes = (num_bytes + 7) & ~7;
	if (num_bytes > 0 && previous_num_bytes > _max_memory - num_bytes)
		throw;
	else if (num_bytes < 0 && previous_num_bytes < _min_bytes - num_bytes)
		throw;

	// Update the number of bytes allocated, and compute the number of pages needed for it.
	_num_bytes += num_bytes;
	const Uptr num_desired_pages = (_num_bytes + IR::numBytesPerPage - 1) >> IR::numBytesPerPageLog2;

	// Grow or shrink the memory object to the desired number of pages.
	if (num_desired_pages > _num_pages)
		Runtime::growMemory(_wasm_core.current_memory, num_desired_pages - _num_pages);
	else if (num_desired_pages < _num_pages)
		Runtime::shrinkMemory(_wasm_core.current_memory, _num_pages - num_desired_pages);

	_num_pages = num_desired_pages;

	return previous_num_bytes;
}

U32 wasm_memory::limit_32bit_address(Uptr address)
{
	return (U32)(address > UINT32_MAX ? UINT32_MAX : address);
}


