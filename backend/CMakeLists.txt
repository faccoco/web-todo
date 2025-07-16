cmake_minimum_required(VERSION 3.16)
project(todo_backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add executable
add_executable(todo_backend
    src/main.cpp
    src/todo_service.cpp
    src/database.cpp
    src/auth_service.cpp
)

# Link libraries
target_link_libraries(todo_backend 
    Threads::Threads
    sqlite3
)

# Compiler options
target_compile_options(todo_backend PRIVATE -Wall -Wextra -O2)

# Test executable
add_executable(todo_tests
    tests/test_main.cpp
    src/todo_service.cpp
    src/database.cpp
    src/auth_service.cpp
)

# Link libraries for tests
target_link_libraries(todo_tests 
    Threads::Threads
    sqlite3
)

# Compiler options for tests
target_compile_options(todo_tests PRIVATE -Wall -Wextra -O2)

# Add test target
enable_testing()
add_test(NAME unit_tests COMMAND todo_tests)