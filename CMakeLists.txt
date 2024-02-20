cmake_minimum_required(VERSION 3.27)

project(lg)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/stay)

file(GLOB_RECURSE
    lg_source
    CONFIGURE_DEPENDS
    "source/*"
)

add_executable(lg ${lg_source})
target_link_libraries(lg PRIVATE stay)