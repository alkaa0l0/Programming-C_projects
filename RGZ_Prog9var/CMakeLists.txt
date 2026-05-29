cmake_minimum_required(VERSION 3.10)
project(translate LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# _GNU_SOURCE нужен для strsep/strdup/localtime_r
add_compile_definitions(_GNU_SOURCE)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

# --- Общая библиотека логики (без main) ---
add_library(translate_core
    src/utf8util.c
    src/dictionary.c
    src/translator.c
    src/logger.c
    src/config.c
)
target_include_directories(translate_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(translate_core PUBLIC Threads::Threads)

# --- Основной исполняемый файл ---
add_executable(translate src/main.c)
target_link_libraries(translate PRIVATE translate_core)

# --- UNIT-тесты ---
enable_testing()
add_executable(test_translate tests/test_translate.c)
target_link_libraries(test_translate PRIVATE translate_core)

add_test(NAME utf8      COMMAND test_translate utf8)
add_test(NAME dict      COMMAND test_translate dict)
add_test(NAME translate COMMAND test_translate translate)
add_test(NAME config    COMMAND test_translate config)
