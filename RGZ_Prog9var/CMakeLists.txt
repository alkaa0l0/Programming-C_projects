cmake_minimum_required(VERSION 3.10)
project(translate LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# strdup и localtime требуют объявлений POSIX
add_compile_definitions(_POSIX_C_SOURCE=200809L)
add_compile_options(-Wall -Wextra)

# Общая библиотека с логикой (без main) — её используют программа и тесты
add_library(translate_core
    src/dictionary.c
    src/translator.c
    src/logger.c
    src/config.c
)
target_include_directories(translate_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Основная программа
add_executable(translate src/main.c)
target_link_libraries(translate PRIVATE translate_core)

# UNIT-тесты
enable_testing()
add_executable(test_translate tests/test_translate.c)
target_link_libraries(test_translate PRIVATE translate_core)

add_test(NAME case      COMMAND test_translate case)
add_test(NAME dict      COMMAND test_translate dict)
add_test(NAME translate COMMAND test_translate translate)
add_test(NAME config    COMMAND test_translate config)
