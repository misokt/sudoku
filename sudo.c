// #include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 9

enum Difficulty {
    easy   = 20,
    medium = 40,
    hard   = 60,
};

int is_safe(size_t grid[N][N], size_t row, size_t col, size_t num)
{
    for (size_t i = 0; i < N; ++i) {
        if (grid[row][i] == num || grid[i][col] == num) return 0;
    }

    size_t start_row = row - row % 3;
    size_t start_col = col - col % 3;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            if (grid[i + start_row][j + start_col] == num) return 0;
        }
    }
    return 1;
}

int fill_grid(size_t grid[N][N])
{
    size_t row, col;
    size_t is_empty = 1;

    for (row = 0; row < N; ++row) {
        for (col = 0; col < N; ++col) {
            if (grid[row][col] == 0) {
                is_empty = 0;
                break;
            }
        }
        if (!is_empty) break;
    }

    if (is_empty) return 1; // is solved

    size_t num = rand() % 9 + 1;
    for (num = 1; num <= N; ++num) {
        if (is_safe(grid, row, col, num)) {
            grid[row][col] = num;
            if (fill_grid(grid)) return 1;
            grid[row][col] = 0;
        }
    }

    return 0;
}

void print_grid(size_t grid[N][N])
{
    for (size_t row = 0; row < N; ++row) {
        for (size_t col = 0; col < N; ++col) {
            printf("%zu ", grid[row][col]);
        }
        printf("\n");
    }
}
#define print_grid_ln(grid) print_grid(grid); printf("\n");

void remove_numbers(size_t grid[N][N], size_t difficulty)
{
    size_t count = difficulty;
    while (count != 0) {
        size_t i = rand() % N;
        size_t j = rand() % N;
        if (grid[i][j] != 0) {
            grid[i][j] = 0;
            --count;
        }
    }
}

int main(void)
{
    srand(time(0));

    size_t grid[N][N] = {0};

    fill_grid(grid);
    print_grid_ln(grid);

    /* remove_numbers(grid, hard); */
    /* print_grid(grid); */

    return 0;
}
