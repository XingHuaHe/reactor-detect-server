cmake_minimum_required(VERSION 3.16)
project(objdetect)


find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
	# If the package has been found, several variables will be set, you can find the full list with descriptions
	# in the OpenCVConfig.cmake file. Print some message showing some of them.
	message(STATUS "OpenCV library status:")
	message(STATUS "	| version:		${OpenCV_VERSION}")
	message(STATUS "	| include path:	${OpenCV_INCLUDE_DIRS}" \n)
else ()
	message(FATAL_ERROR "Could not locate OpenCV." \n)
endif()


file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)


include_directories(${INCLUDE_PATH}/objdetect)
include_directories("/usr/local/include/opencv4/")


set(LIBRARY_OUTPUT_PATH ${LIBS_PATH})
message(STATUS "`libobjdetect.a` output path: " ${LIBRARY_OUTPUT_PATH})


add_library(${OBJECTDETECT_LIB} STATIC ${SRC_LIST})
target_link_libraries(${OBJECTDETECT_LIB} PUBLIC ${TORCH_LIBRARIES})
target_link_libraries(${OBJECTDETECT_LIB} PUBLIC ${OpenCV_LIBS})