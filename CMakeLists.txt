cmake_minimum_required(VERSION 3.18.4)
project(mportlink)

file(GLOB_RECURSE SRC_FILES_LIST src/*.c)

find_package(PkgConfig REQUIRED)

pkg_search_module(MM REQUIRED mm-glib)

include_directories(${MM_INCLUDE_DIRS})
link_directories(${MM_LIBRARY_DIRS})
add_definitions(${MM_CFLAGS_OTHER})

add_executable(${PROJECT_NAME} ${SRC_FILES_LIST})
target_link_libraries(${PROJECT_NAME} ${MM_LIBRARIES})
