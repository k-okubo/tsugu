include_directories(SYSTEM "${LLVM_INCLUDE_DIR}")

add_library(engine
  compiler.cpp
  engine.cpp
)
