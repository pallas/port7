cmake_minimum_required(VERSION 3.1)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

project(Port7)
add_executable(port7 port7.cc)
find_library(EVENT_LIBRARY event)
if(EVENT_LIBRARY)
	target_link_libraries(port7 PUBLIC ${EVENT_LIBRARY})
endif()

install(TARGETS port7
	RUNTIME DESTINATION bin
	)
