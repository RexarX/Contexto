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
BUILD_TYPE="Debug"
BUILD_DIR="$PROJECT_ROOT/build"
CONFIG_DIR="$PROJECT_ROOT/configs"
STATIC_CONFIG="static_config.yaml"
RUNTIME_CONFIG="config.yaml"
LOG_LEVEL="debug"
ARGS=()
INTERACTIVE=0
# Check if the script is being run interactively (with a terminal)
[[ -t 0 ]] && INTERACTIVE=1

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
    echo "Contexto Backend Run Script"
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -t, --type <type>         Build type: Debug (default), Release, RelWithDebInfo"
    echo "  -b, --build-dir <dir>     Build directory (default: build)"
    echo "  -c, --config-dir <dir>    Config directory (default: configs)"
    echo "  -s, --static-config <file> Static config file (default: static_config.yaml)"
    echo "  -a, --arg <arg>           Additional argument to pass to the executable"
    echo "  -i, --interactive         Force interactive mode (select parameters from menus)"
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

select_log_level() {
    print_info "Please select the log level:"
    echo "1) trace (most detailed)"
    echo "2) debug"
    echo "3) info"
    echo "4) warning"
    echo "5) error"
    echo "6) critical"
    echo "7) none (no logging)"
    
    local choice
    read -p "Enter your choice (1-7): " choice
    case $choice in
        1) LOG_LEVEL="trace";;
        2) LOG_LEVEL="debug";;
        3) LOG_LEVEL="info";;
        4) LOG_LEVEL="warning";;
        5) LOG_LEVEL="error";;
        6) LOG_LEVEL="critical";;
        7) LOG_LEVEL="none";;
        *)
            print_error "Invalid choice! Please select 1-7."
            select_log_level
            ;;
    esac
    
    print_info "Selected log level: $LOG_LEVEL"
}

