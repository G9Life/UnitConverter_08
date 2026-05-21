cmake_minimum_required(VERSION 3.16)
project(UnitConverter VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# RED phase (default): link tests against stubs in tests/red_phase_stubs.cpp only.
# GREEN phase: cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=OFF
option(UNIT_CONVERTER_RED_PHASE "TDD RED: stub core (tests fail until GREEN src/)" ON)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

if(UNIT_CONVERTER_RED_PHASE)
    add_library(unit_converter_core tests/red_phase_stubs.cpp)
else()
    add_library(unit_converter_core
        src/domain/UnitRegistry.cpp
        src/domain/Converter.cpp
        src/boundary/InputParser.cpp
        src/data/ConfigLoader.cpp
    )
endif()

target_include_directories(unit_converter_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(UnitConverter src/UnitConverter.cpp)
target_link_libraries(UnitConverter PRIVATE unit_converter_core)

enable_testing()

add_executable(unit_converter_tests
    tests/domain_tests.cpp
    tests/boundary_tests.cpp
    tests/data_tests.cpp
)

if(NOT UNIT_CONVERTER_RED_PHASE)
    target_sources(unit_converter_tests PRIVATE tests/golden_master_tests.cpp)
endif()

target_compile_definitions(unit_converter_tests
    PRIVATE
        TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
)

if(NOT UNIT_CONVERTER_RED_PHASE)
    target_compile_definitions(unit_converter_tests
        PRIVATE
            UNIT_CONVERTER_EXE="$<TARGET_FILE:UnitConverter>"
            GOLDEN_MASTER_EXPECTED="${CMAKE_CURRENT_SOURCE_DIR}/tests/golden_master_expected.txt"
    )
endif()

target_link_libraries(unit_converter_tests
    PRIVATE
        unit_converter_core
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(unit_converter_tests)
