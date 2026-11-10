add_library(search_common STATIC thread-pool.cpp thread-pool.h text-utils.h text-utils.cpp database.cpp database.h config.cpp config.h)
target_include_directories(search_common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(search_common PUBLIC pqxx Threads::Threads Boost::locale)
target_compile_features(search_common PUBLIC cxx_std_20)
set_target_properties(search_common PROPERTIES CXX_EXTENSIONS NO)
if(MSVC)
    target_compile_options(search_common PUBLIC /utf-8)
endif()


