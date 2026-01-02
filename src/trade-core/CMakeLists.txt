if(WIN32)
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
    "${PROJECT_SOURCE_DIR}/../api/ctp/v6.7.2/win64/")
else()
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
    "${PROJECT_SOURCE_DIR}/../api/ctp/v6.7.2/linux64/")
endif()


if(WIN32)
    file(GLOB_RECURSE DLLs ${PROJECT_SOURCE_DIR}/../api/ctp/v6.7.2/win64/*.dll)
    file(COPY ${DLLs} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endif()


include_directories(${CMAKE_INCLUDE_PATH})
link_directories(${CMAKE_LIBRARY_PATH})

aux_source_directory(market MARKET)
aux_source_directory(trader TRADER)


add_library(trade-core STATIC interface.cpp ${MARKET} ${TRADER})
set(CTP_LIBs thostmduserapi_se thosttraderapi_se)
target_link_libraries(trade-core ${CTP_LIBs})
