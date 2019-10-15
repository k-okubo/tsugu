include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(tsugu_platform_linux linux.c)
add_library(tsugu_platform_dummy dummy.c)
