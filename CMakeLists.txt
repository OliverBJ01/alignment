cmake_minimum_required(VERSION 3.7)
project(camLasAlignment)

set(CMAKE_CXX_STANDARD 11)

find_package( OpenCV REQUIRED )

set(SOURCE_FILES main.cpp helpers.cpp udpSender.cpp camLasAlignment.h bresenham.cpp scannerComms.cpp)
add_executable(camLasAlignment ${SOURCE_FILES})

include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries( camLasAlignment pthread ${OpenCV_LIBS} )