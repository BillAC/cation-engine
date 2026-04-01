cmake_minimum_required(VERSION 3.10)
project(cation_engine VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories - now everything is in src
include_directories(src)

# Source files for main application
set(SOURCES
    src/Constants.cpp
    src/LigandCSVLoader.cpp
    src/LigandData.cpp
    src/Solver.cpp
    src/CationEngine.cpp
    src/main.cpp
)

# Source files for test application
set(TEST_SOURCES
    src/Constants.cpp
    src/LigandCSVLoader.cpp
    src/LigandData.cpp
    src/Solver.cpp
    src/CationEngine.cpp
    src/test/CationEngineTest.cpp
    src/test/SolverTest.cpp
    src/test/validation_suite.cpp
    src/test/ReferenceSolverTest.cpp
    src/test/test_runner.cpp
)

# Create main executable
add_executable(cation_engine_main ${SOURCES})

# Create test executable
add_executable(cation_engine_test ${TEST_SOURCES})

# Link libraries for Windows
if(WIN32)
    if(MSVC)
        # MSVC specific options
        target_compile_definitions(cation_engine_main PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_definitions(cation_engine_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        
        target_compile_options(cation_engine_main PRIVATE /W4 /O2)
        target_compile_options(cation_engine_test PRIVATE /W4 /O2)
        
        target_link_libraries(cation_engine_main
            PRIVATE
            user32
            gdi32
            comctl32
            comdlg32
            shell32
        )
    else()
        # For Windows cross-compilation with MinGW, link necessary Win32 libraries
        target_link_libraries(cation_engine_main
            PRIVATE
            user32
            gdi32
            comctl32
            comdlg32
            shell32
        )
        
        # Enforce static linking for portability (removes libgcc_s_seh-1.dll dependency)
        set_target_properties(cation_engine_main PROPERTIES LINK_FLAGS "-static -static-libgcc -static-libstdc++")
        set_target_properties(cation_engine_test PROPERTIES LINK_FLAGS "-static -static-libgcc -static-libstdc++")
        
        target_compile_options(cation_engine_main PRIVATE
            $<$<CONFIG:Release>:-O2 -Wall -Wextra>
            $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
        )
    endif()

    target_link_libraries(cation_engine_test
        PRIVATE
        user32
        gdi32
        comctl32
        comdlg32
        shell32
    )
endif()

# Set Windows-specific properties
if(WIN32)
    # Ensure the main app is a GUI application (no console window)
    set_target_properties(cation_engine_main PROPERTIES 
        WIN32_EXECUTABLE TRUE
    )
endif()