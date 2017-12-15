

file(GLOB sources "*.cpp" "*.h")

add_library(producer ${sources})


find_package(LevelDB)
target_include_directories(producer SYSTEM PUBLIC ${LEVELDB_INCLUDE_DIRS})
#target_link_libraries(producer ${LEVELDB_LIBRARIES})

target_include_directories(producer PRIVATE 
	${CMAKE_SOURCE_DIR}/evmjit/include 
	"${UTILS_INCLUDE_DIR}" 
	"${UTILS_INCLUDE_DIR}/fc/include" 
	${Boost_INCLUDE_DIR}

	)

target_link_libraries(producer PRIVATE Boost::system)
