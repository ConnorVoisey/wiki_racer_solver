# Default recipe to display help
default:
    @just --list

# Configure and build the project
build:
    mkdir -p build
    cd build && cmake ..
    cd build && cmake --build .

# Build and run tests
test: build
    cd build && ctest --output-on-failure

# Format all C source files
format:
    find src tests -name "*.c" -o -name "*.h" | xargs clang-format -i

# Clean build artifacts
clean:
    rm -rf build

# Build and run the build_graph binary
run-build-graph: build
    ./build/build_graph

# Build and run the solver binary (when implemented)
run-solver: build
    ./build/solver

# Full rebuild from scratch
rebuild: clean build

# Run tests with verbose output
test-verbose: build
    cd build && ctest --verbose
