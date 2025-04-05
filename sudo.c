#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define N 9

typedef enum {
    Easy   = 20,
    Medium = 40,
    Hard   = 60,
} Difficulty;

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

void shuffle_numbers(size_t *array, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        size_t j = rand() % size;
        size_t temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
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

    size_t numbers[N];
    for (size_t i = 0; i < N; ++i) {
        numbers[i] = i+1;
    }

    shuffle_numbers(numbers, N);

    for (size_t i = 0; i < N; ++i) {
        size_t num = numbers[i];
        if (is_safe(grid, row, col, num)) {
            grid[row][col] = num;
            if (fill_grid(grid)) return 1;
            grid[row][col] = 0;
        }
    }

    return 0;
}

void print_grid_stdout(size_t grid[N][N])
{
    for (size_t row = 0; row < N; ++row) {
        for (size_t col = 0; col < N; ++col) {
            printf("%zu ", grid[row][col]);
        }
        printf("\n");
    }
}
#define print_grid_ln(grid) print_grid_stdout(grid); printf("\n");

void remove_numbers(size_t grid[N][N], size_t difficulty)
{
    while (difficulty != 0) {
        size_t i = rand() % N;
        size_t j = rand() % N;
        if (grid[i][j] != 0) {
            grid[i][j] = 0;
            --difficulty;
        }
    }
}

void print_grid_stdscr(WINDOW *win, size_t grid[N][N])
{
    for (size_t row = 0; row < N; ++row) {
        for (size_t col = 0; col < N; ++col) {
            size_t y = row * 2 + 1;
            size_t x = col * 4 + 1;

            if (row % 3 == 0) {
                mvwprintw(win, y, x, "=====");
            } else {
                mvwprintw(win, y, x, "- - -");
            }

            if (col % 3 == 0 || col == 0)
                (grid[row][col] == 0) ? mvwprintw(win, y + 1, x, "|   |") : mvwprintw(win, y + 1, x, "| %zu |", grid[row][col]);
            else if (col + 1 == N)
                (grid[row][col] == 0) ? mvwprintw(win, y + 1, x, "    |") : mvwprintw(win, y + 1, x, "  %zu |", grid[row][col]);
            else
                (grid[row][col] == 0) ? mvwprintw(win, y + 1, x, "     ") : mvwprintw(win, y + 1, x, "  %zu  ", grid[row][col]);

            if (row + 1 == N) mvwprintw(win, y + 2, x, "=====");
        }
    }
}

void draw_grid(WINDOW *win, size_t grid[N][N], size_t cursor_row, size_t cursor_col) {
    // werase(win);

    box(win, 0, 0);
    mvwprintw(win, 0, 0, "sudo");

    print_grid_stdscr(win, grid);

    size_t highlight_y = cursor_row * 2 + 1;
    size_t highlight_x = cursor_col * 4 + 1;

    wmove(win, highlight_y, highlight_x);

    wattron(win, A_REVERSE); // "highlights" current cell
    /* mvwprintw(win, highlight_y, highlight_x, "-----"); */
    if (grid[cursor_row][cursor_col] == 0)
        mvwprintw(win, highlight_y + 1, highlight_x, "|   |");
    else
        mvwprintw(win, highlight_y + 1, highlight_x, "| %zu |", grid[cursor_row][cursor_col]);

    /* mvwprintw(win, highlight_y + 2, highlight_x, "-----"); */
    wattroff(win, A_REVERSE);

    wrefresh(win);
}

int main(void)
{
    srand(time(0));

    size_t grid[N][N] = {0};
    fill_grid(grid);
    remove_numbers(grid, Hard);
    print_grid_ln(grid);

    const char *INIT_TEXT    = "Press any key to start..";
    const char *INVALID_MOVE = "Invalid move";

    /* ----------------------  */

    initscr();
    noecho();
    keypad(stdscr, TRUE);
    cbreak();
    curs_set(0);

    if (has_colors()) {
        // start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        // attrset(COLOR_PAIR(1 or 2));
    }

    WINDOW *win = newwin(N * 2 + 3, N * 4 + 3, (LINES / N * 2), (COLS / N * 2));
    size_t cursor_row= 0, cursor_col = 0;

    size_t c;
    size_t quit = 0;

    mvwprintw(stdscr, LINES / 2, COLS / 2, "%s", INIT_TEXT);

    while (!quit) {
        draw_grid(win, grid, cursor_row, cursor_col);

        c = getch();
        if (c) {
            mvwprintw(stdscr, LINES / 2, COLS / 2, "%*c", (int)strlen(INIT_TEXT), ' ');
            mvwprintw(stdscr, N + 1, 1, "%*c", (int)strlen(INVALID_MOVE), ' ');
        }
        switch (c) {
        case KEY_UP:
            if (cursor_row > 0) {
                --cursor_row;
            }
            break;
        case KEY_DOWN:
            if (cursor_row < N - 1) {
                ++cursor_row;
            }
            break;
        case KEY_LEFT:
            if (cursor_col > 0) {
                --cursor_col;
            }
            break;
        case KEY_RIGHT:
            if (cursor_col < N - 1) {
                ++cursor_col;
            }
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if (is_safe(grid, cursor_row, cursor_col, c - '0'))
                grid[cursor_row][cursor_col] = c - '0';
            else
                mvwprintw(stdscr, LINES / 2, COLS / 2, "%s", INVALID_MOVE);
            break;
        case 'q': // quit
            quit = 1;
            break;
        default:
            break;
        }
    }

    delwin(win);
    endwin();
    printf("endwin();\n");

    return 0;
}
