cmake_minimum_required(VERSION 3.16)
project(HTTP_Proxy)
set(CMAKE_PREFIX_PATH "/home/claudiu/Qt/6.8.0/gcc_64")
# Find Qt6 libraries
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Specify the source files
add_executable(HTTP_Proxy src/main.cpp)

# Link Qt6 Widgets library
target_link_libraries(HTTP_Proxy Qt6::Widgets)
