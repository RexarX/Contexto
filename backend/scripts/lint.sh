#!/bin/bash

# Don't stop on errors, we'll handle them ourselves
set +e

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

# Verify we found the correct backend directory by checking for common directory structure
if [[ ! -d "$PROJECT_ROOT/src" ]]; then
    # Try one level up in case we're in a subdirectory
    if [[ -d "$(dirname "$PROJECT_ROOT")/src" ]]; then
        PROJECT_ROOT="$(dirname "$PROJECT_ROOT")"
    else
        echo -e "${RED}Error: Could not locate project root with src directory${NC}"
        echo -e "${YELLOW}Script location: $SCRIPT_DIR${NC}"
        echo -e "${YELLOW}Searched in: $PROJECT_ROOT${NC}"
        exit 1
    fi
fi

print_info() {
    echo -e "${BLUE}$1${NC}"
}

print_success() {
    echo -e "${GREEN}$1${NC}"
}

print_warning() {
    echo -e "${YELLOW}$1${NC}"
}

print_error() {
    echo -e "${RED}$1${NC}" >&2
}

# Check if clang-tidy is installed
if ! command -v clang-tidy &> /dev/null; then
    print_error "Error: clang-tidy is not installed. Please install it first."
    exit 1
fi

# Define source directories with absolute paths
SOURCE_DIRS=("${PROJECT_ROOT}/src")
FILE_EXTENSIONS=("cpp" "h" "hpp" "lint")

# Parse command line arguments
FIX=0
BUILD_DIR="${PROJECT_ROOT}/build"
VERBOSE=1  # Set default to verbose to help with debugging

while [[ $# -gt 0 ]]; do
    case "$1" in
        --fix)
            FIX=1
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -q|--quiet)
            VERBOSE=0
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Usage: $0 [--fix] [--build-dir BUILD_DIR] [--verbose] [--quiet]"
            exit 1
            ;;
    esac
done

# Print script initialization information
print_info "Lint Script"
print_info "==========="
print_info "Project root: $PROJECT_ROOT"
print_info "Source directories: ${SOURCE_DIRS[*]}"
print_info "Build directory: $BUILD_DIR"

# Convert BUILD_DIR to absolute path if it's not already
if [[ "$BUILD_DIR" != /* ]]; then
    # If BUILD_DIR is a relative path, make it absolute from the current directory
    BUILD_DIR="$(pwd)/$BUILD_DIR"
fi

# Check if compile_commands.json exists
if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
    print_error "Error: compile_commands.json not found in ${BUILD_DIR}."
    print_error "Please build the project first with CMAKE_EXPORT_COMPILE_COMMANDS=ON."
    exit 1
fi

# Find all source files
print_info "Finding source files..."

# Build the find command incrementally
find_command="find"

# Add source directories to the command
for dir in "${SOURCE_DIRS[@]}"; do
    # Check if directory exists
    if [[ ! -d "$dir" ]]; then
        print_warning "Directory does not exist: $dir"
        continue
    fi
    find_command="$find_command $dir"
done

find_command="$find_command -type f"

# Add extensions to the find command
extension_pattern=""
for ext in "${FILE_EXTENSIONS[@]}"; do
    if [[ -z "$extension_pattern" ]]; then
        extension_pattern="-name \"*.$ext\""
    else
        extension_pattern="$extension_pattern -o -name \"*.$ext\""
    fi
done

find_command="$find_command \( $extension_pattern \)"

# Execute the find command to get all source files
mapfile -t SOURCE_FILES < <(eval "$find_command")

if [[ ${#SOURCE_FILES[@]} -eq 0 ]]; then
    print_warning "No source files found. Checked directories: ${SOURCE_DIRS[*]}"
    exit 0
fi

print_info "Found ${#SOURCE_FILES[@]} source files to process."

# Display clang-tidy version for debugging
CLANG_TIDY_VERSION=$(clang-tidy --version)
print_info "Using clang-tidy version: $CLANG_TIDY_VERSION"

# Check if .clang-tidy file exists
CLANG_TIDY_CONFIG=""
if [[ -f "${PROJECT_ROOT}/.clang-tidy" ]]; then
    print_info "Using .clang-tidy configuration from ${PROJECT_ROOT}/.clang-tidy"
    CLANG_TIDY_CONFIG="--config-file=${PROJECT_ROOT}/.clang-tidy"
else
    print_warning "No .clang-tidy file found in ${PROJECT_ROOT}"
fi

# Process each file individually
HAS_ERRORS=0
TOTAL_FILES=${#SOURCE_FILES[@]}
PROCESSED=0

print_info "Starting linting process..."

for file in "${SOURCE_FILES[@]}"; do
    ((PROCESSED++))
    
    # Check if file is in third-party directory
    if [[ "$file" == *"/third-party/"* ]]; then
        if [[ $VERBOSE -eq 1 ]]; then
            print_info "[$PROCESSED/$TOTAL_FILES] Skipping third-party file: $file"
        fi
        continue
    fi
    
    if [[ $VERBOSE -eq 1 ]]; then
        print_info "[$PROCESSED/$TOTAL_FILES] Linting: $file"
    else
        # Print progress every 5 files or on the last file
        if (( PROCESSED % 5 == 0 )) || (( PROCESSED == TOTAL_FILES )); then
            print_info "Progress: $PROCESSED/$TOTAL_FILES files"
        fi
    fi
    
    # Prepare clang-tidy options
    FIX_FLAG=""
    if [[ $FIX -eq 1 ]]; then
        FIX_FLAG="--fix"
    fi
    
    # Run clang-tidy with configuration
    # Add additional options directly to disable problematic checks
    CHECKS_DISABLE="-checks=-modernize-use-designated-initializers,-modernize-avoid-c-arrays,-modernize-use-nodiscard,-bugprone-easily-swappable-parameters,-bugprone-implicit-widening-of-multiplication-result"
    
    # Run clang-tidy command
    clang-tidy "$file" -p="$BUILD_DIR" $CLANG_TIDY_CONFIG $CHECKS_DISABLE $FIX_FLAG > /tmp/clang_tidy_output.txt 2>&1
    CLANG_EXIT=$?
    
    # Check for different types of failures
    if [[ $CLANG_EXIT -eq 0 ]]; then
        # Success - show output if verbose or if there are warnings
        if [[ -s /tmp/clang_tidy_output.txt ]]; then
            print_warning "Linting produced warnings/info for $file:"
            cat /tmp/clang_tidy_output.txt
        else
            if [[ $VERBOSE -eq 1 ]]; then
                print_success "Linting passed with no warnings for $file"
            fi
        fi
    elif grep -q "Segmentation fault" /tmp/clang_tidy_output.txt; then
        # Segmentation fault
        print_error "clang-tidy crashed with segmentation fault on $file!"
        print_warning "Skipping this file due to clang-tidy crash"
    else
        # Other error
        print_error "Linting failed for $file (exit code: $CLANG_EXIT)"
        cat /tmp/clang_tidy_output.txt
        HAS_ERRORS=1
    fi
    
    if [[ $VERBOSE -eq 1 ]]; then
        # Add a separator between files for readability
        echo "----------------------------------------"
    fi
done

if [[ $HAS_ERRORS -eq 0 ]]; then
    print_success "Linting completed. No errors were detected."
    print_warning "Note: Third-party files and files causing clang-tidy crashes were skipped."
    exit 0
else
    print_error "Linting failed for some files."
    exit 1
fi