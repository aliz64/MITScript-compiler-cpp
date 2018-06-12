#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
PARSER=$ROOT/alternate_implem//parser/mitscript

#for filename in $ROOT/tests/interpreter/private/*.mit; do
#    echo "Test - $(basename $filename)"
#    if $PARSER $filename > tmp.out; then
#        echo "Test Failed"
#    else
#        echo "Test Passed"
#    fi
#    echo ""
#    rm tmp.out
#done

for filename in $ROOT/alternate_implem/tests/interpreter/private/*.mit; do
    echo "Test - $(basename $filename)"
    $PARSER $filename > tmp.out
    if diff tmp.out $ROOT/alternate_implem/tests/interpreter/private/$(basename $filename .mit).mit.out; then
        echo "Test Passed"
    else
        echo "Test Failed"
    fi
    echo ""
    rm tmp.out
done
