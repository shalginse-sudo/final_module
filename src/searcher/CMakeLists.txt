add_executable(searcher main.cpp)
target_link_libraries(searcher PRIVATE search_common)
set_target_properties(searcher PROPERTIES CXX_EXTENSIONS NO)
add_custom_command(TARGET searcher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.ini" "$<TARGET_FILE_DIR:searcher>/config.ini"
    VERBATIM)
