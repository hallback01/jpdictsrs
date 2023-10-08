cmake_minimum_required(VERSION 3.15)
project(jpdictsrs CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Curses REQUIRED)
set(CURSES_NEED_WIDE TRUE)

# add include directories
include_directories(${CURSES_INCLUDE_DIR})
include_directories(
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/dep/curl/include
)

# add source files
file(GLOB_RECURSE src_files "src/*.cpp")
file(GLOB_RECURSE include_files "include/*.h")

# create executable
add_executable(jpdictsrs ${src_files} ${include_files})
target_link_libraries(jpdictsrs ${CURSES_LIBRARIES})

# link
if(WIN32)
    target_link_libraries(jpdictsrs -static -static-libgcc -static-libstdc++)
endif()