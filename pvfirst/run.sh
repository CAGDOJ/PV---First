#!/bin/bash
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

CMAKE_LOG="$BUILD_DIR/cmake.log"
MAKE_LOG="$BUILD_DIR/make.log"

NEED_CONFIGURE=0

if [ ! -f Makefile ]; then
    NEED_CONFIGURE=1
fi

if [ -f "$ROOT_DIR/CMakeLists.txt" ] && [ -f Makefile ]; then
    if [ "$ROOT_DIR/CMakeLists.txt" -nt Makefile ]; then
        NEED_CONFIGURE=1
    fi
fi

echo "Preparando ambiente..."

if [ "$NEED_CONFIGURE" -eq 1 ]; then
    if ! cmake .. >"$CMAKE_LOG" 2>&1; then
        echo ""
        echo "Erro na configuracao do projeto."
        echo "Abaixo esta o log do CMake:"
        echo "------------------------------------------------------------"
        cat "$CMAKE_LOG"
        echo "------------------------------------------------------------"
        exit 1
    fi
fi

if ! make -s -j"$(nproc)" >"$MAKE_LOG" 2>&1; then
    echo ""
    echo "Erro na compilacao do projeto."
    echo "Abaixo esta o log do make:"
    echo "------------------------------------------------------------"
    cat "$MAKE_LOG"
    echo "------------------------------------------------------------"
    exit 1
fi

cd "$ROOT_DIR"

if [ -t 1 ]; then
    clear
fi

./build/pvfirst "$@"