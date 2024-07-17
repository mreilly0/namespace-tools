#!/usr/bin/env sh
mkdir -p dist
gcc namespace/namespace.c -o dist/namespace
gcc namespacef/namespacef.c -o dist/namespacef
