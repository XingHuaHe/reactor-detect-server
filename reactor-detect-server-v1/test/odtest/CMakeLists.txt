cmake_minimum_required(VERSION 3.16)
project(test)


set(EXECUTABLE_OUTPUT_PATH ${EXEC_BIN_PATH})
message(STATUS "`odtest` executable output path: " ${EXECUTABLE_OUTPUT_PATH})


add_executable(odtest01 "odtest01.cpp")
target_link_libraries(odtest01 PUBLIC 
	${TORCH_LIBRARIES}
	${OpenCV_LIBS}
	${OBJECTDETECT_LIB})
set_property(TARGET odtest01 PROPERTY CXX_STANDARD 17)

add_executable(odtest02 "odtest02.cpp")
target_link_libraries(odtest02 PUBLIC 
	${OpenCV_LIBS})

add_executable(odtest03 "odtest03.cpp")
target_link_libraries(odtest03 PUBLIC 
	${OpenCV_LIBS})