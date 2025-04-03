#!/bin/bash

set -xe

# cc -Wall -Wextra -o sudo sudo.c -lncurses
# ./sudo

cc -Wall -Wextra -o sudoku sudo.c -lncurses
./sudoku
