add_executable(tsugu
  tsugu.c
)
target_link_libraries(tsugu
  tsugu_core
  tsugu_engine
  ${LLVM_LIBS}
)
install(TARGETS tsugu
  RUNTIME DESTINATION bin
)
