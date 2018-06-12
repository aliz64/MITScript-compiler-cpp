#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
PARSER=$ROOT/alternate_implem/vm/binaries/mitscript

if [ ! -f "$PARSER" ]; then
    echo "VM not found!"
    exit 1
fi

for filename in $ROOT/alternate_implem/tests/interpreter/bad*.mit; do
    echo "Test - $(basename $filename)"
    if $PARSER $filename -s > tmp.out; then
        echo "Test Failed"
    else
        echo "Test Passed"
    fi
    echo ""
    rm tmp.out
done

for filename in $ROOT/alternate_implem/tests/interpreter/good*.mit; do
    echo "Test - $(basename $filename)"
    $PARSER $filename -s > tmp.out
    if diff tmp.out $ROOT/alternate_implem/tests/interpreter/$(basename $filename .mit).output; then
        echo "Test Passed"
    else
        echo "Test Failed"
    fi
    echo ""
    rm tmp.out
done
