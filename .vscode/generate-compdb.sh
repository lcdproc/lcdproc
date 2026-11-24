#!/bin/sh
set -e

if [ ! -f compile_commands.json ] || [ Makefile -nt compile_commands.json ]; then
    echo "Regenerating compile_commands.json..."
    make compile_commands
else
    echo "compile_commands.json is up to date"
fi