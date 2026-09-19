cmake_minimum_required(VERSION 3.10)
project(Myapp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)   # 源码与执行字符集统一为 UTF-8，保证 ÷ × − ’ 正确输出
endif()

file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(Myapp ${SRC})
target_include_directories(Myapp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
