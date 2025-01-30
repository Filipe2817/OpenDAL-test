#!/bin/bash

# Exit on error and undefined variables
set -eu

show_usage() {
    echo "Usage: $0 <command>"
    echo "Commands:"
    echo "  build    Build the OpenDAL C binding"
    echo "  clean    Clean build artifacts"
    echo "  help     Show this help message"
}

navigate() {
    cd "${OPENDAL_PATH}/bindings/c" || { 
        echo "Failed to access opendal C binding"
        exit 1
    }
}

build() {
    navigate
    echo "Building OpenDAL C binding..."
    
    mkdir -p build
    cd build
    cmake ..
    make
    
    echo "Build completed successfully."
}

clean() {
    navigate
    echo "Cleaning OpenDAL build artifacts..."

    if [ -d "build" ]; then
        cargo clean
        cd build
        make clean
        cd ..
        rm -rf build
    fi
    
    echo "Clean completed successfully."
}

###############################

SCRIPT_PATH="$(dirname "$0")"
PROJECT_ROOT="${SCRIPT_PATH}/.."

if [ ! -f "${PROJECT_ROOT}/.env" ]; then
    echo "Error: .env file not found"
    exit 1
fi

# shellcheck disable=SC1091
source "$PROJECT_ROOT/.env"

if [ -z "$OPENDAL_PATH" ]; then
    echo "Error: OPENDAL_PATH is not set in the .env file."
    exit 1
fi

if [ ! -d "$OPENDAL_PATH" ]; then
    echo "Error: Directory $OPENDAL_PATH does not exist."
    exit 1
fi

case "${1:-help}" in
    "build")
        build
        ;;
    "clean")
        clean
        ;;
    "help"|*)
        show_usage
        ;;
esac
