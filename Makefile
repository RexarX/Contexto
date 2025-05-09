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
	@echo -e "  $(GREEN)make build$(RESET)            - Build the backend"
	@echo -e "  $(GREEN)make run$(RESET)              - Run the backend"
	@echo -e "  $(GREEN)make frontend$(RESET)         - Run the frontend development server"
	@echo -e "  $(GREEN)make test$(RESET)             - Run backend tests"
	@echo -e "  $(GREEN)make clean$(RESET)            - Clean build artifacts"
	@echo ""
	@echo -e "$(YELLOW)Docker Commands:$(RESET)"
	@echo -e "  $(GREEN)make docker-build$(RESET)     - Build Docker images"
	@echo -e "  $(GREEN)make docker-up$(RESET)        - Start all Docker containers"
	@echo -e "  $(GREEN)make docker-down$(RESET)      - Stop all Docker containers"
	@echo -e "  $(GREEN)make docker-backend$(RESET)   - Start backend container only"
	@echo -e "  $(GREEN)make docker-frontend$(RESET)  - Start frontend container only"
	@echo -e "  $(GREEN)make docker-dev$(RESET)       - Start dev container with shell"
	@echo -e "  $(GREEN)make docker-test$(RESET)      - Run tests in Docker"
	@echo -e "  $(GREEN)make docker-clean$(RESET)     - Clean Docker resources"
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

# Run backend
.PHONY: run
run:
	@echo -e "$(BLUE)Running backend...$(RESET)"
	cd $(BACKEND_DIR) && ./scripts/run.sh --type $(BUILD_TYPE)

# Run frontend development server
.PHONY: frontend
frontend:
	@echo -e "$(BLUE)Starting frontend development server...$(RESET)"
	cd $(FRONTEND_DIR) && npm run dev

# Run backend tests
.PHONY: test
test:
	@echo -e "$(BLUE)Running tests...$(RESET)"
	cd $(BACKEND_DIR) && ./scripts/build.sh --type $(BUILD_TYPE) \
		--compiler $(COMPILER) \
		--build-system $(BUILD_SYSTEM) \
		--jobs $(PARALLEL_JOBS) \
		--tests
	cd $(BUILD_DIR) && ctest -V

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

# Start all services
.PHONY: docker-up
docker-up:
	@echo -e "$(BLUE)Starting all Docker containers...$(RESET)"
	$(DOCKER_COMPOSE) up

# Start containers in background
.PHONY: docker-up-d
docker-up-d:
	@echo -e "$(BLUE)Starting all Docker containers in background...$(RESET)"
	$(DOCKER_COMPOSE) up -d

# Stop all services
.PHONY: docker-down
docker-down:
	@echo -e "$(BLUE)Stopping all Docker containers...$(RESET)"
	$(DOCKER_COMPOSE) down

# Start only the backend container
.PHONY: docker-backend
docker-backend:
	@echo -e "$(BLUE)Starting backend container...$(RESET)"
	$(DOCKER_COMPOSE) up backend

# Start only the frontend container
.PHONY: docker-frontend
docker-frontend:
	@echo -e "$(BLUE)Starting frontend container...$(RESET)"
	$(DOCKER_COMPOSE) up frontend

# Start dev container with shell access
.PHONY: docker-dev
docker-dev:
	@echo -e "$(BLUE)Starting dev container...$(RESET)"
	$(DOCKER_COMPOSE) run --service-ports dev

# Run tests in Docker
.PHONY: docker-test
docker-test:
	@echo -e "$(BLUE)Running tests in Docker...$(RESET)"
	$(DOCKER_COMPOSE) run --rm backend /bin/bash -c "cd /app/backend && ./scripts/build.sh --type $(BUILD_TYPE) --compiler gcc --build-system ninja --tests && cd build && ctest -V"

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
