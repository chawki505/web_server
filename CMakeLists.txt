cmake_minimum_required(VERSION 3.13)
project(web_server C)

set(CMAKE_C_STANDARD 11)

add_executable(web_server src/server.c)