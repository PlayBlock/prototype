add_executable(ethvm main.cpp)

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
		target_link_libraries(ethvm PRIVATE ${l})
endforeach()

target_link_libraries(ethvm PRIVATE ethereum evm ethashseal devcore Boost::program_options )

target_include_directories(ethvm PRIVATE ../utils ${WAVM_INCLUDE_DIRS})

install(TARGETS ethvm DESTINATION bin OPTIONAL)
