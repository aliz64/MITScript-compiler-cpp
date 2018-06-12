#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
PARSER=$ROOT/alternate_implem/vm/binaries/mitscript

for filename in $ROOT/alternate_implem/tests/interpreter/private/*.mit; do
    echo "Test - $(basename $filename)"
    $PARSER $filename -s > tmp.out
    if diff tmp.out $ROOT/alternate_implem/tests/interpreter/private/$(basename $filename .mit).mit.out; then
        echo "Test Passed"
    else
        echo "Test Failed"
    fi
    echo ""
    rm tmp.out
done
