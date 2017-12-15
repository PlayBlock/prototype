

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

FIND_PACKAGE(Boost 1.34 REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES(../../deps/install/win64/include)

ADD_LIBRARY(fc STATIC ${sources})

INSTALL( TARGETS fc
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION . )