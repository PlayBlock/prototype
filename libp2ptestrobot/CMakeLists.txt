

file(GLOB sources "*.cpp" "*.hpp" 
"${CMAKE_SOURCE_DIR}/test/tools/libtesteth/*.cpp"
"${CMAKE_SOURCE_DIR}/test/tools/libtesteth/*.h"
"${CMAKE_SOURCE_DIR}/test/tools/libtestutils/*.cpp"
"${CMAKE_SOURCE_DIR}/test/tools/libtestutils/*.h"
"${CMAKE_SOURCE_DIR}/test/tools/fuzzTesting/*.cpp"
"${CMAKE_SOURCE_DIR}/test/tools/fuzzTesting/*.h"
"${CMAKE_SOURCE_DIR}/test/tools/jsontests/*.cpp"
"${CMAKE_SOURCE_DIR}/test/tools/jsontests/*.h"
)

add_library(p2ptestrobot ${sources} )

set_property(TARGET p2ptestrobot PROPERTY CXX_STANDARD 14)

target_include_directories(p2ptestrobot PRIVATE 
	${CMAKE_SOURCE_DIR}/evmjit/include  
	${UTILS_INCLUDE_DIR} 
	${UTILS_INCLUDE_DIR}/fc/include
	${Boost_INCLUDE_DIR}

	)


target_link_libraries(p2ptestrobot PRIVATE Boost::system fc ethereum)
ADD_DEFINITIONS(-DCHAINBASE_CHECK_LOCKING)
