#!/bin/bash

# Stop on errors
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Determine script location and project root regardless of how the script is invoked
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
SCRIPT_NAME="$(basename "$SCRIPT_PATH")"

# Determine the actual backend directory (project root)
# If script is in backend/scripts, project root is one directory up
# If script is in backend/, project root is the script directory
if [[ "$SCRIPT_DIR" == */scripts ]]; then
    PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
else
    PROJECT_ROOT="$SCRIPT_DIR"
fi

# Verify we found the correct backend directory by checking for CMakeLists.txt
if [[ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]]; then
    # Try one level up in case we're in a subdirectory
    if [[ -f "$(dirname "$PROJECT_ROOT")/CMakeLists.txt" ]]; then
        PROJECT_ROOT="$(dirname "$PROJECT_ROOT")"
    else
        echo -e "${RED}Error: Could not locate project root with CMakeLists.txt${NC}"
        echo -e "${YELLOW}Script location: $SCRIPT_DIR${NC}"
        echo -e "${YELLOW}Searched in: $PROJECT_ROOT${NC}"
        exit 1
    fi
fi

# Default values
BUILD_TYPE=""
COMPILER=""
CXX_COMPILER=""
BUILD_SYSTEM=""
BUILD_TESTS=""
BUILD_DIR="$PROJECT_ROOT/build"
CLEAN_BUILD=0
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
FORMAT_CODE=0
LINT_CODE=0
RUN_AFTER_BUILD=0

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
    echo "Contexto Backend Build Script"
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -t, --type <type>         Build type: Debug (default), Release, RelWithDebInfo"
    echo "  -c, --compiler <name>     Compiler: gcc (default), clang"
    echo "  -b, --build-system <name> Build system: ninja (default), make"
    echo "  -d, --build-dir <dir>     Build directory (default: build)"
    echo "  -j, --jobs <num>          Number of parallel jobs (default: detected CPU count)"
    echo "      --tests               Build tests"
    echo "      --no-tests            Don't build tests (default)"
    echo "      --clean               Clean build directory before building"
    echo "      --format              Format code using clang-format before building"
    echo "      --lint                Run clang-tidy before building"
    echo "      --run                 Run the program after building"
    echo "  -h, --help                Show this help message"
}

ask_yes_no() {
    local prompt=$1
    local default=$2
    local yn

    if [[ "$default" == "y" ]]; then
        prompt="$prompt [Y/n]"
    else
        prompt="$prompt [y/N]"
    fi

    while true; do
        read -p "$prompt " yn
        case $yn in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            "" )
                if [[ "$default" == "y" ]]; then
                    return 0
                else
                    return 1
                fi
                ;;
            * ) echo "Please answer yes or no.";;
        esac
    done
}

select_build_type() {
    print_info "Please select the build type:"
    echo "1) Debug"
    echo "2) RelWithDebInfo"
    echo "3) Release"
    
    local choice
    read -p "Enter your choice (1, 2, or 3): " choice
    case $choice in
        1) BUILD_TYPE="Debug";;
        2) BUILD_TYPE="RelWithDebInfo";;
        3) BUILD_TYPE="Release";;
        *)
            print_error "Invalid choice! Please select 1, 2, or 3."
            select_build_type
            ;;
    esac
    
    print_info "Selected build type: $BUILD_TYPE"
}

select_compiler() {
    # Check for available compilers
    local HAS_GCC=0
    local HAS_CLANG=0
    
    if command -v gcc &>/dev/null; then
        HAS_GCC=1
    fi
    
    if command -v clang &>/dev/null; then
        HAS_CLANG=1
    fi
    
    if [[ $HAS_GCC -eq 0 && $HAS_CLANG -eq 0 ]]; then
        print_error "No supported compiler found!"
        exit 1
    fi
    
    if [[ $HAS_GCC -eq 1 && $HAS_CLANG -eq 0 ]]; then
        print_warning "Only GCC is available. Using GCC."
        COMPILER="gcc"
        CXX_COMPILER="g++"
        return 0
    fi
    
    if [[ $HAS_GCC -eq 0 && $HAS_CLANG -eq 1 ]]; then
        print_warning "Only Clang is available. Using Clang."
        COMPILER="clang"
        CXX_COMPILER="clang++"
        return 0
    fi
    
    # Both are available, ask user
    print_info "Please select the compiler:"
    echo "1) GCC"
    echo "2) Clang"
    
    local choice
    read -p "Enter your choice (1 or 2): " choice
    case $choice in
        1)
            COMPILER="gcc"
            CXX_COMPILER="g++"
            ;;
        2)
            COMPILER="clang"
            CXX_COMPILER="clang++"
            ;;
        *)
            print_error "Invalid choice! Please select 1 or 2."
            select_compiler
            ;;
    esac
    
    print_info "Selected compiler: $COMPILER / $CXX_COMPILER"
}

