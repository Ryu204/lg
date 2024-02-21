cmake_minimum_required(VERSION 3.27)

project(lg)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/stay)
add_subdirectory(extern/LDtkLoader)

file(GLOB_RECURSE
    lg_source
    CONFIGURE_DEPENDS
    "source/*"
)

option (lg_CLANG_TIDY "Use clang-tidy analyzer" OFF)
if (lg_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy"
        "--header-filter=${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp"
        "--config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy")
    if (MSVC)
        list(APPEND CMAKE_CXX_CLANG_TIDY "--extra-arg=/EHsc")
    endif()
endif()

add_executable(lg ${lg_source})
target_link_libraries(lg PRIVATE 
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,stay>" 
    LDtkLoader)

file(COPY asset DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
