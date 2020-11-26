cmake_minimum_required (VERSION 2.6)


project(mtfind)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")

include_directories(e)
add_definitions()

add_executable(mtfind 

main.cpp
)


#target_link_libraries(avclean curl pthread)

