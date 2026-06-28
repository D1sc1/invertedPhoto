cmake_minimum_required(VERSION 3.15)
project(invertedPhoto VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a Release build for single-config generators when nothing is set.
get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)

add_executable(inverted_photo
    main.cpp
    src/logger.cpp
    src/image.cpp
    src/inverter.cpp
    src/stb_impl.cpp
)

target_include_directories(inverted_photo PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb"
)

target_link_libraries(inverted_photo PRIVATE Threads::Threads)

# Strict warnings for our own code...
if(MSVC)
    target_compile_options(inverted_photo PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(inverted_photo PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ...but silence the third-party stb implementation unit.
if(MSVC)
    set_source_files_properties(src/stb_impl.cpp PROPERTIES COMPILE_OPTIONS "/w")
else()
    set_source_files_properties(src/stb_impl.cpp PROPERTIES COMPILE_OPTIONS "-w")
endif()

# ---------------------------------------------------------------------------
# Tests: a round-trip correctness check driven by CTest.
#   inv(inv(inv(x))) must equal inv(x) at the pixel level.
# ---------------------------------------------------------------------------
enable_testing()
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(
        NAME roundtrip
        COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify_roundtrip.py"
                "$<TARGET_FILE:inverted_photo>"
    )
else()
    message(STATUS "Python3 interpreter not found - round-trip test not registered")
endif()
