add_custom_target(check-lang
    COMMAND ${PROJECT_SOURCE_DIR}/external/src/llvm-9.0.0.src/build/bin/llvm-lit -sv ${PROJECT_SOURCE_DIR}/test/lang
)
add_dependencies(check-lang tsugu)
