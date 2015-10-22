project(segy_converter)
cmake_minimum_required(VERSION 2.6)
set(${PROJECT_NAME}_headers seismogram.h)
set(${PROJECT_NAME}_sources main.cpp seismogram.cpp)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_headers} ${${PROJECT_NAME}_sources})

