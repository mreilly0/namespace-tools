#!/usr/bin/env sh
mkdir -p dist
gcc -O3 namespace/namespace.c -o dist/namespace
gcc -O3 namespacef/namespacef.c -o dist/namespacef
