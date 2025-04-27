#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define N 9

typedef enum {
    EASY,
    MEDIUM,
    HARD,
    COUNT_DIFFICULTY
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

    size_t numbers[N] = {0};
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
    while (difficulty-- != 0) {
        size_t i = rand() % N;
        size_t j = rand() % N;
        if (grid[i][j] != 0)
            grid[i][j] = 0;
    }
}

void print_grid_window(WINDOW *win, size_t grid[N][N])
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

            size_t cell_value = grid[row][col];
            if (col % 3 == 0 || col == 0)
                (cell_value == 0) ? mvwprintw(win, y + 1, x, "|   |") : mvwprintw(win, y + 1, x, "| %zu |", cell_value);
            else if (col + 1 == N)
                (cell_value == 0) ? mvwprintw(win, y + 1, x, "    |") : mvwprintw(win, y + 1, x, "  %zu |", cell_value);
            else
                (cell_value == 0) ? mvwprintw(win, y + 1, x, "     ") : mvwprintw(win, y + 1, x, "  %zu  ", cell_value);

            if (row + 1 == N)
                mvwprintw(win, y + 2, x, "=====");
        }
    }
}

void highlight_cells(WINDOW *win, size_t grid[N][N], size_t cell_value)
{
    // TODO: turn highlight off/on x amount of times to indicate completed set of a number
    // NOTE: A_BLINK of attron/wattron does not work on some (many?) terminal emulators
    for (size_t row = 0; row < N; ++row) {
        for (size_t col = 0; col < N; ++col) {
            if (grid[row][col] == cell_value) {
                mvwprintw(win, row * 2 + 2, col * 4 + 1, "  %zu  ", grid[row][col]);
            }
        }

    }
}

bool highlight_same_value = true;
bool game_started         = false;
bool completed            = false;

void draw_grid(WINDOW *win, size_t grid[N][N], size_t cursor_row, size_t cursor_col, Difficulty current_difficulty)
{
    box(win, 0, 0);

    switch (current_difficulty) {
    case 0:
        mvwprintw(win, 0, 0, "sudo-[  Easy  ]");
        break;
    case 1:
        mvwprintw(win, 0, 0, "sudo-[ Medium ]");
        break;
    case 2:
        mvwprintw(win, 0, 0, "sudo-[  Hard  ]");
        break;
    default:
        mvwprintw(win, 0, 0, "sudo");
        break;
    }

    print_grid_window(win, grid);

    size_t highlight_y = cursor_row * 2 + 1;
    size_t highlight_x = cursor_col * 4 + 1;
    size_t cell_value  = grid[cursor_row][cursor_col];

    wmove(win, highlight_y, highlight_x);
    wattron(win, A_REVERSE); // "highlights" current cell

    if (cell_value == 0) {
        mvwprintw(win, highlight_y + 1, highlight_x, "|   |");
        completed = false;
    }
    else {
        if (highlight_same_value) {
            highlight_cells(win, grid, cell_value);
            mvwprintw(win, highlight_y + 1, highlight_x, "| %zu |", cell_value);
        }
        else {
            mvwprintw(win, highlight_y + 1, highlight_x, "| %zu |", cell_value);
        }
    }

    wattroff(win, A_REVERSE);

    if (game_started) {
        size_t count_cell_value = 0;
        size_t count_filled_cells = 0;
        for (size_t row = 0; row < N; ++row) {
            for (size_t col = 0; col < N; ++col) {
                if (grid[row][col] == cell_value) {
                    ++count_cell_value;
                }
                if (grid[row][col] != 0) {
                    ++count_filled_cells;
                }
            }
        }

        if (count_cell_value == N) {
            //                                      vv = strlen("0 completed.");
            mvwprintw(stdscr, LINES * 0.75, (COLS - 12) * 0.5, "%lu completed.", cell_value);
            completed = true;
        }

        if (count_filled_cells == N * N) {
            //                                      vv = strlen("Puzzle completed.");
            mvwprintw(stdscr, LINES * 0.75, (COLS - 17) * 0.5, "Puzzle completed.");
            completed = true;
        }
    }

    wrefresh(win);
}

void show_controls()
{
    const char *controls[] = {"Controls:",
                              "[Arrow Keys] Navigate Cells",
                              "[TAB] Change Difficulty",
                              "[H] Highlight Same Value Cells",
                              "[Q] Quit"};
    size_t controls_count = *(&controls + 1) - controls;

    size_t longest_string = 0;
    for (size_t i = 0; i < controls_count; ++i) {
        if (strlen(controls[i]) > longest_string) {
            longest_string = strlen(controls[i]);
        }
    }

    for (size_t i = 0; i < controls_count; ++i) {
        mvwprintw(stdscr, LINES * 0.5 + i, (COLS - longest_string) * 0.5, "%s", controls[i]);
    }
}

Difficulty switch_difficulty(Difficulty current)
{
    return (current + 1) % COUNT_DIFFICULTY;
}

int main(void)
{
    srand(time(0));

    size_t difficulty_values[COUNT_DIFFICULTY] = {20, 40 , 60};
    Difficulty current_difficulty = EASY;

    size_t grid_puzzle[N][N] = {0};
    fill_grid(grid_puzzle);

    size_t grid_solved[N][N] = {0};
    memcpy(&grid_solved, &grid_puzzle, sizeof(grid_puzzle));

    remove_numbers(grid_puzzle, difficulty_values[current_difficulty]);

    // TODO: better to specify a key rather than "any"
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

    // TODO: exit if not enough screen space
    size_t grid_y = N * 2 + 3;
    size_t grid_x = N * 4 + 3;

    WINDOW *sudoku_matrix = newwin(grid_y, grid_x, N, (COLS - grid_x) / 2);

    size_t cursor_row = 0;
    size_t cursor_col = 0;

    mvwprintw(stdscr, LINES * 0.75, (COLS - strlen(INIT_TEXT)) * 0.5, "%s", INIT_TEXT);
    show_controls();

    size_t mistakes = 0;
    bool quit = false;
    size_t c;

    while (!quit) {
        draw_grid(sudoku_matrix, grid_puzzle, cursor_row, cursor_col, current_difficulty);

        c = getch();
        if (c) {
            mvwprintw(stdscr, LINES * 0.75, (COLS - strlen(INIT_TEXT)) * 0.5, "%*c", (int)strlen(INIT_TEXT), ' ');
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
        case '9': {
            if (!completed) {
                size_t user_input = c - '0';
                if (grid_solved[cursor_row][cursor_col] == user_input) {
                    grid_puzzle[cursor_row][cursor_col] = user_input;
                }
                else {
                    mvwprintw(stdscr, LINES * 0.75, (COLS - strlen(INVALID_MOVE)) * 0.5, "%s", INVALID_MOVE);
                    ++mistakes;
                }
            }
            break;
        }
        case '\t': // switch difficulty
            current_difficulty = switch_difficulty(current_difficulty); // traverse through difficulties

            fill_grid(grid_puzzle);
            memcpy(&grid_solved, &grid_puzzle, sizeof(grid_puzzle));
            remove_numbers(grid_puzzle, difficulty_values[current_difficulty]);
            completed = false;
            break;
        case 'H': // (toggle) highlight same value cells
            highlight_same_value = !highlight_same_value;
            break;
        case 'Q': // quit
            quit = true;
            break;
        default:
            break;
        }

        if (!game_started && c) {
            game_started = true;
        }
    }

    delwin(sudoku_matrix);
    endwin();

    printf("Mistakes: %zu\nGoodbye!\n", mistakes);
    return 0;
}
