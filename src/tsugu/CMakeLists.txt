add_executable(tsugu
  tsugu.c
)
target_link_libraries(tsugu
  core
  engine
  ${LLVM_LIBS}
)
