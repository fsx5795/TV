cmake_minimum_required(VERSION 3.24)
#set(CMAKE_MAKE_PROGRAM "C:/msys64/mingw64/bin/ninja.exe")
project(media)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(GLOB HEADS ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB SOURS ${PROJECT_SOURCE_DIR}/*.c)
#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_package(PkgConfig REQUIRED)
find_package(CURL REQUIRED)
message(${CURL_LIBRARIES})
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
#PKG_CHECK_MODULES(GTK4 REQUIRED gtk4)
PKG_CHECK_MODULES(GST REQUIRED gstreamer-1.0>=1.8 gstreamer-video-1.0)
include_directories(${GST_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
#include_directories(${GST_INCLUDE_DIRS} ${GTK4_INCLUDE_DIRS})
link_directories(${GST_LIBRARY_DIRS} ${GTK3_LIBRARY_DIRS} ${CURL_LIBRARY_DIRS})
#link_directories(${GST_LIBRARY_DIRS} ${GTK4_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})
#add_definitions(${GTK4_CFLAGS_OTHER})
add_executable(${PROJECT_NAME} ${HEADS} ${SOURS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GST_LIBRARIES} ${GTK3_LIBRARIES} ${OPENGL_LIBRARIES} ${CURL_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} PRIVATE ${GST_LIBRARIES} ${GTK4_LIBRARIES} ${OPENGL_LIBRARIES})