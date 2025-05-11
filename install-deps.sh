#!/bin/bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Use local userver path instead of remote URL
USERVER_PATH="backend/third-party/userver/scripts/docs/en/deps"

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
    echo -e "${YELLOW}$1${NC}"
}

# Check if userver submodule exists
check_userver_submodule() {
    if [ ! -d "$USERVER_PATH" ]; then
        print_error "userver submodule not found at $USERVER_PATH"
        print_info "Make sure the git submodule is initialized with:"
        print_info "  git submodule update --init --recursive"
        exit 1
    fi
}

# Detect OS and version
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        VERSION_ID=$VERSION_ID
        OS_LIKE=$ID_LIKE
    elif [ -f /etc/lsb-release ]; then
        . /etc/lsb-release
        OS=$DISTRIB_ID
        VERSION_ID=$DISTRIB_RELEASE
        OS_LIKE=""
    elif [ "$(uname)" == "Darwin" ]; then
        OS="macos"
        VERSION_ID=$(sw_vers -productVersion)
        OS_LIKE=""
    else
        print_error "Unable to detect operating system"
        exit 1
    fi
}

get_base_distro() {
    local distro=$1
    local like=$2

    # Check if the distro is directly supported
    case $distro in
        ubuntu|debian|fedora|arch|manjaro|gentoo|macos)
            echo "$distro"
            return
            ;;
    esac

    # If not directly supported, check OS_LIKE
    if [ -n "$like" ]; then
        # Split OS_LIKE on spaces and check each base distro
        for base in $like; do
            case $base in
                arch)
                    echo "arch"
                    return
                    ;;
                debian)
                    echo "debian"
                    return
                    ;;
                fedora)
                    echo "fedora"
                    return
                    ;;
                ubuntu)
                    echo "ubuntu"
                    return
                    ;;
            esac
        done
    fi

    # If no match found
    echo ""
}

install_ubuntu() {
    local version=$1
    local deps_file="$USERVER_PATH/ubuntu-$version.md"

    # Fallback to latest LTS if exact version not found
    if [ ! -f "$deps_file" ]; then
        print_warning "Dependencies file not found for Ubuntu $version, falling back to 22.04..."
        deps_file="$USERVER_PATH/ubuntu-22.04.md"
    fi

    if [ ! -f "$deps_file" ]; then
        print_error "Dependencies file not found: $deps_file"
        exit 1
    fi

    print_info "Installing dependencies for Ubuntu-based system..."
    # shellcheck disable=SC2046
    sudo apt install --allow-downgrades -y $(cat "$deps_file" | tr '\n' ' ')

    # Install Eigen specifically
    print_info "Installing Eigen library..."
    sudo apt install -y libeigen3-dev
}

install_debian() {
    local version=$1
    local deps_file="$USERVER_PATH/debian-11.md"  # Default to Debian 11

    if [ ! -f "$deps_file" ]; then
        print_error "Dependencies file not found: $deps_file"
        exit 1
    fi

    print_info "Installing dependencies for Debian-based system..."
    # shellcheck disable=SC2046
    sudo apt install --allow-downgrades -y $(cat "$deps_file" | tr '\n' ' ')

    # Install Eigen specifically
    print_info "Installing Eigen library..."
    sudo apt install -y libeigen3-dev
}

install_fedora() {
    local version=$1
    local deps_file="$USERVER_PATH/fedora-36.md"  # Default to latest

    if [ ! -f "$deps_file" ]; then
        print_error "Dependencies file not found: $deps_file"
        exit 1
    fi

    print_info "Installing dependencies for Fedora-based system..."
    # shellcheck disable=SC2046
    sudo dnf install -y $(cat "$deps_file" | tr '\n' ' ')

    # Install Eigen specifically
    print_info "Installing Eigen library..."
    sudo dnf install -y eigen3-devel
}

