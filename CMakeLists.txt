cmake_minimum_required(VERSION 3.14)
project(LibraryOS VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SQLite amalgamation, vendored in third_party/ (public domain).
add_library(sqlite3 STATIC third_party/sqlite3/sqlite3.c)
target_include_directories(sqlite3 PUBLIC third_party/sqlite3)
if(UNIX)
  target_link_libraries(sqlite3 PUBLIC dl pthread)
endif()

add_library(lms STATIC src/date.cpp src/database.cpp src/library.cpp src/html_export.cpp)
target_include_directories(lms PUBLIC include)
target_link_libraries(lms PUBLIC sqlite3)
if(MSVC)
  target_compile_options(lms PRIVATE /W4)
else()
  target_compile_options(lms PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(lms_cli src/main.cpp)
set_target_properties(lms_cli PROPERTIES OUTPUT_NAME lms)
target_link_libraries(lms_cli PRIVATE lms)

enable_testing()
add_executable(test_library tests/test_library.cpp)
target_link_libraries(test_library PRIVATE lms)
add_test(NAME library COMMAND test_library)
