cmake_minimum_required(VERSION 3.16)
project(test)


set(EXECUTABLE_OUTPUT_PATH ${EXEC_BIN_PATH})
message(STATUS "`client` executable output path: " ${EXECUTABLE_OUTPUT_PATH})


add_executable(client01 "client01.cpp")
add_executable(client02 "client02.cpp")
add_executable(client03 "client03.cpp")
