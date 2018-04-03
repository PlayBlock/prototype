
set(SOURCES
	ExtVMFace.cpp
	Instruction.cpp
	VM.cpp
	VMOpt.cpp
	VMCalls.cpp
	VMSIMD.cpp
	VMValidate.cpp
	VMFactory.cpp
	WASM_VM.cpp
	Vote.cpp
)

if (EVMJIT)
	list(APPEND SOURCES
		JitVM.cpp
		SmartVM.cpp
	)
endif()

file(GLOB HEADERS "*.h")

add_library(evm ${SOURCES} ${HEADERS})

add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-DWAVM_METRICS_OUTPUT=0)
add_definitions(-DPRETEND_32BIT_ADDRESS_SPACE=0)
add_definitions(-DENABLE_SIMD_PROTOTYPE=1)
add_definitions(-DENABLE_THREADING_PROTOTYPE=1)

find_package(WAVM REQUIRED) 
	
foreach (l ${WASM_LIB_LIST})  
		target_link_libraries(evm PUBLIC ${l})
endforeach()

target_link_libraries(evm PUBLIC ethcore devcore jumphash PRIVATE jsoncpp_lib_static)
target_include_directories(evm PUBLIC 
${CMAKE_SOURCE_DIR}/evmjit/include 
${WAVM_INCLUDE_DIRS}
"${UTILS_INCLUDE_DIR}/fc/include" 
"${UTILS_INCLUDE_DIR}"
)

if (EVMJIT)
	target_link_libraries(evm PRIVATE evmjit)
endif()
