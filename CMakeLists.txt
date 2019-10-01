cmake_minimum_required(VERSION 2.8.12)
project(tsugu)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -pedantic -Wshadow")
set(CMAKE_C_FLAGS_DEBUG "-O0 -Werror -g -pg")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG -g -pg")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic -Wshadow")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Werror -g -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG -g -pg")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

set(LLVM_CONFIG "llvm-config-7")
execute_process (COMMAND ${LLVM_CONFIG} --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process (COMMAND ${LLVM_CONFIG} --libs OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory(lib)
add_subdirectory(src)
