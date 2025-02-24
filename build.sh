#!/bin/bash

# Exit on any error
set -e

# Colors for better output
declare -r RED='\033[0;31m'
declare -r GREEN='\033[0;32m'
declare -r YELLOW='\033[1;33m'
declare -r BLUE='\033[0;34m'
declare -r NC='\033[0m' # No Color

# Default values
declare BUILD_TYPE=""
declare COMPILER=""
declare CXX_COMPILER=""
declare BUILD_SYSTEM=""
declare BUILD_TESTS=""
declare -r SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
declare BUILD_DIR=""
declare -i PARALLEL_JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)"

usage() {
    cat << EOF
Usage: $(basename "$0") [OPTIONS]

Options:
    -h, --help              Show this help message
    -t, --type TYPE        Build type (Debug|RelWithDebInfo|Release)
    -c, --compiler NAME    Compiler to use (gcc|clang)
    -b, --build-system SYS Build system to use (make|ninja)
    -d, --build-dir DIR    Build directory (default: build-<TYPE>)
    -j, --jobs N           Number of parallel jobs (default: number of CPU cores)
    --tests               Enable building tests
    --clean               Clean build directory before building
EOF
    exit 0
}

print_error() {
    echo -e "${RED}Error: $1${NC}" >&2
}

print_info() {
    echo -e "${BLUE}$1${NC}"
}

print_success() {
    echo -e "${GREEN}$1${NC}"
}

print_warning() {
    echo -e "${YELLOW}$1${NC}"
}

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to ask yes/no questions
ask_yes_no() {
    local prompt="$1"
    local default="$2"
    local response

    # Set default value indicators
    local yes_indicator="y"
    local no_indicator="n"
    if [[ "$default" == "y" ]]; then
        yes_indicator="Y"
        no_indicator="n"
    elif [[ "$default" == "n" ]]; then
        yes_indicator="y"
        no_indicator="N"
    fi

    # Ask the question
    echo -n -e "${YELLOW}$prompt [$yes_indicator/$no_indicator]: ${NC}"
    read -r response

    # Process the response
    case "$response" in
        [Yy]*)
            return 0
            ;;
        [Nn]*)
            return 1
            ;;
        "")
            if [[ "$default" == "y" ]]; then
                return 0
            elif [[ "$default" == "n" ]]; then
                return 1
            fi
            ;;
        *)
            print_error "Invalid response. Please answer y or n."
            ask_yes_no "$prompt" "$default"
            ;;
    esac
}

# Validate build type
validate_build_type() {
    local type="$1"
    case "$type" in
        Debug|RelWithDebInfo|Release) return 0 ;;
        *) return 1 ;;
    esac
}

# Interactive menu for build type
select_build_type() {
    print_info "Please select the build type:"
    echo "1) Debug"
    echo "2) RelWithDebInfo"
    echo "3) Release"
    echo -n -e "${YELLOW}Enter your choice (1, 2, or 3): ${NC}"
    read -r choice

    case $choice in
        1) BUILD_TYPE="Debug" ;;
        2) BUILD_TYPE="RelWithDebInfo" ;;
        3) BUILD_TYPE="Release" ;;
        *)
            print_error "Invalid choice! Please select 1, 2, or 3."
            exit 1
            ;;
    esac
}

# Validate and set compiler
set_compiler() {
    local compiler="$1"
    case "$compiler" in
        gcc)
            if ! command_exists gcc || ! command_exists g++; then
                print_error "GCC/G++ not found"
                exit 1
            fi
            COMPILER="gcc"
            CXX_COMPILER="g++"
            ;;
        clang)
            if ! command_exists clang || ! command_exists clang++; then
                print_error "Clang/Clang++ not found"
                exit 1
            fi
            COMPILER="clang"
            CXX_COMPILER="clang++"
            ;;
        *)
            print_error "Invalid compiler: $compiler"
            exit 1
            ;;
    esac
}

# Interactive menu for compiler
select_compiler() {
    local options=()
    local available_compilers=""

    if command_exists gcc && command_exists g++; then
        options+=("gcc")
        available_compilers+="1) GCC\n"
    fi
    if command_exists clang && command_exists clang++; then
        options+=("clang")
        available_compilers+="2) Clang\n"
    fi

    case "${#options[@]}" in
        0)
            print_error "No supported compiler found!"
            exit 1
            ;;
        1)
            print_warning "Only ${options[0]} is available. Using ${options[0]}."
            set_compiler "${options[0]}"
            ;;
        *)
            print_info "Please select the compiler:"
            echo -e "$available_compilers"
            echo -n -e "${YELLOW}Enter your choice (1 or 2): ${NC}"
            read -r choice

            case $choice in
                1) set_compiler "gcc" ;;
                2) set_compiler "clang" ;;
                *)
                    print_error "Invalid choice! Please select 1 or 2."
                    exit 1
                    ;;
            esac
            ;;
    esac
}

# Validate and set build system
set_build_system() {
    local system="$1"
    case "$system" in
        make)
            if ! command_exists make; then
                print_error "Make not found"
                exit 1
            fi
            BUILD_SYSTEM="make"
            ;;
        ninja)
            if ! command_exists ninja; then
                print_error "Ninja not found"
                exit 1
            fi
            BUILD_SYSTEM="ninja"
            ;;
        *)
            print_error "Invalid build system: $system"
            exit 1
            ;;
    esac
}