select_config_file() {
    local config_type=$1  # "static" or "runtime"
    local config_var=$2   # The variable to set
    local default_value=$3
    local title_case="${config_type^}"  # Capitalize first letter
    
    # Default location
    local config_dir="${CONFIG_DIR}"
    
    # List available config files
    print_info "Select ${title_case} Config File:"
    
    # Create the config directory if it doesn't exist
    if [[ ! -d "$config_dir" ]]; then
        mkdir -p "$config_dir"
        print_warning "Created config directory: $config_dir"
    fi
    
    # Find all .yaml files in the config directory
    local yaml_files=("$config_dir"/*.yaml "$config_dir"/*.yml)
    
    if [[ ${#yaml_files[@]} -eq 0 || ! -f "${yaml_files[0]}" ]]; then
        print_warning "No YAML files found in $config_dir"
        print_info "Using default: $default_value"
        # Set the variable using indirect reference
        eval "$config_var=\"$default_value\""
        return
    fi
    
    # Print the list of files
    local i=1
    local default_index=1
    for file in "${yaml_files[@]}"; do
        if [[ -f "$file" ]]; then
            local basename=$(basename "$file")
            echo "$i) $basename"
            # If this file matches our default, note the index
            if [[ "$basename" == "$default_value" ]]; then
                default_index=$i
            fi
            ((i++))
        fi
    done
    
    # Add option to create a new file
    echo "$i) Create a new file"
    
    # Ask the user to select a file
    local max_choice=$i
    read -p "Enter your choice (1-$max_choice) [default: $default_index]: " choice
    
    # Use default if empty
    if [[ -z "$choice" ]]; then
        choice=$default_index
    fi
    
    # Handle the selection
    if [[ "$choice" -eq "$max_choice" ]]; then
        # Create a new file
        read -p "Enter a name for the new ${config_type} config file: " new_name
        if [[ ! "$new_name" =~ \.ya?ml$ ]]; then
            new_name="${new_name}.yaml"
        fi
        local new_file="$config_dir/$new_name"
        touch "$new_file"
        print_info "Created new file: $new_file"
        # Set the variable using indirect reference
        eval "$config_var=\"$new_name\""
    elif [[ "$choice" -ge 1 && "$choice" -lt "$max_choice" ]]; then
        local selected_index=$(($choice - 1))
        local selected_file=""
        
        # Find the selected file
        i=0
        for file in "${yaml_files[@]}"; do
            if [[ -f "$file" ]]; then
                if [[ $i -eq $selected_index ]]; then
                    selected_file=$(basename "$file")
                    break
                fi
                ((i++))
            fi
        done
        
        print_info "Selected: $selected_file"
        # Set the variable using indirect reference
        eval "$config_var=\"$selected_file\""
    else
        print_error "Invalid choice!"
        select_config_file "$config_type" "$config_var" "$default_value"
    fi
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
            -c|--config-dir)
                if [[ "${2:0:1}" != "/" ]]; then
                    CONFIG_DIR="$(pwd)/$2"
                else
                    CONFIG_DIR="$2"
                fi
                shift 2
                ;;
            -s|--static-config)
                STATIC_CONFIG="$2"
                shift 2
                ;;
            -a|--arg)
                ARGS+=("$2")
                shift 2
                ;;
            -i|--interactive)
                INTERACTIVE=1
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

find_executable() {
    # Find the executable name from CMakeLists.txt
    local project_name=$(grep -m 1 "set(PROJECT_NAME" "$PROJECT_ROOT/CMakeLists.txt" | cut -d ' ' -f2 | tr -d ')' | tr -d '\r\n')

    local exe_path="$PROJECT_ROOT/bin/$BUILD_TYPE/$project_name"
    
    # Print debug message to stderr so it doesn't get captured in command substitution
    print_info "Looking for executable at: $exe_path" >&2
    
    if [[ -f "$exe_path" ]]; then
        # Only output the path, nothing else
        echo "$exe_path"
        return 0
    else
        return 1
    fi
}

list_available_build_types() {
    # Extract project name and clean any trailing carriage returns or newlines
    local project_name=$(grep -m 1 "set(PROJECT_NAME" "$PROJECT_ROOT/CMakeLists.txt" | cut -d ' ' -f2 | tr -d ')' | tr -d '\r\n')
    local bin_dir="$PROJECT_ROOT/bin"
    local found_types=()
    
    print_info "Project name from CMakeLists.txt: '$project_name'" >&2
    
    # Check if bin directory exists
    if [[ ! -d "$bin_dir" ]]; then
        print_warning "bin directory not found at $bin_dir" >&2
        return 1
    fi
    
    print_info "Checking for build types in: $bin_dir" >&2
    
    # Look for subdirectories
    for dir_entry in "$bin_dir"/*; do
        if [[ -d "$dir_entry" ]]; then
            local dir_name=$(basename "$dir_entry")
            print_info "Checking directory: $dir_name" >&2
            
            # Check for executable in this directory
            if [[ -f "$dir_entry/$project_name" ]]; then
                print_info "Found executable in $dir_name" >&2
                # Prevent duplicate entries
                if [[ ! " ${found_types[*]} " =~ " ${dir_name} " ]]; then
                    found_types+=("$dir_name")
                fi
            fi
        fi
    done
    
    # Return the found types - just list each once
    if [[ ${#found_types[@]} -gt 0 ]]; then
        # Use printf '%s\n' to output each type on a new line, then sort -u to remove duplicates
        printf '%s\n' "${found_types[@]}" | sort -u
    else
        print_warning "No executables found in any build directory" >&2
    fi
}

select_from_available_build_types() {
    local -a types=("$@")
    
    print_info "Please select a build type:"
    local i=1
    for type in "${types[@]}"; do
        echo "$i) $type"
        ((i++))
    done
    
    local choice
    local max=${#types[@]}
    while true; do
        read -p "Enter your choice (1-$max): " choice
        if [[ $choice =~ ^[0-9]+$ ]] && (( choice >= 1 && choice <= max )); then
            BUILD_TYPE="${types[$((choice-1))]}"
            break
        fi
        print_warning "Invalid choice. Please enter a number between 1 and $max."
    done
    
    print_info "Selected build type: $BUILD_TYPE"
}

check_configs() {
    local static_config_path="$CONFIG_DIR/$STATIC_CONFIG"
    
    if [[ ! -f "$static_config_path" ]]; then
        print_warning "Static config file not found: $static_config_path"
        print_warning "Creating a default static_config.yaml file"
        
        # Create configs directory if it doesn't exist
        mkdir -p "$CONFIG_DIR"
        
        # Create a basic static config
        cat > "$static_config_path" << EOL
components_manager:
  task_processors:
    main-task-processor:
      worker_threads: 4
    fs-task-processor:
      worker_threads: 2

  default_task_processor: main-task-processor

  components:
    server:
      listener:
        port: 8080
        task_processor: main-task-processor
      listener-monitor:
        port: 8085
        task_processor: main-task-processor

    session-manager:
      max-sessions: 10000

    tests-control:
      path: /tests/{action}
      task_processor: main-task-processor
      method: POST

    testsuite-support: {}

    logging:
      fs-task-processor: fs-task-processor
      loggers:
        default:
          file_path: '@stderr'
          level: info
          overflow_behavior: discard
        
        session-logger:
          file_path: 'logs/session.log'
          level: debug
          overflow_behavior: discard
        
        http-logger:
          file_path: 'logs/http.log'
          level: info
          overflow_behavior: discard
        
        error-logger:
          file_path: 'logs/errors.log'
          level: error
          overflow_behavior: discard

    # HTTP Client components
    http-client:
      fs-task-processor: fs-task-processor
      user-agent: Contexto/1.0
      dns_resolver: async
    
    # DNS Client
    dns-client:
      fs-task-processor: fs-task-processor
    
    # API handlers
    Contexto-new-game-handler:
      path: /api/new-game
      method: POST
      task_processor: main-task-processor
      log-level: DEBUG

    Contexto-guess-handler:
      path: /api/guess
      method: POST
      task_processor: main-task-processor
      log-level: DEBUG

    # Ping handler
    ping:
      path: /ping
      method: GET
      task_processor: main-task-processor
      log-level: INFO
EOL
    fi
    
    # Return only the static config path
    echo "$static_config_path"
}

main() {
    print_info "Contexto Backend Run Script"
    print_info "==========================="
    
    parse_args "$@"
    
    # Interactive mode - if running in a terminal and interactive flag was set
    if [[ $INTERACTIVE -eq 1 ]]; then
        print_info "Running in interactive mode"
        
        # Read available types into an array, one per line
        mapfile -t available_types < <(list_available_build_types)
        
        if [[ ${#available_types[@]} -eq 0 ]]; then
            print_error "No built executables found in bin/ directory."
            print_warning "Please build the project first with ./scripts/build.sh"
            exit 1
        else
            if [[ ${#available_types[@]} -gt 1 ]]; then
                select_from_available_build_types "${available_types[@]}"
            else
                BUILD_TYPE="${available_types[0]}"
                print_info "Using the only available build type: $BUILD_TYPE"
            fi
        fi
    fi
        
    # Check config files - but only care about static config
    static_config=$(check_configs)
    print_info "Using static config: $static_config"
    
    # Find executable
    local executable=$(find_executable)
    if [[ $? -ne 0 ]]; then
        print_error "Could not find executable for build type: $BUILD_TYPE"
        exit 1
    fi

    print_info "Found executable: $executable"

    # Check if the executable exists
    if [[ ! -f "$executable" ]]; then
        print_error "Executable file not found: $executable"
        exit 1
    fi

    # Add execute permission if needed
    if [[ ! -x "$executable" ]]; then
        print_warning "Adding execute permission to $executable"
        chmod +x "$executable" || {
            print_error "Failed to add execute permission"
            exit 1
        }
    fi

    # Build command array
    local cmd=("$executable")

    # Add static config
    if [[ -f "$static_config" ]]; then
        cmd+=("--config" "$static_config")
        print_info "Using config: $static_config"
    fi

    # Add additional arguments
    for arg in "${ARGS[@]}"; do
        cmd+=("$arg")
    done

    # Log the full command
    print_info "Executing command:"
    echo "${cmd[*]}"

    # Execute
    "${cmd[@]}"
    local exit_code=$?
    
    if [[ $exit_code -ne 0 ]]; then
        print_error "Executable exited with code: $exit_code"
        exit $exit_code
    fi
}

main "$@"