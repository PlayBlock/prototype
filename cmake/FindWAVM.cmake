INCLUDE (FindPackageHandleStandardArgs)

set(LIBS WAST;WASM;Emscripten;Runtime;IR;Logging;LLVMPasses;LLVMipo;LLVMInstrumentation;LLVMVectorize;
LLVMIRReader;LLVMAsmParser;LLVMLinker;LLVMMCJIT;LLVMExecutionEngine;LLVMRuntimeDyld;LLVMDebugInfoDWARF;LLVMX86CodeGen;LLVMAsmPrinter;LLVMDebugInfoCodeView;
LLVMDebugInfoMSF;LLVMGlobalISel;LLVMSelectionDAG;LLVMCodeGen;LLVMScalarOpts;LLVMInstCombine;LLVMBitWriter;LLVMTransformUtils;LLVMTarget;LLVMAnalysis;LLVMProfileData;LLVMX86AsmParser;LLVMX86Desc;LLVMX86AsmPrinter;LLVMX86Utils;
LLVMObject;LLVMMCParser;LLVMBitReader;LLVMCore;LLVMX86Disassembler;LLVMX86Info;LLVMMCDisassembler;LLVMMC;LLVMSupport;LLVMDemangle;Platform)

IF (MSVC)
	FIND_PATH (WAVM_ROOT_DIR NAMES include/WASM/WASM/WASM.h)
	# Re-use the previous path:
	FIND_PATH (
		WAVM_INCLUDE_DIR
		NAMES WASM/WASM/WASM.h
		HINTS ${WAVM_ROOT_DIR}
		PATH_SUFFIXES WASM
	)

ELSE ()
	SET	(WAVM_ROOT_DIR ${PROJECT_SOURCE_DIR}/deps)
	SET (WAVM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/install/x64/include)
ENDIF(MSVC)


 IF (MSVC)
  
	foreach (l ${LIBS})  
		string(TOUPPER ${l} L)
		
		FIND_LIBRARY(
			WASM_${L}_LIB
			NAMES ${l} 
			HINTS ${WAVM_ROOT_DIR} 
		) 

		set(WASM_LIB_LIST ${WASM_LIB_LIST} ${WASM_${L}_LIB})
	endforeach()
  
ELSE()

	foreach (l ${LIBS})  
		string(TOUPPER ${l} L)
		
		FIND_LIBRARY(
			WASM_${L}_LIB
			NAMES ${l} 
			HINTS ${WAVM_ROOT_DIR}/install/x64/lib
		) 

		set(WASM_LIB_LIST ${WASM_LIB_LIST} ${WASM_${L}_LIB})
	endforeach()

ENDIF(MSVC)


SET (WAVM_INCLUDE_DIRS ${WAVM_INCLUDE_DIR}/WASM)
SET (WAVM_LIBRARIES ${WAVM_LIBRARY})

message(STATUS "WAVM_ROOT_DIR : " ${WAVM_ROOT_DIR})
message(STATUS "WAVM_INCLUDE_DIRS : " ${WAVM_INCLUDE_DIRS})
message(STATUS "WASM_LIB_LIST : " ${WASM_LIB_LIST})

MARK_AS_ADVANCED (WAVM_INCLUDE_DIRS WAVM_LIBRARIES)
