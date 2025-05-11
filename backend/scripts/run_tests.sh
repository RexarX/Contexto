#!/bin/bash

# Stop on errors
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Determine script location and project root
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default values
BUILD_TYPE="Debug"
BUILD_DIR="$PROJECT_ROOT/build"
TEST_FILTER=""
TEST_VERBOSE=0
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
BUILD_BEFORE_TEST=1
CUSTOM_TEST_ARGS=""
TEST_OUTPUT_FORMAT="auto" # auto, junit, or console
EXTERNAL_BUILD=0  # New flag to indicate if building is handled externally

print_info() {
    echo -e "${BLUE}$1${NC}"
}

print_error() {
    echo -e "${RED}Error: $1${NC}" >&2
}

print_success() {
    echo -e "${GREEN}$1${NC}"
}

print_warning() {
    echo -e "${YELLOW}Warning: $1${NC}"
}

print_help() {
    echo "Contexto Test Runner"
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -t, --type <type>         Build type: Debug (default), Release, RelWithDebInfo"
    echo "  -b, --build-dir <dir>     Build directory (default: build)"
    echo "  -f, --filter <pattern>    Run only tests matching the pattern"
    echo "  -v, --verbose             Run tests with verbose output"
    echo "  -j, --jobs <num>          Number of parallel test jobs (default: detected CPU count)"
    echo "  --no-build                Don't build before testing"
    echo "  --external-build          Skip build step (used when build is handled externally)"
    echo "  --test-args <args>        Additional arguments to pass to CTest"
    echo "  --format <format>         Test output format: auto, junit, console (default: auto)"
    echo "  -h, --help                Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --verbose                       # Run all tests with verbose output"
    echo "  $0 --filter utf8                   # Run only tests containing 'utf8'"
    echo "  $0 --type Release --no-build       # Run tests in Release mode without building"
    echo "  $0 --format junit                  # Output in JUnit XML format"
}

parse_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help)
                print_help
                exit 0
                ;;
            -t|--type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            -b|--build-dir)
                if [[ "${2:0:1}" != "/" ]]; then
                    BUILD_DIR="$(pwd)/$2"
                else
                    BUILD_DIR="$2"
                fi
                shift 2
                ;;
            -f|--filter)
                TEST_FILTER="$2"
                shift 2
                ;;
            -v|--verbose)
                TEST_VERBOSE=1
                shift
                ;;
            -j|--jobs)
                PARALLEL_JOBS="$2"
                shift 2
                ;;
            --no-build)
                BUILD_BEFORE_TEST=0
                shift
                ;;
            --external-build)
                EXTERNAL_BUILD=1
                BUILD_BEFORE_TEST=0  # Implicitly disable internal build
                shift
                ;;
            --test-args)
                CUSTOM_TEST_ARGS="$2"
                shift 2
                ;;
            --format)
                TEST_OUTPUT_FORMAT="$2"
                shift 2
                ;;
            *)
                print_error "Unknown option: $1"
                print_help
                exit 1
                ;;
        esac
    done
}

build_tests() {
    # Skip if external build is being used
    if [[ $EXTERNAL_BUILD -eq 1 ]]; then
        print_info "Skipping build (handled externally)"
        return 0
    fi

    print_info "Building tests..."

    local build_cmd=("$PROJECT_ROOT/scripts/build.sh" "--type" "$BUILD_TYPE" "--build-dir" "$BUILD_DIR" "--tests" "--no-run")

    print_info "Running: ${build_cmd[*]}"
    "${build_cmd[@]}"

    if [[ $? -ne 0 ]]; then
        print_error "Failed to build tests"
        exit 1
    fi

    print_success "Tests built successfully"
}

run_tests() {
    print_info "Running tests with configuration: $BUILD_TYPE"

    # Move to build directory
    cd "$BUILD_DIR"

    # Prepare CTest arguments
    local ctest_args=("-C" "$BUILD_TYPE")

    # Add verbosity
    if [[ $TEST_VERBOSE -eq 1 ]]; then
        ctest_args+=("-V")
    fi

    # Add parallel jobs
    ctest_args+=("-j" "$PARALLEL_JOBS")

    # Add filter if specified
    if [[ -n "$TEST_FILTER" ]]; then
        ctest_args+=("-R" "$TEST_FILTER")
    fi

    # Add output format
    if [[ "$TEST_OUTPUT_FORMAT" == "junit" ]]; then
        # Create a reports directory if it doesn't exist
        mkdir -p "$PROJECT_ROOT/test-reports"
        ctest_args+=("--output-junit" "$PROJECT_ROOT/test-reports/test-results.xml")
    elif [[ "$TEST_OUTPUT_FORMAT" == "auto" && -n "$CI" ]]; then
        # If running in CI environment, default to JUnit format
        mkdir -p "$PROJECT_ROOT/test-reports"
        ctest_args+=("--output-junit" "$PROJECT_ROOT/test-reports/test-results.xml")
    fi

    # Add any custom arguments
    if [[ -n "$CUSTOM_TEST_ARGS" ]]; then
        ctest_args+=($CUSTOM_TEST_ARGS)
    fi

    # Display the command
    print_info "Running: ctest ${ctest_args[*]}"

    # Run the tests
    ctest "${ctest_args[@]}"
    local test_result=$?

    if [[ $test_result -eq 0 ]]; then
        print_success "All tests passed!"
    else
        print_error "Test failures occurred"
        return $test_result
    fi
}

main() {
    print_info "Contexto Test Runner"
    print_info "===================="

    parse_args "$@"

    # Build tests if requested
    if [[ $BUILD_BEFORE_TEST -eq 1 ]]; then
        build_tests
    else
        print_info "Skipping build phase"
    fi

    # Run tests
    run_tests
    exit $?
}

main "$@"