# Interactive menu for build system
select_build_system() {
    local options=()
    local available_systems=""

    if command_exists make; then
        options+=("make")
        available_systems+="1) Make\n"
    fi
    if command_exists ninja; then
        options+=("ninja")
        available_systems+="2) Ninja\n"
    fi

    case "${#options[@]}" in
        0)
            print_error "No supported build system found!"
            exit 1
            ;;
        1)
            print_warning "Only ${options[0]} is available. Using ${options[0]}."
            set_build_system "${options[0]}"
            ;;
        *)
            print_info "Please select the build system:"
            echo -e "$available_systems"
            echo -n -e "${YELLOW}Enter your choice (1 or 2): ${NC}"
            read -r choice

            case $choice in
                1) set_build_system "make" ;;
                2) set_build_system "ninja" ;;
                *)
                    print_error "Invalid choice! Please select 1 or 2."
                    exit 1
                    ;;
            esac
            ;;
    esac
}

# Parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help)
                usage
                ;;
            -t|--type)
                if ! validate_build_type "$2"; then
                    print_error "Invalid build type: $2"
                    exit 1
                fi
                BUILD_TYPE="$2"
                shift 2
                ;;
            -c|--compiler)
                set_compiler "$2"
                shift 2
                ;;
            -b|--build-system)
                set_build_system "$2"
                shift 2
                ;;
            -d|--build-dir)
                BUILD_DIR="$2"
                shift 2
                ;;
            -j|--jobs)
                if ! [[ "$2" =~ ^[0-9]+$ ]]; then
                    print_error "Invalid number of jobs: $2"
                    exit 1
                fi
                PARALLEL_JOBS="$2"
                shift 2
                ;;
            --tests)
                BUILD_TESTS=1
                shift
                ;;
            --no-tests)
                BUILD_TESTS=0
                shift
                ;;
            --clean)
                CLEAN_BUILD=1
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                usage
                ;;
        esac
    done
}

# Function to check current CMake generator in build directory
get_current_generator() {
    local dir="$1"
    if [ -f "$dir/CMakeCache.txt" ]; then
        grep "CMAKE_GENERATOR:INTERNAL=" "$dir/CMakeCache.txt" | cut -d= -f2
    fi
}

# Function to check if we need to clean the build directory
need_clean_build() {
    local dir="$1"
    local new_system="$2"

    # If directory doesn't exist, no need to clean
    [ ! -d "$dir" ] && return 1

    local current_generator
    current_generator=$(get_current_generator "$dir")

    # If no generator found, no need to clean
    [ -z "$current_generator" ] && return 1

    # Check if generator would change
    if [ "$new_system" = "ninja" ] && [ "$current_generator" != "Ninja" ]; then
        return 0
    elif [ "$new_system" = "make" ] && [ "$current_generator" != "Unix Makefiles" ]; then
        return 0
    fi

    return 1
}

main() {
    # Parse command line arguments
    parse_args "$@"

    # If any required option is not set, use interactive menus
    [[ -z "$BUILD_TYPE" ]] && select_build_type
    [[ -z "$COMPILER" ]] && select_compiler
    [[ -z "$BUILD_SYSTEM" ]] && select_build_system

    # Ask about tests if not specified through command line
    if [[ -z "$BUILD_TESTS" ]]; then
        if ask_yes_no "Do you want to build tests?" "y"; then
            BUILD_TESTS=1
        else
            BUILD_TESTS=0
        fi
    fi

    # Set build directory if not specified
    [[ -z "$BUILD_DIR" ]] && BUILD_DIR="build"

    # Check if we need to clean due to generator change
    if need_clean_build "$BUILD_DIR" "$BUILD_SYSTEM"; then
        print_warning "Build system changed from $(get_current_generator "$BUILD_DIR") to $BUILD_SYSTEM"
        print_warning "Cleaning build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    # Clean build directory if explicitly requested
    elif [[ -n "$CLEAN_BUILD" && -d "$BUILD_DIR" ]]; then
        print_warning "Cleaning build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi

    # Create and enter build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR" || exit 1

    # Configure project
    print_info "Configuring project with:"
    print_info "  Build type: $BUILD_TYPE"
    print_info "  Compiler: $COMPILER"
    print_info "  Build system: $BUILD_SYSTEM"
    print_info "  Tests: $([ "$BUILD_TESTS" -eq 1 ] && echo "enabled" || echo "disabled")"

    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DCMAKE_C_COMPILER=$COMPILER"
        "-DCMAKE_CXX_COMPILER=$CXX_COMPILER"
        "-DBUILD_TESTS=$([[ "$BUILD_TESTS" -eq 1 ]] && echo "ON" || echo "OFF")"
    )

    [[ "$BUILD_SYSTEM" = "ninja" ]] && cmake_args+=("-GNinja")

    if ! cmake "${cmake_args[@]}" ..; then
        print_error "CMake configuration failed!"
        exit 1
    fi

    # Build project
    print_info "Building project with $BUILD_SYSTEM..."
    if [[ "$BUILD_SYSTEM" = "ninja" ]]; then
        ninja -j "$PARALLEL_JOBS"
    else
        make -j "$PARALLEL_JOBS"
    fi

    print_success "Build completed successfully!"
}

# Start script execution
main "$@"