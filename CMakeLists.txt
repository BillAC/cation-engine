cmake_minimum_required(VERSION 3.10)
project(cation_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(include)

# Source files for main application
set(SOURCES
    src/Constants.cpp
    src/LigandCSVLoader.cpp
    src/LigandData.cpp
    src/Solver.cpp
    main.cpp
)

# Source files for test application
set(TEST_SOURCES
    src/Constants.cpp
    src/LigandCSVLoader.cpp
    src/LigandData.cpp
    src/Solver.cpp
    test_ligand_loading.cpp
    test/CationEngineTest.cpp
    test/SolverTest.cpp
    test/validation_suite.cpp
    test/ReferenceSolverTest.cpp
    test_runner.cpp
)

# Create main executable
add_executable(cation_engine_main ${SOURCES})

# Create test executable
add_executable(cation_engine_test ${TEST_SOURCES})

# Link libraries for Windows cross-compilation
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_CXX_COMPILER MATCHES "mingw")
    # For Windows cross-compilation with MinGW, link necessary Win32 libraries
    target_link_libraries(cation_engine_main
        PRIVATE
        user32
        gdi32
        comctl32
        comdlg32
    )

    target_link_libraries(cation_engine_test
        PRIVATE
        user32
        gdi32
        comctl32
        comdlg32
    )
endif()

# Compiler flags
target_compile_options(cation_engine_main PRIVATE
    $<$<CONFIG:Release>:-O2 -Wall -Wextra>
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
)

target_compile_options(cation_engine_test PRIVATE
    $<$<CONFIG:Release>:-O2 -Wall -Wextra>
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
)

# Set Windows-specific properties if needed
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(cation_engine_main PROPERTIES
        VS_GLOBAL_ProjectName "cation_engine_main"
    )
    set_target_properties(cation_engine_test PROPERTIES
        VS_GLOBAL_ProjectName "cation_engine_test"
    )
endif()