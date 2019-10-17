set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding")

include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(tsugu_core
  ast.c
  error.c
  parser.c
  resolver.c
  scanner.c
  symtbl.c
  token.c
  tyenv.c
  tymap.c
  type.c
  verifier.c
)