select_build_system() {
    # Check for available build systems
    local HAS_MAKE=0
    local HAS_NINJA=0
    
    if command -v make &>/dev/null; then
        HAS_MAKE=1
    fi
    
    if command -v ninja &>/dev/null; then
        HAS_NINJA=1
    fi
    
    if [[ $HAS_MAKE -eq 0 && $HAS_NINJA -eq 0 ]]; then
        print_error "No supported build system found!"
        exit 1
    fi
    
    if [[ $HAS_MAKE -eq 1 && $HAS_NINJA -eq 0 ]]; then
        print_warning "Only Make is available. Using Make."
        BUILD_SYSTEM="make"
        return 0
    fi
    
    if [[ $HAS_MAKE -eq 0 && $HAS_NINJA -eq 1 ]]; then
        print_warning "Only Ninja is available. Using Ninja."
        BUILD_SYSTEM="ninja"
        return 0
    fi
    
    # Both are available, ask user
    print_info "Please select the build system:"
    echo "1) Make"
    echo "2) Ninja (faster)"
    
    local choice
    read -p "Enter your choice (1 or 2): " choice
    case $choice in
        1) BUILD_SYSTEM="make";;
        2) BUILD_SYSTEM="ninja";;
        *)
            print_error "Invalid choice! Please select 1 or 2."
            select_build_system
            ;;
    esac
    
    print_info "Selected build system: $BUILD_SYSTEM"
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
            -c|--compiler)
                COMPILER="$2"
                if [[ "$COMPILER" == "gcc" ]]; then
                    CXX_COMPILER="g++"
                elif [[ "$COMPILER" == "clang" ]]; then
                    CXX_COMPILER="clang++"
                else
                    print_error "Unsupported compiler: $COMPILER"
                    exit 1
                fi
                shift 2
                ;;
            -b|--build-system)
                BUILD_SYSTEM="$2"
                shift 2
                ;;
            -d|--build-dir)
                # If a relative path is provided, make it relative to the current directory
                if [[ "${2:0:1}" != "/" ]]; then
                    BUILD_DIR="$(pwd)/$2"
                else
                    BUILD_DIR="$2"
                fi
                shift 2
                ;;
            -j|--jobs)
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
            --format)
                FORMAT_CODE=1
                shift
                ;;
            --lint)
                LINT_CODE=1
                shift
                ;;
            --run)
                RUN_AFTER_BUILD=1
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                print_help
                exit 1
                ;;
        esac
    done
}

check_dependencies() {
    print_info "Checking build dependencies..."
    
    # Check for CMake
    if ! command -v cmake &>/dev/null; then
        print_error "CMake not found"
        print_info "Please install cmake (https://cmake.org/download/)"
        exit 1
    fi
    
    # Validate compiler
    if [[ -n "$COMPILER" ]]; then
        if ! command -v "$COMPILER" &>/dev/null; then
            print_error "Compiler $COMPILER not found"
            COMPILER=""  # Reset to allow selection
        fi
        
        if ! command -v "$CXX_COMPILER" &>/dev/null; then
            print_error "C++ Compiler $CXX_COMPILER not found"
            COMPILER=""  # Reset to allow selection
        fi
    fi
    
    # Validate build system
    if [[ "$BUILD_SYSTEM" == "ninja" ]]; then
        if ! command -v ninja &>/dev/null; then
            print_warning "Ninja build system not found"
            BUILD_SYSTEM=""  # Reset to allow selection
        fi
    elif [[ "$BUILD_SYSTEM" == "make" ]]; then
        if ! command -v make &>/dev/null; then
            print_warning "Make build system not found"
            BUILD_SYSTEM=""  # Reset to allow selection
        fi
    fi
    
    # Check for Git
    if ! command -v git &>/dev/null; then
        print_warning "Git not found, version information might be unavailable"
    fi
    
    print_success "Build dependency check completed"
}

check_generator_change() {
    # Check if we need to clean due to build system change
    if [[ -f "$BUILD_DIR/CMakeCache.txt" ]]; then
        local CURRENT_GENERATOR=""
        
        # Extract current generator
        if grep -q "CMAKE_GENERATOR:INTERNAL=" "$BUILD_DIR/CMakeCache.txt"; then
            CURRENT_GENERATOR=$(grep "CMAKE_GENERATOR:INTERNAL=" "$BUILD_DIR/CMakeCache.txt" | cut -d'=' -f2)
            
            # Check if generator would change
            if [[ "$BUILD_SYSTEM" == "ninja" && "$CURRENT_GENERATOR" != "Ninja" ]]; then
                print_warning "Build system changed from $CURRENT_GENERATOR to Ninja"
                CLEAN_BUILD=1
            elif [[ "$BUILD_SYSTEM" == "make" && "$CURRENT_GENERATOR" != "Unix Makefiles" ]]; then
                print_warning "Build system changed from $CURRENT_GENERATOR to Make"
                CLEAN_BUILD=1
            fi
        fi
    fi
}

