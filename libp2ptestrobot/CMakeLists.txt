

file(GLOB sources "*.cpp" "*.hpp")

add_library(p2ptestrobot ${sources})

set_property(TARGET p2ptestrobot PROPERTY CXX_STANDARD 14)

target_include_directories(producer PRIVATE 
	${CMAKE_SOURCE_DIR}/evmjit/include  
	"${UTILS_INCLUDE_DIR}/fc/include" 
	"${UTILS_INCLUDE_DIR}" 
	${Boost_INCLUDE_DIR}

	)

target_link_libraries(p2ptestrobot PRIVATE Boost::system fc ethereum)
ADD_DEFINITIONS(-DCHAINBASE_CHECK_LOCKING)
