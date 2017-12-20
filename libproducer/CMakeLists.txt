

file(GLOB sources "*.cpp" "*.hpp")

add_library(producer ${sources})


find_package(LevelDB)
target_include_directories(producer SYSTEM PUBLIC ${LEVELDB_INCLUDE_DIRS})
#target_link_libraries(producer ${LEVELDB_LIBRARIES})
set_property(TARGET producer PROPERTY CXX_STANDARD 14)

target_include_directories(producer PRIVATE 
	${CMAKE_SOURCE_DIR}/evmjit/include 
	"${UTILS_INCLUDE_DIR}" 
	"${UTILS_INCLUDE_DIR}/fc/include" 
	${Boost_INCLUDE_DIR}

	)

target_link_libraries(producer PRIVATE Boost::system fc)
ADD_DEFINITIONS(-DCHAINBASE_CHECK_LOCKING)
