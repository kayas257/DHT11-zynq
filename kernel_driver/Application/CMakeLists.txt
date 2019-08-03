cmake_minimum_required (VERSION 2.6)
project (Userlevel_memorymap)
# The version number.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_AR arm-linux-gnueabihf-ar) 
add_executable(dht11 dht11_app.c)
