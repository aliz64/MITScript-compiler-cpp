#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
PARSER=$ROOT/alternate_implem/vm/binaries/mitscript

if [ ! -f "$PARSER" ]; then
    echo "VM not found!"
    exit 1
fi

for filename in $ROOT/alternate_implem/tests/interpreter/good*.mit; do
    echo "Test - $(basename $filename)"
    $PARSER $filename -s 
done
