

set (fc_sources
	src/time.cpp
)

#file ( GLOB_RECURSE fc_headers ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)
set ( fc_headers
	include/fc/time.hpp
)

set (sources
	${fc_sources}
)

list(APPEND sources ${fc_headers})


ADD_LIBRARY(fc STATIC ${sources})

target_link_libraries(fc PRIVATE Boost::chrono Boost::date_time)

target_include_directories(fc PRIVATE include)