run_executable() {
    local run_script="${PROJECT_ROOT}/scripts/run.sh"
    
    if [[ ! -x "$run_script" ]]; then
        chmod +x "$run_script"
    fi
    
    print_info "Running the executable..."
    
    # Run the script with appropriate parameters
    "$run_script" --type "$BUILD_TYPE" --build-dir "$BUILD_DIR"
    
    local exit_code=$?
    if [[ $exit_code -ne 0 ]]; then
        print_error "Program exited with non-zero code: $exit_code"
    fi
}

main() {
    print_info "Contexto Backend Build Script"
    print_info "====================================="
    
    print_info "Project root detected at: $PROJECT_ROOT"
    
    parse_args "$@"
    check_dependencies
    
    # Interactive prompts for missing configuration
    if [[ -z "$BUILD_TYPE" ]]; then
        select_build_type
    fi
    
    if [[ -z "$COMPILER" ]]; then
        select_compiler
    fi
    
    if [[ -z "$BUILD_SYSTEM" ]]; then
        select_build_system
    fi
    
    # Ask about tests if not specified through command line
    if [[ -z "$BUILD_TESTS" ]]; then
        if ask_yes_no "Do you want to build tests?" "n"; then
            BUILD_TESTS=1
        else
            BUILD_TESTS=0
        fi
    fi
    
    # Check if build system changed
    check_generator_change
    
    # Format code if requested
    if [[ "$FORMAT_CODE" -eq 1 ]]; then
        print_info "Formatting code..."
        local format_script="${PROJECT_ROOT}/scripts/format.sh"
        if [[ -f "$format_script" ]]; then
            bash "$format_script"
            if [[ $? -ne 0 ]]; then
                print_error "Code formatting failed"
                exit 1
            fi
        else
            print_warning "format.sh not found at $format_script, skipping formatting"
        fi
    fi
    
    # Clean build directory if requested
    if [[ "$CLEAN_BUILD" -eq 1 && -d "$BUILD_DIR" ]]; then
        print_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    # Create and enter build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR" || exit 1
    
    # Configure project
    print_info "Configuring project with:"
    print_info "  Build type: $BUILD_TYPE"
    print_info "  Compiler: $COMPILER / $CXX_COMPILER"
    print_info "  Build system: $BUILD_SYSTEM"
    print_info "  Tests: $([[ "$BUILD_TESTS" -eq 1 ]] && echo "Enabled" || echo "Disabled")"
    print_info "  Build directory: $BUILD_DIR"
    
    # Build the cmake arguments
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DCMAKE_C_COMPILER=$COMPILER"
        "-DCMAKE_CXX_COMPILER=$CXX_COMPILER"
        "-DBUILD_TESTS=$([[ "$BUILD_TESTS" -eq 1 ]] && echo "ON" || echo "OFF")"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    )
    
    # Set generator based on build system
    if [[ "$BUILD_SYSTEM" == "ninja" ]]; then
        cmake_args+=("-GNinja")
    elif [[ "$BUILD_SYSTEM" == "make" ]]; then
        cmake_args+=("-G" "Unix Makefiles")  # Fixed Unix Makefiles format
    fi
    
    # Print the actual command for debugging
    echo -n "Running: cmake "
    printf "%s " "${cmake_args[@]}"
    echo "$PROJECT_ROOT"
    
    # Run CMake with the correct path to project root
    if ! cmake "${cmake_args[@]}" "$PROJECT_ROOT"; then
        print_error "CMake configuration failed!"
        exit 1
    fi
    
    # Build the project
    print_info "Building Contexto Backend..."
    
    if [[ "$BUILD_SYSTEM" == "ninja" ]]; then
        if ! ninja -j "$PARALLEL_JOBS"; then
            print_error "Build failed!"
            exit 1
        fi
    else
        if ! make -j "$PARALLEL_JOBS"; then
            print_error "Build failed!"
            exit 1
        fi
    fi
    
    # Lint code if requested - after build so compile_commands.json is available
    if [[ "$LINT_CODE" -eq 1 ]]; then
        print_info "Linting code..."
        local lint_script="${PROJECT_ROOT}/scripts/lint.sh"
        if [[ -f "$lint_script" ]]; then
            # Pass the absolute build directory path to lint.sh
            bash "$lint_script" --build-dir "$BUILD_DIR"
            if [[ $? -ne 0 ]]; then
                print_error "Code linting failed"
                exit 1
            fi
        else
            print_warning "lint.sh not found at $lint_script, skipping linting"
        fi
    fi
    
    print_success "Build successful!"
    
    # Display the path to the executable based on build type
    local exe_path="$BUILD_DIR/bin"
    [[ -d "$exe_path/$BUILD_TYPE" ]] && exe_path="$exe_path/$BUILD_TYPE"
    
    if [[ -d "$exe_path" ]]; then
        print_info "Executables can be found in: $exe_path"
    fi
    
    # Ask about running the application if not specified through command line
    if [[ "$RUN_AFTER_BUILD" -ne 1 ]]; then
        if ask_yes_no "Do you want to run the application?" "y"; then
            RUN_AFTER_BUILD=1
        fi
    fi
    
    # Run the executable if requested
    if [[ "$RUN_AFTER_BUILD" -eq 1 ]]; then
        run_executable
    fi
}

main "$@"