cmake_minimum_required(VERSION 3.15)
project(wiki_racer_solver C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add O2 optimization
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Build graph binary
add_executable(build_graph
    src/arena.c
    src/build_graph.c
    src/interner.c
    src/str.c
    src/vec.c
    src/bin/build_graph.c
)

# Solver binary (add source files when ready)
add_executable(solver
    src/bin/solver.c
)
# For now, create a placeholder so CMake doesn't fail
set_target_properties(solver PROPERTIES
    EXCLUDE_FROM_ALL TRUE
)

# Download munit for unit testing
include(FetchContent)
FetchContent_Declare(
    munit
    GIT_REPOSITORY https://github.com/nemequ/munit.git
    GIT_TAG master
)
FetchContent_MakeAvailable(munit)

# Enable testing
enable_testing()

# Unit tests
add_executable(run_tests
    tests/test_main.c
    src/interner.c
    src/arena.c
    src/str.c
    src/vec.c
    src/build_graph.c
    ${munit_SOURCE_DIR}/munit.c
)

target_include_directories(run_tests PRIVATE
    ${munit_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME unit_tests COMMAND run_tests)

# Optional: Add install targets
install(TARGETS build_graph DESTINATION bin)
install(TARGETS solver DESTINATION bin)  # Uncomment when solver is ready

