add_subdirectory(core)
add_subdirectory(engine)

link_directories("${LLVM_LIBDIR}")

add_executable(tsugu
  tsugu.c
)
target_link_libraries(tsugu
  core
  engine
  ${LLVM_LIBS}
)

install(TARGETS tsugu
  RUNTIME DESTINATION bin
)
