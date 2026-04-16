#!/bin/bash
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

missing=()
for pkg in build-essential cmake pkg-config libcurl4-openssl-dev libsimgrid-dev; do
    dpkg -s "$pkg" >/dev/null 2>&1 || missing+=("$pkg")
done

if [ ${#missing[@]} -gt 0 ]; then
    echo ""
    echo "Faltam dependencias para compilar o projeto no Ubuntu/WSL:"
    printf ' - %s\n' "${missing[@]}"
    echo ""
    echo "Vou tentar instalar agora..."
    sudo apt-get update
    sudo apt-get install -y "${missing[@]}"
fi

if ! pkg-config --exists simgrid; then
    echo ""
    echo "Nao consegui localizar o SimGrid pelo pkg-config."
    echo "Confira se o pacote libsimgrid-dev realmente foi instalado."
    exit 1
fi

echo "Usando SimGrid $(pkg-config --modversion simgrid)"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake ..
make -j"$(nproc)"
./pvfirst --log=host_energy.thres:critical "$@"