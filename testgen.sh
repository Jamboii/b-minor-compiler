#!/bin/sh

TESTDIR=$1
TESTFIL=$2;

./bminor -codegen tests/${TESTDIR}/codegen/good${TESTFIL}.bminor tests/${TESTDIR}/codegen/good${TESTFIL}.s

gcc -g tests/${TESTDIR}/codegen/good${TESTFIL}.s library.c -o tests/${TESTDIR}/codegen/good${TESTFIL}

echo ""

./tests/${TESTDIR}/codegen/good${TESTFIL}