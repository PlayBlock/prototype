

file(GLOB sources "*.cpp" "*.hpp" 

"${CMAKE_SOURCE_DIR}/test/tools/libtesteth/*.h"
"${CMAKE_SOURCE_DIR}/test/tools/libtestutils/*.h"
"${CMAKE_SOURCE_DIR}/test/tools/fuzzTesting/*.h"
"${CMAKE_SOURCE_DIR}/test/tools/jsontests/*.h"
${CMAKE_SOURCE_DIR}/test/tools/libtesteth/TestSuite.cpp
${CMAKE_SOURCE_DIR}/test/tools/libtesteth/TestOutputHelper.cpp
${CMAKE_SOURCE_DIR}/test/tools/libtesteth/TestHelper.cpp
${CMAKE_SOURCE_DIR}/test/tools/libtesteth/Stats.cpp
${CMAKE_SOURCE_DIR}/test/tools/jsontests/StateTests.cpp
${CMAKE_SOURCE_DIR}/test/tools/libtesteth/Options.cpp
${CMAKE_SOURCE_DIR}/test/tools/libtesteth/ImportTest.cpp
${CMAKE_SOURCE_DIR}/test/tools/fuzzTesting/fuzzHelper.cpp
${CMAKE_SOURCE_DIR}/test/tools/libtesteth/FillJsonFunctions.cpp
${CMAKE_SOURCE_DIR}/test/tools/fuzzTesting/createRandomTest.cpp
${CMAKE_SOURCE_DIR}/test/tools/libtestutils/Common.cpp
${CMAKE_SOURCE_DIR}/test/tools/fuzzTesting/BoostRandomCode.cpp
${CMAKE_SOURCE_DIR}/test/tools/jsontests/BlockChainTests.cpp
${CMAKE_SOURCE_DIR}/test/tools/libtesteth/BlockChainHelper.cpp

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
