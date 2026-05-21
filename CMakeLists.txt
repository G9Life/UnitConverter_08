cmake_minimum_required(VERSION 3.16)
project(UnitConverter VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_library(unit_converter_core
    src/domain/UnitRegistry.cpp
    src/domain/Converter.cpp
    src/boundary/InputParser.cpp
    src/data/ConfigLoader.cpp
)

target_include_directories(unit_converter_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(UnitConverter UnitConverter.cpp)
target_link_libraries(UnitConverter PRIVATE unit_converter_core)

enable_testing()

add_executable(unit_converter_tests
    tests/domain_tests.cpp
    tests/boundary_tests.cpp
    tests/data_tests.cpp
)

target_compile_definitions(unit_converter_tests
    PRIVATE
        TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
)

target_link_libraries(unit_converter_tests
    PRIVATE
        unit_converter_core
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(unit_converter_tests)
