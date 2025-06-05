#!/bin/bash

set -xe

ARG=$1

if [[ $(date +%d) == "01" ]] && [[ $(date +%m) == "04" ]]; then
    PROGRAM="sudo"
else
    PROGRAM="sudoku"
fi

if [[ $ARG == "static" ]]; then
    PROGRAM="sudoku"
    cc -static ./sudoku.c -o $PROGRAM -lncursesw
    shasum -a 256 $PROGRAM > $PROGRAM".sum"
    sha256sum -c $PROGRAM".sum"
    exit 0
fi

cc -Wall -Wextra -o $PROGRAM ./sudoku.c -lncurses

if [[ $ARG == "run" ]]; then
    ./$PROGRAM
fi
