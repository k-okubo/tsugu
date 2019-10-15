set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -pedantic -Wshadow -nostdlib")
set(CMAKE_C_FLAGS_DEBUG "-O0 -Werror -g")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG -g")

add_executable(core_client core_client.c)
target_link_libraries(core_client tsugu_core)
