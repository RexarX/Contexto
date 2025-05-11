# Shell configuration
SHELL := /bin/bash
.SHELLFLAGS := -ec

# Default target when just running "make"
.PHONY: all
all: help

# Constants and configurable variables
PROJECT_NAME := contexto

# Docker configuration
DOCKER_COMPOSE := docker compose
DOCKER_COMPOSE_FILE := docker-compose.yml

# Build configuration
BUILD_TYPE ?= Release  # Default build type (can be overridden with make BUILD_TYPE=Debug ...)
COMPILER ?= gcc        # Default compiler
BUILD_SYSTEM ?= ninja  # Default build system
PARALLEL_JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) # Auto-detect CPU cores

# Directories
ROOT_DIR := $(shell pwd)
BACKEND_DIR := $(ROOT_DIR)/backend
FRONTEND_DIR := $(ROOT_DIR)/frontend
BUILD_DIR := $(BACKEND_DIR)/build

# Colors for terminal output
BLUE := \033[1;34m
GREEN := \033[0;32m
YELLOW := \033[1;33m
RED := \033[0;31m
RESET := \033[0m

# Display helper information
.PHONY: help
help:
	@echo -e "$(BLUE)$(PROJECT_NAME) Development Commands$(RESET)"
	@echo -e "$(BLUE)=============================$(RESET)"
	@echo ""
	@echo -e "$(YELLOW)Development Commands:$(RESET)"
	@echo -e "  $(GREEN)make init$(RESET)             - Initialize project (git submodules & dependencies)"
	@echo -e "  $(GREEN)make deps$(RESET)             - Install dependencies"
	@echo -e "  $(GREEN)make build$(RESET)            - Build the backend (default build type)"
	@echo -e "  $(GREEN)make build-debug$(RESET)      - Build Debug configuration"
	@echo -e "  $(GREEN)make build-release$(RESET)    - Build Release configuration"
	@echo -e "  $(GREEN)make build-relwithdebinfo$(RESET) - Build RelWithDebInfo configuration"
	@echo -e "  $(GREEN)make run$(RESET)              - Run the backend (default build type)"
	@echo -e "  $(GREEN)make run-debug$(RESET)        - Run Debug configuration"
	@echo -e "  $(GREEN)make run-release$(RESET)      - Run Release configuration"
	@echo -e "  $(GREEN)make run-relwithdebinfo$(RESET) - Run RelWithDebInfo configuration"
	@echo -e "  $(GREEN)make frontend$(RESET)         - Run the frontend development server"
	@echo -e "  $(GREEN)make test$(RESET)             - Run backend tests (default build type)"
	@echo -e "  $(GREEN)make test-debug$(RESET)       - Run tests for Debug build"
	@echo -e "  $(GREEN)make test-release$(RESET)     - Run tests for Release build"
	@echo -e "  $(GREEN)make test-relwithdebinfo$(RESET) - Run tests for RelWithDebInfo build"
	@echo -e "  $(GREEN)make test-filter$(RESET)      - Run specific tests (FILTER=pattern)"
	@echo -e "  $(GREEN)make clean$(RESET)            - Clean build artifacts"
	@echo ""
	@echo -e "$(YELLOW)Docker Commands:$(RESET)"
	@echo -e "  $(GREEN)make docker-build$(RESET)     - Build Docker images"
	@echo -e "  $(GREEN)make docker-up$(RESET)        - Start backend and frontend containers"
	@echo -e "  $(GREEN)make docker-down$(RESET)      - Stop all Docker containers"
	@echo -e "  $(GREEN)make docker-backend$(RESET)   - Start backend container only"
	@echo -e "  $(GREEN)make docker-backend-debug$(RESET)   - Start backend container with Debug build"
	@echo -e "  $(GREEN)make docker-backend-release$(RESET)   - Start backend container with Release build"
	@echo -e "  $(GREEN)make docker-backend-relwithdebinfo$(RESET)   - Start backend with RelWithDebInfo build"
	@echo -e "  $(GREEN)make docker-frontend$(RESET)  - Start frontend container only"
	@echo -e "  $(GREEN)make docker-dev$(RESET)       - Start dev container with shell"
	@echo -e "  $(GREEN)make docker-test$(RESET)      - Run tests in Docker (default build type)"
	@echo -e "  $(GREEN)make docker-test-debug$(RESET) - Run Debug tests in Docker"
	@echo -e "  $(GREEN)make docker-test-release$(RESET) - Run Release tests in Docker"
	@echo -e "  $(GREEN)make docker-test-all$(RESET)  - Run all test configurations in Docker"
	@echo -e "  $(GREEN)make docker-clean$(RESET)     - Clean Docker resources"
	@echo ""
	@echo -e "$(YELLOW)Advanced Testing:$(RESET)"
	@echo -e "  $(GREEN)bash scripts/test_matrix.sh$(RESET) - Run tests across multiple configurations"
	@echo -e "  $(GREEN)bash scripts/test_matrix.sh --filter utf8$(RESET) - Run specific tests across configurations"
	@echo ""
	@echo -e "$(YELLOW)Build Options:$(RESET)"
	@echo -e "  $(GREEN)BUILD_TYPE=$(RESET)$(BUILD_TYPE)       (Debug|Release|RelWithDebInfo)"
	@echo -e "  $(GREEN)COMPILER=$(RESET)$(COMPILER)         (gcc|clang)"
	@echo -e "  $(GREEN)BUILD_SYSTEM=$(RESET)$(BUILD_SYSTEM)     (ninja|make)"
	@echo -e "  $(GREEN)PARALLEL_JOBS=$(RESET)$(PARALLEL_JOBS)"
	@echo ""
	@echo -e "Examples:"
	@echo -e "  $(GREEN)make build BUILD_TYPE=Debug$(RESET)"
	@echo -e "  $(GREEN)make docker-up$(RESET)"
	@echo -e "  $(GREEN)make test-filter FILTER=utf8$(RESET)"
	@echo -e "  $(GREEN)make docker-backend-debug$(RESET)"

# Initialize project
.PHONY: init
init:
	@echo -e "$(BLUE)Initializing project...$(RESET)"
	git submodule update --init --recursive
	$(MAKE) deps

# Install dependencies
.PHONY: deps
deps:
	@echo -e "$(BLUE)Installing dependencies...$(RESET)"
	./install-deps.sh

# Build backend
.PHONY: build
build:
	@echo -e "$(BLUE)Building backend ($(BUILD_TYPE))...$(RESET)"
	cd $(BACKEND_DIR) && ./scripts/build.sh --type $(BUILD_TYPE) \
		--compiler $(COMPILER) \
		--build-system $(BUILD_SYSTEM) \
		--jobs $(PARALLEL_JOBS)
	@echo -e "$(GREEN)Build successful!$(RESET)"

# Build specific build types
.PHONY: build-debug build-release build-relwithdebinfo
build-debug:
	@echo -e "$(BLUE)Building Debug configuration...$(RESET)"
	$(MAKE) build BUILD_TYPE=Debug

build-release:
	@echo -e "$(BLUE)Building Release configuration...$(RESET)"
	$(MAKE) build BUILD_TYPE=Release

build-relwithdebinfo:
	@echo -e "$(BLUE)Building RelWithDebInfo configuration...$(RESET)"
	$(MAKE) build BUILD_TYPE=RelWithDebInfo

# Run backend
.PHONY: run
run:
	@echo -e "$(BLUE)Running backend...$(RESET)"
	cd $(BACKEND_DIR) && ./scripts/run.sh --type $(BUILD_TYPE)

# Run specific build types
.PHONY: run-debug run-release run-relwithdebinfo
run-debug:
	@echo -e "$(BLUE)Running Debug configuration...$(RESET)"
	$(MAKE) run BUILD_TYPE=Debug

run-release:
	@echo -e "$(BLUE)Running Release configuration...$(RESET)"
	$(MAKE) run BUILD_TYPE=Release

run-relwithdebinfo:
	@echo -e "$(BLUE)Running RelWithDebInfo configuration...$(RESET)"
	$(MAKE) run BUILD_TYPE=RelWithDebInfo

# Run frontend development server
.PHONY: frontend
frontend:
	@echo -e "$(BLUE)Starting frontend development server...$(RESET)"
	cd $(FRONTEND_DIR) && npm run dev

# Run backend tests
.PHONY: test
test:
	@echo -e "$(BLUE)Building and running tests ($(BUILD_TYPE))...$(RESET)"
	cd $(BACKEND_DIR) && ./scripts/build.sh --type $(BUILD_TYPE) \
		--compiler $(COMPILER) \
		--build-system $(BUILD_SYSTEM) \
		--jobs $(PARALLEL_JOBS) \
		--tests \
		--no-run
	cd $(BACKEND_DIR) && ./scripts/run_tests.sh --type $(BUILD_TYPE) --external-build

# Testing specific build types
.PHONY: test-debug test-release test-relwithdebinfo
test-debug:
	@echo -e "$(BLUE)Testing Debug configuration...$(RESET)"
	$(MAKE) test BUILD_TYPE=Debug

test-release:
	@echo -e "$(BLUE)Testing Release configuration...$(RESET)"
	$(MAKE) test BUILD_TYPE=Release

test-relwithdebinfo:
	@echo -e "$(BLUE)Testing RelWithDebInfo configuration...$(RESET)"
	$(MAKE) test BUILD_TYPE=RelWithDebInfo

# Run tests matching a pattern
# Usage: make test-filter FILTER=utf8
.PHONY: test-filter
test-filter:
	@if [ -z "$(FILTER)" ]; then \
		echo -e "$(RED)Error: FILTER parameter is required$(RESET)"; \
		echo -e "Example: make test-filter FILTER=utf8"; \
		exit 1; \
	fi
	@echo -e "$(BLUE)Running tests matching '$(FILTER)'...$(RESET)"
	cd $(BACKEND_DIR) && ./scripts/run_tests.sh --type $(BUILD_TYPE) --filter "$(FILTER)"

# Run tests with verbose output
.PHONY: test-verbose
test-verbose:
	@echo -e "$(BLUE)Running tests with verbose output...$(RESET)"
	cd $(BACKEND_DIR) && ./scripts/run_tests.sh --type $(BUILD_TYPE) --verbose

# Make sure other containers aren't running
.PHONY: _ensure_clean_containers
_ensure_clean_containers:
	@echo -e "$(BLUE)Ensuring clean container environment...$(RESET)"
	$(DOCKER_COMPOSE) down 2>/dev/null || true

# Run backend tests in Docker
.PHONY: docker-test
docker-test: _ensure_clean_containers
	@echo -e "$(BLUE)Running tests in Docker with $(BUILD_TYPE) build...$(RESET)"
	BUILD_TYPE=$(BUILD_TYPE) $(DOCKER_COMPOSE) run --rm test \
		/bin/bash -c "cd /app/backend && ./scripts/build.sh --type $(BUILD_TYPE) --tests --no-run && ./scripts/run_tests.sh --type $(BUILD_TYPE) --external-build --format junit"
	@$(DOCKER_COMPOSE) down

# Convenience shortcuts for specific build types
.PHONY: docker-test-debug docker-test-release docker-test-relwithdebinfo docker-test-all
docker-test-debug:
	@echo -e "$(BLUE)Running Debug tests in Docker...$(RESET)"
	$(MAKE) docker-test BUILD_TYPE=Debug

docker-test-release:
	@echo -e "$(BLUE)Running Release tests in Docker...$(RESET)"
	$(MAKE) docker-test BUILD_TYPE=Release

docker-test-relwithdebinfo:
	@echo -e "$(BLUE)Running RelWithDebInfo tests in Docker...$(RESET)"
	$(MAKE) docker-test BUILD_TYPE=RelWithDebInfo

docker-test-all: _ensure_clean_containers
	@echo -e "$(BLUE)Running all test configurations in Docker...$(RESET)"
	BUILD_TYPE=Debug $(DOCKER_COMPOSE) run --rm test \
		/bin/bash -c "cd /app/backend && ./scripts/build.sh --type Debug --tests --no-run && ./scripts/run_tests.sh --type Debug --external-build --format junit"
	$(DOCKER_COMPOSE) down
	BUILD_TYPE=Release $(DOCKER_COMPOSE) run --rm test \
		/bin/bash -c "cd /app/backend && ./scripts/build.sh --type Release --tests --no-run && ./scripts/run_tests.sh --type Release --external-build --format junit"
	$(DOCKER_COMPOSE) down
	BUILD_TYPE=RelWithDebInfo $(DOCKER_COMPOSE) run --rm test \
		/bin/bash -c "cd /app/backend && ./scripts/build.sh --type RelWithDebInfo --tests --no-run && ./scripts/run_tests.sh --type RelWithDebInfo --external-build --format junit"
	$(DOCKER_COMPOSE) down

# Run tests with specific filter in Docker
.PHONY: docker-test-filter
docker-test-filter: _ensure_clean_containers
	@if [ -z "$(FILTER)" ]; then \
		echo -e "$(RED)Error: FILTER parameter is required$(RESET)"; \
		echo -e "Example: make docker-test-filter FILTER=utf8 BUILD_TYPE=Debug"; \
		exit 1; \
	fi
	@echo -e "$(BLUE)Running tests matching '$(FILTER)' with $(BUILD_TYPE) build in Docker...$(RESET)"
	BUILD_TYPE=$(BUILD_TYPE) $(DOCKER_COMPOSE) run --rm test \
		/bin/bash -c "cd /app/backend && ./scripts/build.sh --type $(BUILD_TYPE) --tests && ./scripts/run_tests.sh --type $(BUILD_TYPE) --format junit --filter \"$(FILTER)\""
	@$(DOCKER_COMPOSE) down

# Run quick test (without rebuilding)
.PHONY: test-quick
test-quick:
	@echo -e "$(BLUE)Running tests without rebuilding...$(RESET)"
	cd $(BACKEND_DIR) && ./scripts/run_tests.sh --type $(BUILD_TYPE) --no-build

# Clean build artifacts
.PHONY: clean
clean:
	@echo -e "$(BLUE)Cleaning build artifacts...$(RESET)"
	rm -rf $(BUILD_DIR)
	@echo -e "$(GREEN)Clean complete!$(RESET)"

# Docker commands
.PHONY: docker-build
docker-build:
	@echo -e "$(BLUE)Building Docker images...$(RESET)"
	$(DOCKER_COMPOSE) build

# Start services
.PHONY: docker-up
docker-up: _ensure_clean_containers
	@echo -e "$(BLUE)Starting backend and frontend containers ($(BUILD_TYPE) build)...$(RESET)"
	BUILD_TYPE=$(BUILD_TYPE) $(DOCKER_COMPOSE) up backend frontend

# Start services with specific build type
.PHONY: docker-up-debug docker-up-release docker-up-relwithdebinfo
docker-up-debug:
	@echo -e "$(BLUE)Starting Docker containers with Debug build...$(RESET)"
	$(MAKE) docker-up BUILD_TYPE=Debug

docker-up-release:
	@echo -e "$(BLUE)Starting Docker containers with Release build...$(RESET)"
	$(MAKE) docker-up BUILD_TYPE=Release

docker-up-relwithdebinfo:
	@echo -e "$(BLUE)Starting Docker containers with RelWithDebInfo build...$(RESET)"
	$(MAKE) docker-up BUILD_TYPE=RelWithDebInfo

# Start containers in background
.PHONY: docker-up-d
docker-up-d: _ensure_clean_containers
	@echo -e "$(BLUE)Starting backend and frontend containers in background ($(BUILD_TYPE) build)...$(RESET)"
	BUILD_TYPE=$(BUILD_TYPE) $(DOCKER_COMPOSE) up -d backend frontend

# Start containers in background with specific build type
.PHONY: docker-up-d-debug docker-up-d-release docker-up-d-relwithdebinfo
docker-up-d-debug:
	@echo -e "$(BLUE)Starting Docker containers in background with Debug build...$(RESET)"
	$(MAKE) docker-up-d BUILD_TYPE=Debug

docker-up-d-release:
	@echo -e "$(BLUE)Starting Docker containers in background with Release build...$(RESET)"
	$(MAKE) docker-up-d BUILD_TYPE=Release

docker-up-d-relwithdebinfo:
	@echo -e "$(BLUE)Starting Docker containers in background with RelWithDebInfo build...$(RESET)"
	$(MAKE) docker-up-d BUILD_TYPE=RelWithDebInfo

# Stop all services
.PHONY: docker-down
docker-down:
	@echo -e "$(BLUE)Stopping all Docker containers...$(RESET)"
	$(DOCKER_COMPOSE) down

# Start only the backend container
.PHONY: docker-backend
docker-backend: _ensure_clean_containers
	@echo -e "$(BLUE)Starting backend container ($(BUILD_TYPE) build)...$(RESET)"
	BUILD_TYPE=$(BUILD_TYPE) $(DOCKER_COMPOSE) run --service-ports backend

# Start backend container with specific build types
.PHONY: docker-backend-debug docker-backend-release docker-backend-relwithdebinfo
docker-backend-debug:
	@echo -e "$(BLUE)Starting backend container with Debug build...$(RESET)"
	$(MAKE) docker-backend BUILD_TYPE=Debug

docker-backend-release:
	@echo -e "$(BLUE)Starting backend container with Release build...$(RESET)"
	$(MAKE) docker-backend BUILD_TYPE=Release

docker-backend-relwithdebinfo:
	@echo -e "$(BLUE)Starting backend container with RelWithDebInfo build...$(RESET)"
	$(MAKE) docker-backend BUILD_TYPE=RelWithDebInfo

# Start only the frontend container
.PHONY: docker-frontend
docker-frontend: _ensure_clean_containers
	@echo -e "$(BLUE)Starting frontend container...$(RESET)"
	$(DOCKER_COMPOSE) up frontend

# Start dev container with shell access
.PHONY: docker-dev
docker-dev: _ensure_clean_containers
	@echo -e "$(BLUE)Starting dev container ($(BUILD_TYPE) build)...$(RESET)"
	BUILD_TYPE=$(BUILD_TYPE) $(DOCKER_COMPOSE) run --service-ports dev

# Start dev container with specific build types
.PHONY: docker-dev-debug docker-dev-release docker-dev-relwithdebinfo
docker-dev-debug:
	@echo -e "$(BLUE)Starting dev container with Debug build...$(RESET)"
	$(MAKE) docker-dev BUILD_TYPE=Debug

docker-dev-release:
	@echo -e "$(BLUE)Starting dev container with Release build...$(RESET)"
	$(MAKE) docker-dev BUILD_TYPE=Release

docker-dev-relwithdebinfo:
	@echo -e "$(BLUE)Starting dev container with RelWithDebInfo build...$(RESET)"
	$(MAKE) docker-dev BUILD_TYPE=RelWithDebInfo

# Clean Docker resources
.PHONY: docker-clean
docker-clean:
	@echo -e "$(BLUE)Cleaning Docker resources...$(RESET)"
	$(DOCKER_COMPOSE) down -v --rmi local
	@echo -e "$(GREEN)Docker clean complete!$(RESET)"

# Development environment setup - for new developers
.PHONY: setup-dev
setup-dev: init
	@echo -e "$(BLUE)Setting up development environment...$(RESET)"
	# Check if node and npm are installed
	@if ! command -v node > /dev/null; then \
		echo -e "$(YELLOW)Node.js not found. Please install Node.js$(RESET)"; \
		exit 1; \
	fi
	# Install frontend dependencies
	cd $(FRONTEND_DIR) && npm install
	# Create frontend environment file if it doesn't exist
	if [ ! -f "$(FRONTEND_DIR)/.env" ]; then \
		cp $(FRONTEND_DIR)/.env.example $(FRONTEND_DIR)/.env; \
		echo -e "$(GREEN)Created frontend .env file from example$(RESET)"; \
	fi
	@echo -e "$(GREEN)Development setup complete!$(RESET)"
