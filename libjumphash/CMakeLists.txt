
file(GLOB sources "*.c" "*.h")

add_library(jumphash ${sources})

#target_include_directories(jumphash SYSTEM PUBLIC ${LEVELDB_INCLUDE_DIRS})
#target_link_libraries(jumphash ${LEVELDB_LIBRARIES})
set_property(TARGET jumphash PROPERTY CXX_STANDARD 14)

