cmake_minimum_required(VERSION 3.16)
project(internet)


file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)


include_directories(${INCLUDE_PATH}/internet)


set(LIBRARY_OUTPUT_PATH ${LIBS_PATH})
message(STATUS "`libinternet.a` output path: " ${LIBRARY_OUTPUT_PATH})


add_library(${INTERNET_LIB} STATIC ${SRC_LIST})