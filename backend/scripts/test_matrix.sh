#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Define test matrix
BUILD_TYPES=("Debug" "Release" "RelWithDebInfo")
COMPILERS=("gcc" "clang")
BUILD_SYSTEMS=("ninja" "make")

# Results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Default test filter
TEST_FILTER=""

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Run tests across different build configurations"
    echo
    echo "Options:"
    echo "  -f, --filter FILTER   Run only tests matching the filter"
    echo "  -t, --types TYPES     Comma-separated list of build types (Debug,Release,RelWithDebInfo)"
    echo "  -c, --compilers COMP  Comma-separated list of compilers (gcc,clang)"
    echo "  -b, --build-systems BS Comma-separated list of build systems (ninja,make)"
    echo "  -h, --help            Show this help message"
}

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -f|--filter)
            TEST_FILTER="$2"
            shift 2
            ;;
        -t|--types)
            IFS=',' read -ra BUILD_TYPES <<< "$2"
            shift 2
            ;;
        -c|--compilers)
            IFS=',' read -ra COMPILERS <<< "$2"
            shift 2
            ;;
        -b|--build-systems)
            IFS=',' read -ra BUILD_SYSTEMS <<< "$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            print_usage
            exit 1
            ;;
    esac
done

echo -e "${BLUE}=== Running Test Matrix ===${NC}"
echo "Build types: ${BUILD_TYPES[*]}"
echo "Compilers: ${COMPILERS[*]}"
echo "Build systems: ${BUILD_SYSTEMS[*]}"
if [[ -n "$TEST_FILTER" ]]; then
    echo "Test filter: $TEST_FILTER"
fi

# Function to run tests for a specific configuration
run_test_config() {
    local build_type=$1
    local compiler=$2
    local build_system=$3

    echo -e "\n${YELLOW}Testing: $build_type / $compiler / $build_system${NC}"

    # Build with the specified configuration
    echo -e "${BLUE}Building...${NC}"
    cd "$(dirname "$0")/.." # Move to project root

    ./backend/scripts/build.sh --type "$build_type" --compiler "$compiler" \
        --build-system "$build_system" --tests --no-run

    if [[ $? -ne 0 ]]; then
        echo -e "${RED}Build failed!${NC}"
        return 1
    fi

    # Run tests
    echo -e "${BLUE}Running tests...${NC}"
    FILTER_ARG=""
    if [[ -n "$TEST_FILTER" ]]; then
        FILTER_ARG="--filter $TEST_FILTER"
    fi

    ./backend/scripts/run_tests.sh --type "$build_type" --no-build $FILTER_ARG
    local test_result=$?

    if [[ $test_result -eq 0 ]]; then
        echo -e "${GREEN}Tests passed!${NC}"
        return 0
    else
        echo -e "${RED}Tests failed with exit code $test_result${NC}"
        return $test_result
    fi
}

# Run through all configurations
for build_type in "${BUILD_TYPES[@]}"; do
    for compiler in "${COMPILERS[@]}"; do
        for build_system in "${BUILD_SYSTEMS[@]}"; do
            TOTAL_TESTS=$((TOTAL_TESTS + 1))

            run_test_config "$build_type" "$compiler" "$build_system"
            if [[ $? -eq 0 ]]; then
                PASSED_TESTS=$((PASSED_TESTS + 1))
            else
                FAILED_TESTS=$((FAILED_TESTS + 1))
            fi
        done
    done
done

# Print summary
echo -e "\n${BLUE}=== Test Matrix Summary ===${NC}"
echo -e "Total configurations: ${TOTAL_TESTS}"
echo -e "${GREEN}Passed: ${PASSED_TESTS}${NC}"
echo -e "${RED}Failed: ${FAILED_TESTS}${NC}"

if [[ $FAILED_TESTS -eq 0 ]]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
