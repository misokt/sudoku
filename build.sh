#!/bin/bash

set -xe

ARG=$1

if [[ $(date +%d) == "01" ]] && [[ $(date +%m) == "04" ]]; then
    PROGRAM="sudo"
else
    PROGRAM="sudoku"
fi

cc -Wall -Wextra -o $PROGRAM ./sudoku.c -lncurses

if [[ $ARG == "run" ]]; then
    ./$PROGRAM
fi
