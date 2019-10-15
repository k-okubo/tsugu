add_subdirectory(lang)
#add_subdirectory(lib)

add_custom_target(check)
add_dependencies(check tsugu)
add_dependencies(check check-lang)
#add_dependencies(check core_client)
