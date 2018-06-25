cmake_minimum_required(VERSION 3.1)
project( PiCamera )
find_package( OpenCV REQUIRED)
find_package( Threads)

# add all the *.c files as sources
FILE(GLOB SRC_FILES *.cpp)
FILE(GLOB HEAD_FILES *.h)
add_executable( PiCamera ${SRC_FILES} ${HEAD_FILES})
#add_executable( PiCamera pi_camera.cpp)


target_link_libraries( PiCamera ${OpenCV_LIBS} )
target_link_libraries( PiCamera ${CMAKE_THREAD_LIBS_INIT} )


#be sure to include the current source directory for header files
#target_include_directories (PiCamera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#include_directories(include ${CMAKE_CURRENT_SOURCE_DIR})
