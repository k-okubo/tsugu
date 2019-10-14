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

set(LLVM_CONFIG "${PROJECT_SOURCE_DIR}/external/llvm-9.0.0/bin/llvm-config")
execute_process (COMMAND ${LLVM_CONFIG} --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process (COMMAND ${LLVM_CONFIG} --libdir OUTPUT_VARIABLE LLVM_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process (COMMAND ${LLVM_CONFIG} --libs OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(LLVM_LIBS "LLVM")
execute_process (COMMAND ${LLVM_CONFIG} --system-libs OUTPUT_VARIABLE LLVM_SYSTEM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory(lib)
add_subdirectory(src)

add_custom_target(check
    COMMAND ${PROJECT_SOURCE_DIR}/external/src/llvm-9.0.0.src/build/bin/llvm-lit -sv ${PROJECT_SOURCE_DIR}/test/
)
add_dependencies(check tsugu)
