cmake_minimum_required(VERSION 3.29)
project(bmpcodec C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

add_executable(bmpcodec main.c)

target_link_libraries(bmpcodec m)