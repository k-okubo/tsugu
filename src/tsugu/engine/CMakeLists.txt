include_directories(SYSTEM "${LLVM_INCLUDE_DIR}")

add_library(tsugu_engine
  compiler.cpp
  engine.cpp
  function_table.cpp
)
