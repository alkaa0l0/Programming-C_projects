cmake_minimum_required(VERSION 3.10)
project(Paint++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =====================================================
# CHECK FOR SFML
# =====================================================
message(STATUS "Checking for SFML library...")
find_package(SFML 2.5 COMPONENTS graphics window system) 

if(SFML_FOUND)
    message(STATUS "SFML found: ${SFML_VERSION}")
else()
    message(FATAL_ERROR "
    SFML not found!
    
    Quick Install:
    - Windows (vcpkg): vcpkg install sfml:x64-windows
    - Windows (manual): Download SFML and set -DSFML_DIR=C:/SFML/lib/cmake/SFML
    - Ubuntu/Debian: sudo apt install libsfml-dev
    - macOS: brew install sfml
    ")
endif()

add_executable(paint src/main.cpp src/paint_core.cpp)

# Link SFML
target_link_libraries(paint sfml-graphics sfml-window sfml-system)

# For Windows: disable console (for a pure GUI application)
if(WIN32)
    set_target_properties(paint PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()
