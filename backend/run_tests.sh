#!/bin/bash

# Todo Backend Test Runner Script

echo "=== Todo Backend Test Runner ==="
echo "Building and running comprehensive unit tests..."
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "info")
            echo -e "${YELLOW}[INFO]${NC} $message"
            ;;
        "success")
            echo -e "${GREEN}[SUCCESS]${NC} $message"
            ;;
        "error")
            echo -e "${RED}[ERROR]${NC} $message"
            ;;
    esac
}

# Change to script directory
cd "$(dirname "$0")"

# Clean up any existing test databases
print_status "info" "Cleaning up test databases..."
rm -f test_*.db
rm -f *.db

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    print_status "info" "Creating build directory..."
    mkdir build
fi

# Build the project
print_status "info" "Building project..."
cd build

if ! cmake .. > /dev/null 2>&1; then
    print_status "error" "CMake configuration failed"
    exit 1
fi

if ! make todo_tests > /dev/null 2>&1; then
    print_status "error" "Build failed"
    exit 1
fi

print_status "success" "Build completed successfully"

# Run the tests
print_status "info" "Running unit tests..."
echo

if ./todo_tests; then
    TEST_EXIT_CODE=$?
else
    TEST_EXIT_CODE=$?
fi

echo

# Clean up test databases
print_status "info" "Cleaning up test databases..."
rm -f test_*.db

# Report results
if [ $TEST_EXIT_CODE -eq 0 ]; then
    print_status "success" "All tests passed! ✓"
    echo
    echo "=== Test Summary ==="
    echo "• Database tests: ✓ Passed"
    echo "• Authentication tests: ✓ Passed"
    echo "• Todo service tests: ✓ Passed"
    echo "• Integration tests: ✓ Passed"
    echo
    echo "Your backend is ready for production!"
else
    print_status "error" "Some tests failed! ✗"
    echo
    echo "Please check the test output above and fix any issues."
    exit 1
fi

# Optional: Run with CMake's test runner
echo
print_status "info" "Running tests with CMake test runner..."
if make test > /dev/null 2>&1; then
    print_status "success" "CMake tests passed"
else
    print_status "error" "CMake tests failed"
fi

echo
echo "=== Test Run Complete ==="