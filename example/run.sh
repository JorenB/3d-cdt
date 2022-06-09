#!/usr/bin/env bash

MAIN=3dcdt.x

rm -f ${MAIN} out/*.dat
make --no-print-directory -C ..
cp ../${MAIN} .
./${MAIN} config.dat
