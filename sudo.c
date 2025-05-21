#include <assert.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define N 9

#define UNUSED(v) (void)(v)

#define FORMAT_TIME(elapsed_time)                                       \
    do {                                                                \
        size_t hours   = 0;                                             \
        size_t minutes = 0;                                             \
        size_t seconds = 0;                                             \
        if (elapsed_time >= 60*60) {                                    \
            hours = elapsed_time / (60*60);                             \
            elapsed_time = (size_t)elapsed_time % (60*60);              \
        }                                                               \
        if (elapsed_time >= 60.0) {                                     \
            minutes = elapsed_time / 60;                                \
            seconds = (size_t)elapsed_time % 60;                        \
        } else {                                                        \
            seconds = (size_t)elapsed_time;                             \
        }                                                               \
        printf("Time taken: %02zu:%02zu:%02zu\n", hours, minutes, seconds); \
    } while (0)

typedef enum {
    EASY,
    MEDIUM,
    HARD,
    COUNT_DIFFICULTY
} Difficulty;

size_t GRID_Y = N * 2 + 3;
size_t GRID_X = N * 4 + 3;

struct timespec time_begin = {0};
struct timespec time_end   = {0};

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
bool number_completed     = false;
bool puzzle_completed     = false;

void save_score(Difficulty current_difficulty)
{
    __builtin_unreachable();
    UNUSED(current_difficulty);
}

double fetch_score()
{
    __builtin_unreachable();
    int difficulty;
    double time;
    UNUSED(difficulty);
    UNUSED(time);
    return 0.0;
}

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
        number_completed = false;
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
            //                                                    vv = strlen("0 completed.");
            mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - 12) * 0.5, "%lu completed.", cell_value);
            number_completed = true;
        }

        if (count_filled_cells == N * N) {
            //                                                    vv = strlen("Puzzle completed.");
            mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - 17) * 0.5, "Puzzle completed.");
            puzzle_completed = true;
            size_t ret = clock_gettime(CLOCK_MONOTONIC, &time_end);
            assert(ret == 0);
        }
    }

    wrefresh(win);
}

void show_controls()
{
    const char *controls[] = {"Controls:",
                              "[TAB] Change Difficulty",
                              "[ H ] Highlight Same Value Cells",
                              "[ Q ] Quit"};
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

double time_taken(struct timespec begin, struct timespec end)
{
    double a = (double)begin.tv_sec + begin.tv_nsec * 1e-9;
    double b = (double)end.tv_sec + end.tv_nsec * 1e-9;
    double elapsed_time = b - a;
    if (elapsed_time < 0.0) {
        elapsed_time = 0.0;
    }
    return elapsed_time;
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

    const char *INIT_TEXT    = "Press ENTER key to start..";
    const char *INVALID_MOVE = "Invalid move";
    const int  len_init_text = strlen(INIT_TEXT);

    /* ----------------------  */

    initscr();
    noecho();
    keypad(stdscr, TRUE);
    cbreak();
    curs_set(0);

    if ((LINES < (int)GRID_Y) || (LINES <= (int)((LINES + GRID_Y) / 2) + 1)  || (COLS < (int)GRID_X)) {
        endwin();
        printf("Terminal size too smol ._.\n");
        fprintf(stderr, "Need minimum: 24 LINES, 39 COLUMNS\n"); // $ echo $LINES $COLUMNS
        return 1;
    }

    WINDOW *sudoku_matrix = newwin(GRID_Y, GRID_X, (LINES - GRID_Y) / 2, (COLS - GRID_X) / 2);

    size_t cursor_row = 0;
    size_t cursor_col = 0;

    mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - len_init_text) * 0.5, "%s", INIT_TEXT);
    show_controls();

    size_t mistakes = 0;
    bool quit = false;
    size_t c;

    while (!quit) {
        draw_grid(sudoku_matrix, grid_puzzle, cursor_row, cursor_col, current_difficulty);

        c = getch();
        if (c) {
            mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - len_init_text) * 0.5, "%*c", len_init_text, ' ');
        }
        switch (c) {
        case KEY_UP:
        case 'w':
            if (cursor_row > 0) {
                --cursor_row;
            }
            break;
        case KEY_DOWN:
        case 's':
            if (cursor_row < N - 1) {
                ++cursor_row;
            }
            break;
        case KEY_LEFT:
        case 'a':
            if (cursor_col > 0) {
                --cursor_col;
            }
            break;
        case KEY_RIGHT:
        case 'd':
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
            if (!number_completed) {
                size_t user_input = c - '0';
                if (grid_solved[cursor_row][cursor_col] == user_input) {
                    grid_puzzle[cursor_row][cursor_col] = user_input;
                }
                else {
                    mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - strlen(INVALID_MOVE)) * 0.5, "%s", INVALID_MOVE);
                    ++mistakes;
                }
            }
            break;
        }
        case '\t': // switch difficulty
            current_difficulty = switch_difficulty(current_difficulty); // traverse through difficulties

            memset(grid_puzzle, 0, sizeof(grid_puzzle));
            fill_grid(grid_puzzle);
            memcpy(&grid_solved, &grid_puzzle, sizeof(grid_puzzle));
            remove_numbers(grid_puzzle, difficulty_values[current_difficulty]);

            number_completed = false;
            puzzle_completed = false;

            size_t ret = clock_gettime(CLOCK_MONOTONIC, &time_begin);
            assert(ret == 0);
            break;
        case 'H': // (toggle) highlight same value cells
            highlight_same_value = !highlight_same_value;
            break;
        case 'Q': // quit
            quit = true;
            if (!puzzle_completed) {
                ret = clock_gettime(CLOCK_MONOTONIC, &time_end);
                assert(ret == 0);
            }
            break;
        default:
            break;
        }

        if (!game_started && c) {
            game_started = true;
            size_t ret = clock_gettime(CLOCK_MONOTONIC, &time_begin);
            assert(ret == 0);
        }
    }

    delwin(sudoku_matrix);
    endwin();

    double elapsed_time = time_taken(time_begin, time_end);
    FORMAT_TIME(elapsed_time);
    printf("Mistakes: %zu\n", mistakes);

    printf("Goodbye!\n");
    return 0;
}
