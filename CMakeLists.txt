cmake_minimum_required(VERSION 3.10)
project(CationEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the include directory
include_directories(include)

# Create the library
add_library(cation_engine
    src/LigandData.cpp
    src/CationEngine.cpp
    src/Solver.cpp
)

# Create the test executable
add_executable(cation_engine_test
    test/SolverTest.cpp
)

# Link the library to the test
target_link_libraries(cation_engine_test cation_engine)

# Create the main executable
add_executable(cation_engine_main
    main.cpp
)

# Link the library to the main executable
target_link_libraries(cation_engine_main cation_engine)

# Set compiler options
target_compile_options(cation_engine PRIVATE -Wall -Wextra)
target_compile_options(cation_engine_test PRIVATE -Wall -Wextra)