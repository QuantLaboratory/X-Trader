include_directories(${CMAKE_INCLUDE_PATH})
link_directories(${CMAKE_LIBRARY_PATH})

if(UNIX)
	set(SYS_LIBs pthread stdc++fs)
endif()

aux_source_directory(. SRCs)
add_library(framework STATIC ${SRCs})
target_link_libraries(framework trade-core ${SYS_LIBs})