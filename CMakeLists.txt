cmake_minimum_required(VERSION 3.6)
project(toolbag)

add_subdirectory(dict)
add_subdirectory(match)

include_directories(dict match)

add_executable(guniq guniq.c dict/dict.c)