install_arch() {
    local deps_file="$USERVER_PATH/arch.md"

    if [ ! -f "$deps_file" ]; then
        print_error "Dependencies file not found: $deps_file"
        exit 1
    fi

    print_info "Installing dependencies for Arch-based system ($OS)..."
    print_info "Using dependencies from $deps_file"

    # Read the dependencies file
    local deps_content
    deps_content=$(cat "$deps_file")

    # Get all non-AUR packages first
    local regular_packages
    regular_packages=$(echo "$deps_content" | grep -v '^makepkg|' || true)

    # Array to store failed packages
    declare -a failed_packages

    if [ -n "$regular_packages" ]; then
        print_info "Checking and installing packages from official repositories..."

        # Try installing each package individually to track failures
        for pkg in $regular_packages; do
            # Check if package is already installed
            if pacman -Qi "$pkg" >/dev/null 2>&1; then
                print_info "Package $pkg is already installed"
                continue
            fi

            # Check if package exists in repositories
            if ! pacman -Si "$pkg" >/dev/null 2>&1; then
                print_warning "Package $pkg not found in official repositories, will try AUR"
                failed_packages+=("$pkg")
                continue
            fi

            # Try to install the package
            print_info "Installing $pkg..."
            if ! sudo pacman -S --needed --noconfirm "$pkg"; then
                print_warning "Failed to install $pkg, will try AUR"
                failed_packages+=("$pkg")
            else
                print_success "Successfully installed $pkg"
            fi
        done
    fi

    # Install failed packages from AUR
    if [ ${#failed_packages[@]} -gt 0 ]; then
        print_info "Installing failed packages from AUR: ${failed_packages[*]}"

        for pkg in "${failed_packages[@]}"; do
            if pacman -Qi "$pkg" >/dev/null 2>&1; then
                print_info "Package $pkg is already installed"
                continue
            fi

            print_info "Installing AUR package: $pkg"
            DIR=$(mktemp -d)
            print_info "Created temporary directory: $DIR"

            if git clone "https://aur.archlinux.org/$pkg.git" "$DIR"; then
                print_info "Successfully cloned $pkg repository"
                (cd "$DIR" && yes | makepkg -si --needed) || print_error "Failed to build/install $pkg"
                print_info "Cleaning up temporary directory"
                rm -rf "$DIR"
            else
                print_error "Failed to clone AUR repository for $pkg"
                rm -rf "$DIR"
            fi
        done
    fi

    # Install Eigen specifically
    print_info "Installing Eigen library..."
    if ! pacman -Qi "eigen" >/dev/null 2>&1; then
        sudo pacman -S --needed --noconfirm eigen
    else
        print_info "Eigen is already installed"
    fi

    print_success "Package installation completed!"
}

manual_aur_install_package() {
    local pkg="$1"

    if ! pacman -Qi "$pkg" >/dev/null 2>&1; then
        print_info "Installing AUR package: $pkg"
        DIR=$(mktemp -d)
        print_info "Created temporary directory: $DIR"

        if git clone "https://aur.archlinux.org/$pkg.git" "$DIR"; then
            print_info "Successfully cloned $pkg repository"
            (cd "$DIR" && makepkg -si --needed) || print_error "Failed to build/install $pkg"
            print_info "Cleaning up temporary directory"
            rm -rf "$DIR"
        else
            print_error "Failed to clone AUR repository for $pkg"
            rm -rf "$DIR"
        fi
    else
        print_info "Package $pkg is already installed"
    fi
}

install_gentoo() {
    local deps_file="$USERVER_PATH/gentoo.md"

    if [ ! -f "$deps_file" ]; then
        print_error "Dependencies file not found: $deps_file"
        exit 1
    fi

    print_info "Installing dependencies for Gentoo-based system..."
    # shellcheck disable=SC2046
    sudo emerge --ask --update --oneshot $(cat "$deps_file" | tr '\n' ' ')

    # Install Eigen specifically
    print_info "Installing Eigen library..."
    sudo emerge --ask dev-cpp/eigen
}

install_macos() {
    local deps_file="$USERVER_PATH/macos.md"

    if [ ! -f "$deps_file" ]; then
        print_error "Dependencies file not found: $deps_file"
        exit 1
    fi

    if ! command -v brew >/dev/null 2>&1; then
        print_error "Homebrew is required for macOS. Please install it first:"
        echo "https://brew.sh/"
        exit 1
    fi

    print_info "Installing dependencies for macOS..."
    # shellcheck disable=SC2046
    brew install $(cat "$deps_file" | tr '\n' ' ')

    # Install Eigen specifically
    print_info "Installing Eigen library..."
    brew install eigen
}

main() {
    # First, check if userver submodule exists
    check_userver_submodule

    print_info "Detecting system..."
    detect_os

    print_info "Detected OS: $OS"
    [ -n "$OS_LIKE" ] && print_info "OS is based on: $OS_LIKE"

    # Get base distribution
    BASE_DISTRO=$(get_base_distro "$OS" "$OS_LIKE")

    if [ -z "$BASE_DISTRO" ]; then
        print_error "Unsupported operating system: $OS"
        echo "Please check manual installation instructions at:"
        echo "https://github.com/userver-framework/userver/tree/develop/scripts/docs/en/deps"
        exit 1
    fi

    print_info "Using package installation method for: $BASE_DISTRO"

    case $BASE_DISTRO in
        ubuntu)
            install_ubuntu "$VERSION_ID"
            ;;
        debian)
            install_debian "$VERSION_ID"
            ;;
        fedora)
            install_fedora "$VERSION_ID"
            ;;
        arch)
            install_arch
            ;;
        gentoo)
            install_gentoo
            ;;
        macos)
            install_macos
            ;;
        *)
            print_error "Unsupported operating system: $OS"
            echo "Please check manual installation instructions at:"
            echo "https://github.com/userver-framework/userver/tree/develop/scripts/docs/en/deps"
            exit 1
            ;;
    esac

    print_success "Dependencies installed successfully for $OS (based on $BASE_DISTRO)!"
}

main "$@"
