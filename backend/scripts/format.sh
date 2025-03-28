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

# Path to the .clang-format file
CLANG_FORMAT_FILE="${PROJECT_ROOT}/.clang-format"

# Check if the .clang-format file exists
if [[ ! -f "$CLANG_FORMAT_FILE" ]]; then
    print_error "Error: .clang-format file not found at: $CLANG_FORMAT_FILE"
    exit 1
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

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    print_error "Error: clang-format is not installed. Please install it first."
    exit 1
fi

# Define source directories with absolute paths
SOURCE_DIRS=("${PROJECT_ROOT}/src")
FILE_EXTENSIONS=("cpp" "h" "hpp" "inl")

# Parse command line arguments
CHECK_ONLY=0
VERBOSE=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --check)
            CHECK_ONLY=1
            shift
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Usage: $0 [--check] [--verbose]"
            exit 1
            ;;
    esac
done

# Print script initialization information
print_info "Format Script"
print_info "============="
print_info "Project root: $PROJECT_ROOT"
print_info "Source directories: ${SOURCE_DIRS[*]}"
print_info "Using clang-format config: $CLANG_FORMAT_FILE"

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

if [[ $VERBOSE -eq 1 ]]; then
    print_info "Running: $find_command"
fi

# Execute the find command to get all source files
mapfile -t SOURCE_FILES < <(eval "$find_command")

if [[ ${#SOURCE_FILES[@]} -eq 0 ]]; then
    print_warning "No source files found. Checked directories: ${SOURCE_DIRS[*]}"
    exit 0
fi

print_info "Found ${#SOURCE_FILES[@]} source files to process."

# Process files
if [[ $CHECK_ONLY -eq 1 ]]; then
    print_info "Checking format only (not modifying files)..."
    needs_formatting=0
    
    for file in "${SOURCE_FILES[@]}"; do
        if [[ $VERBOSE -eq 1 ]]; then
            print_info "Checking: $file"
        fi
        
        if ! clang-format -style=file:${CLANG_FORMAT_FILE} --dry-run --Werror "$file" &> /dev/null; then
            print_warning "File needs formatting: $file"
            needs_formatting=1
        fi
    done
    
    if [[ $needs_formatting -eq 0 ]]; then
        print_success "All files are correctly formatted."
        exit 0
    else
        print_error "Some files need formatting. Run '${SCRIPT_NAME}' to format them."
        exit 1
    fi
else
    print_info "Formatting files..."
    for file in "${SOURCE_FILES[@]}"; do
        if [[ $VERBOSE -eq 1 ]]; then
            print_info "Formatting: $file"
        fi
        clang-format -style=file:${CLANG_FORMAT_FILE} -i "$file"
    done
    print_success "All files formatted successfully."
fi