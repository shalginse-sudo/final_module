add_executable(indexer main.cpp)
target_link_libraries(indexer PRIVATE search_common)
set_target_properties(indexer PROPERTIES CXX_EXTENSIONS NO)
add_custom_command(TARGET indexer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.ini" "$<TARGET_FILE_DIR:indexer>/config.ini"
    VERBATIM)
