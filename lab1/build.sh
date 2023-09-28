#!/bin/bash

set -euo pipefail
root_dir=$(dirname "${BASH_SOURCE[0]}")

cd "$root_dir"
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ..
mv build/mydaemon mydaemon
rm -rf build
