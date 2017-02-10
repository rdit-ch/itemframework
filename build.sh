#!/bin/bash


OSNAME=linux
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then OSNAME=macx; fi

COMPILER=clang
if [[ "$CC" == *gcc ]]; then COMPILER=g++; fi


SPEC=$OSNAME-$COMPILER
MODE=debug
CORES=4

echo "Building itemframework with $CORES Cores and Mkspec $SPEC"
qmake itemframework.pro -r -spec $SPEC CONFIG+=$MODE && make -j $CORES
exit

