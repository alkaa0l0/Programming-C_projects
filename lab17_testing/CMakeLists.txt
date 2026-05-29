cmake_minimum_required(VERSION 3.10)
project(LinkedListTests)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CMOCKA REQUIRED cmocka IMPORTED_TARGET)

# Используем ваш существующий файл lab17_testing.c
add_executable(test_list lab17_testing.c)

target_link_libraries(test_list PkgConfig::CMOCKA)
target_compile_options(test_list PRIVATE -Wall -Wextra)

add_custom_target(run_tests
    COMMAND ./test_list
    DEPENDS test_list
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests..."
)
