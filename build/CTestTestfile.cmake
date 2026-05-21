# CMake generated Testfile for 
# Source directory: C:/DEV/UnitConverter_08
# Build directory: C:/DEV/UnitConverter_08/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("C:/DEV/UnitConverter_08/build/unit_converter_tests-b12d07c_include.cmake")
add_test(GoldenMaster "C:/DEV/UnitConverter_08/build/unit_converter_tests.exe" "[golden][regression][r01]")
set_tests_properties(GoldenMaster PROPERTIES  LABELS "golden;regression;r01" _BACKTRACE_TRIPLES "C:/DEV/UnitConverter_08/CMakeLists.txt;76;add_test;C:/DEV/UnitConverter_08/CMakeLists.txt;0;")
subdirs("_deps/catch2-build")
