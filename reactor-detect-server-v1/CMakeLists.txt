cmake_minimum_required(VERSION 3.16)
project(reactor-detect-server)


# root path.
set(ROOT_PATH ${PROJECT_SOURCE_DIR})
# include path.
set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
# source path
set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
# library (both static and dynamic libraries).
set(LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# binary path.
set(EXEC_BIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
# libtorch.
set(TORCH_PATH "/root/xinghua.he/libtorch")


# libraries name.
set(INTERNET_LIB internet)
set(OBJECTDETECT_LIB ojbdetect)


include_directories(${INCLUDE_PATH})
include_directories(${INCLUDE_PATH}/internet)
include_directories(${INCLUDE_PATH}/objdetect)

# libopencv.
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

# libtorch.
set(CMAKE_PREFIX_PATH ${TORCH_PATH})
include_directories(${TORCH_PATH}/include)
message("CMAKE_PREFIX_PATH=" ${CMAKE_PREFIX_PATH})
find_package(Torch REQUIRED)
if (Torch_FOUND)
	message(STATUS "Torch library status:")
	message(STATUS "	| version:		${Torch_VERSION}")
	message(STATUS "	| include path:	${Torch_INCLUDE_DIRS}" \n)
else()
	message(FATAL_ERROR "Could not locate Torch." \n)
endif()


include_directories(${OpenCV_INCLUDE_DIRS})


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)


set(EXECUTABLE_OUTPUT_PATH ${EXEC_BIN_PATH})
message(STATUS "`reactor` executable output path: " ${EXECUTABLE_OUTPUT_PATH})


add_executable(reactor reactor.cpp)
target_link_libraries(reactor PUBLIC
	${INTERNET_LIB}
	${TORCH_LIBRARIES}
	${OBJECTDETECT_LIB})
set_property(TARGET reactor PROPERTY CXX_STANDARD 17)
