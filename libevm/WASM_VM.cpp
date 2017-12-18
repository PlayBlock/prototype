
#include "WASM_VM.h"
#include <libethereum/ExtVM.h>
#include <iostream>

using namespace IR;
using namespace Runtime;

Runtime::MemoryInstance* WASM_CORE::current_memory;
ExtVMFace* WASM_CORE::current_ext;
bytes WASM_CORE::current_parameter;
u256 WASM_CORE::u256_temp;
WASM_CORE* WASM_VM::m_core;
//by dz
u256* WASM_VM::m_io_gas = nullptr;
int WASM_CORE::mem_end = 0;
//by dz end

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
	auto data = _ext.data;
	//std::cout << "data:" << (data.count() ? (const char*)(data.data()) : "") << std::endl;

	bytes retbytes;
	//try {
		if (!WASM_VM::m_core)
		{
			WASM_VM::m_core = new WASM_CORE();
			WASM_VM::m_core->init();
		}
		std::string string_temp((_ext).code.begin(), (_ext).code.end());
		bytes _totalBytes((_ext).data.begin(), (_ext).data.end());
		//by hjx 20171213
		if (isCreation)
		{
			retbytes = WASM_VM::m_core->run(string_temp, "init", 0, bytes(), _ext, true);
		}
		else
		{
			if (_totalBytes.size() < 8)
				BOOST_THROW_EXCEPTION(WASMParameterLengthTooSmall());
			//std::string funcName_;
			bytes applyFunction(_totalBytes.begin(), _totalBytes.begin() + 8);
			uint64_t applyFunctionUint;
			memcpy(&applyFunctionUint, applyFunction.data(), sizeof(applyFunctionUint));
			bytes paraString_(_totalBytes.begin() + 8, _totalBytes.end());

			bytes logs = asBytes("abc logs");
			_ext.log({}, bytesConstRef(logs.data(), logs.size()));

			//retbytes = WASM_VM::m_core->run(string_temp, funcName_.c_str(), paraString_, _ext);
			retbytes = WASM_VM::m_core->run(string_temp, "apply", applyFunctionUint, paraString_, _ext, false);
		}
		//by hjx 20171213 end

		WASM_VM::m_core->destory();

		if (!data.count())
		{ 
			//m_bytes = bytesConstRef(_ext.code.data(), _ext.code.size());
			bytes result = _ext.code;
			return owning_bytes_ref{ std::move(result), 0, result.size() };
		}
		else
		{
			//m_bytes = bytesConstRef(retbytes.data(), retbytes.size());
			return owning_bytes_ref{ std::move(retbytes), 0, retbytes.size() };
		}


	//}
	//catch (WASMOutOfGas)
	//{
	//	WASM_VM::m_core->destory();
	//	std::cout << "WASMOutOfGas~~" << std::endl;
	//	BOOST_THROW_EXCEPTION(OutOfGas());
	//}

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
			return true;
		}
		else if (type.kind == ObjectKind::memory)
		{
			outObject = asObject(Runtime::createMemory(asMemoryType(type)));
			Log::printf(Log::Category::error, "Generated stub for missing memory import %s.%s : %s\n", moduleName.c_str(), exportName.c_str(), asString(type).c_str());
			return true;
		}
		else if (type.kind == ObjectKind::table)
		{
			outObject = asObject(Runtime::createTable(asTableType(type)));
			Log::printf(Log::Category::error, "Generated stub for missing table import %s.%s : %s\n", moduleName.c_str(), exportName.c_str(), asString(type).c_str());
			return true;
		}
		else if (type.kind == ObjectKind::global)
		{
			outObject = asObject(Runtime::createGlobal(asGlobalType(type), Runtime::Value(asGlobalType(type).valueType, Runtime::UntaggedValue())));
			Log::printf(Log::Category::error, "Generated stub for missing global import %s.%s : %s\n", moduleName.c_str(), exportName.c_str(), asString(type).c_str());
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
		std::cerr << "Error deserializing WebAssembly binary file:" << std::endl;
		std::cerr << exception.message << std::endl;
		return false;
	}
	catch (IR::ValidationException exception)
	{
		std::cerr << "Error validating WebAssembly binary file:" << std::endl;
		std::cerr << exception.message << std::endl;
		return false;
	}
	catch (std::bad_alloc)
	{
		std::cerr << "Memory allocation failed: input is likely malformed" << std::endl;
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
	WASM_CORE::u256_temp = 0;
	Runtime::init();
}

void WASM_CORE::destory()
{
	Runtime::freeUnreferencedObjects({});
}

bytes WASM_CORE::run(const std::string& string, const char* functionName, uint64_t applyFunction, const bytes& args, ExtVMFace& _ext, bool isCreation)
{
	if (*(U32*)string.data() != 0x6d736100)
	{
		return bytes(0);
	}

	Serialization::MemoryInputStream inputStream((const U8*)string.data(), string.size());

	Module module;
	//WASM::serialize(inputStream, module);
	try
	{
		WASM::serializeWithInjection(inputStream, module);
	}
	catch (const Serialization::FatalSerializationException& exception)
	{		
		BOOST_THROW_EXCEPTION(BadInstruction());
	}


	//Module module;
	//if (!(loadBinaryModule(string, module)))
	//	return bytes(0);


	//module.memories.defs.front().type.size.min = 65 * 1024;
	RootResolver rootResolver;
	LinkResult linkResult = linkModule(module, rootResolver);
	if (!linkResult.success)
	{
		//return;
		std::cerr << "Failed to link module:" << std::endl;
		for (auto& missingImport : linkResult.missingImports)
		{
			std::cerr << "Missing import: module=\"" << missingImport.moduleName
				<< "\" export=\"" << missingImport.exportName
				<< "\" type=\"" << asString(missingImport.type) << "\"" << std::endl;
		}
		return bytes(0);
	}
	ModuleInstance* moduleInstance = instantiateModule(module, std::move(linkResult.resolvedImports));


	if (!moduleInstance) { return bytes(0); }

	current_memory = Runtime::getDefaultMemory(moduleInstance);
	current_ext = &_ext;
	current_parameter = args;

	setMemoryEnd(0);
	char* memstart = &memoryRef<char>(current_memory, 0);

	for (uint32_t i = 0; i < 10000; ++i)
	{
		if (memstart[i]) {
			setMemoryEnd(i + 1);
		}
	}


	if (!current_memory) {
		return bytes(0);
	}
	Emscripten::initInstance(module, moduleInstance);

	// Look up the function export to call.
	FunctionInstance* functionInstance;

	functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, functionName));
	if (!functionInstance)
	{
		std::cerr << "Module does not export '" << functionName << "'" << std::endl;
		return bytes(0);
	}

	const FunctionType* functionType = getFunctionType(functionInstance);

	// Set up the arguments for the invoke.
	std::vector<Value> invokeArgs;

	//by hjx 20171213
	if (isCreation == false)
	{
		Value value = (U64)applyFunction;
		invokeArgs.push_back(value);
	}
	//by hjx 20171213 end

	Timing::Timer executionTimer;
	auto functionResult = invokeFunction(functionInstance, invokeArgs);
	Timing::logTimer("Invoked function", executionTimer);
	std::string stringResult = asString(functionResult);
	return asBytes(stringResult.c_str());

}


