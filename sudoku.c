#include <assert.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define VERSION "0.7.0"
#define ONE_KB 1024
#define N 9

#define UNUSED(v) (void)(v)
#define SHIFT(xs, xs_size) (assert((xs_size) > 0), (xs_size)--, *(xs)++)

#define FORMAT_TIME(prefix, elapsed_time)                               \
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
        printf("%s %02zu:%02zu:%02zu\n", prefix, hours, minutes, seconds); \
    } while (0)

// Note to user:
// If number of difficulties is increased, then following functions need
// to be updated.
// (function      : what to look for)
// 1. draw_grid() : switch (sd->current_difficultyu)
// 2. main()      : difficulty_values[COUNT_DIFFICULTY]
// 3. main()      : strcmp(flag, "-times")
// Number 2 is important. Others are cosmetic.
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
        if (grid[i][j] != 0) {
            grid[i][j] = 0;
        }
    }
}

int setup_score_file(char *path_score_file)
{
    const char *score_file = "scores.sudoku";

    const char *xdg_cache_home = getenv("XDG_CACHE_HOME");
    if (xdg_cache_home == NULL) {
        const char *home_dir = getenv("HOME");
        if (home_dir == NULL) {
            fprintf(stderr, "ERROR: could not get $HOME environment variable\n");
            return 1;
        }
        sprintf(path_score_file, "%s/%s/%s", home_dir, ".cache", score_file);
    } else {
        sprintf(path_score_file, "%s/%s", xdg_cache_home, score_file);
    }

    FILE *f = fopen(path_score_file, "r+");
    if (f) {      // score_file exists
        fclose(f);
        return 0;
    }

    if (!f) {
        f = fopen(path_score_file, "w+");
        if (!f) {
            fprintf(stderr, "ERROR: failed to open or create score file at %s\n", path_score_file);
            fclose(f);
            return 1;
        }
    }

    for (size_t i = 0; i < COUNT_DIFFICULTY; ++i) {
        fprintf(f, "0.000000 ");
    }

    fclose(f);
    return 0;
}

// A score is the time taken to complete a puzzle of the respective current_difficulty
typedef struct {
    bool       save_scores;
    char       path_score_file[ONE_KB];
    Difficulty current_difficulty;
    double     current_score;
    double     best_scores[COUNT_DIFFICULTY];
    bool       hint_used;
} Score_Data;

void save_score(Score_Data *sd)
{
    if (!sd->save_scores || sd->hint_used ||
        (sd->best_scores[sd->current_difficulty] != 0.0 &&
         sd->current_score > sd->best_scores[sd->current_difficulty])) {
        return;
    }

    FILE *f = fopen(sd->path_score_file, "r+");
    assert(f != NULL);

    if (sd->best_scores[sd->current_difficulty] == 0.0) {
        sd->best_scores[sd->current_difficulty] = sd->current_score;
        for (size_t i = 0; i < COUNT_DIFFICULTY; ++i) {
            fprintf(f, "%lf ", sd->best_scores[i]);
        }
        fclose(f);
        return;
    }

    //                                                     vv = strlen("Puzzle completed. Improved time!")
    mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS -  32) * 0.5, "Puzzle completed. Improved time!");

    sd->best_scores[sd->current_difficulty] = sd->current_score;
    for (size_t i = 0; i < COUNT_DIFFICULTY; ++i) {
        fprintf(f, "%lf ", sd->best_scores[i]);
    }

    fclose(f);
}

void grab_scores(Score_Data *sd)
{
    char line[ONE_KB];
    char *sep = " ";
    FILE *f = fopen(sd->path_score_file, "r");
    assert(f != NULL);
    assert(fgets(line, sizeof(line), f) != NULL);
    fclose(f);

    char *token = strtok(line, sep);
    for (size_t i = 0; i < COUNT_DIFFICULTY && token != NULL; ++i) {
        sd->best_scores[i] = atof(token);
        token = strtok(NULL, sep);
    }
}

typedef struct {
    WINDOW *window;
    bool   puzzle_started;
    size_t cursor_col;
    size_t cursor_row;
    bool   highlight_same_value;
    bool   number_completed;
    bool   puzzle_completed;
} Window_Info;

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
            if (col % 3 == 0 || col == 0) {
                (cell_value == 0) ? mvwprintw(win, y + 1, x, "|   |") : mvwprintw(win, y + 1, x, "| %zu |", cell_value);
            }
            else if (col + 1 == N) {
                (cell_value == 0) ? mvwprintw(win, y + 1, x, "    |") : mvwprintw(win, y + 1, x, "  %zu |", cell_value);
            }
            else {
                (cell_value == 0) ? mvwprintw(win, y + 1, x, "     ") : mvwprintw(win, y + 1, x, "  %zu  ", cell_value);
            }

            if (row + 1 == N) {
                mvwprintw(win, y + 2, x, "=====");
            }
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
                // mvwprintw(win, row * 2 + 2, col * 4 + 1, "  %zu  ", grid[row][col]); // Highlight length of same value cells is equal to currently selected cell
                mvwprintw(win, row * 2 + 2, col * 4 + 2, " %zu ", grid[row][col]); // Reduced highlight length of same value cells versus currently selected cell
            }
        }
    }
}

void draw_grid(Window_Info *winfo, size_t grid[N][N], Score_Data *sd)
{
    box(winfo->window, 0, 0);

    switch (sd->current_difficulty) {
    case 0:
        mvwprintw(winfo->window, 0, 0, "sudoku-[  Easy  ]");
        break;
    case 1:
        mvwprintw(winfo->window, 0, 0, "sudoku-[ Medium ]");
        break;
    case 2:
        mvwprintw(winfo->window, 0, 0, "sudoku-[  Hard  ]");
        break;
    default:
        mvwprintw(winfo->window, 0, 0, "sudoku");
        break;
    }

    print_grid_window(winfo->window, grid);

    size_t highlight_y = winfo->cursor_row * 2 + 1;
    size_t highlight_x = winfo->cursor_col * 4 + 1;
    size_t cell_value  = grid[winfo->cursor_row][winfo->cursor_col];

    wmove(winfo->window, highlight_y, highlight_x);
    wattron(winfo->window, A_REVERSE); // Reverses background/foreground to "highlight" current cell

    if (cell_value == 0) {
        mvwprintw(winfo->window, highlight_y + 1, highlight_x, "|   |");
        winfo->number_completed = false;
    }
    else {
        if (winfo->highlight_same_value) {
            highlight_cells(winfo->window, grid, cell_value);
            mvwprintw(winfo->window, highlight_y + 1, highlight_x, "| %zu |", cell_value);
        }
        else {
            mvwprintw(winfo->window, highlight_y + 1, highlight_x, "| %zu |", cell_value);
        }
    }

    wattroff(winfo->window, A_REVERSE);

    if (winfo->puzzle_started && winfo->puzzle_completed) {
            mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - 17) * 0.5, "Puzzle completed.");
    } else if (winfo->puzzle_started && cell_value != 0) {
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

        if (count_cell_value == N && !winfo->puzzle_completed) {
            //                                                    vv = strlen("0 completed.");
            mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - 12) * 0.5, "%lu completed.", cell_value);
            winfo->number_completed = true;
        }

        if (count_filled_cells == N * N && !winfo->puzzle_completed) {
            //                                                    vv = strlen("Puzzle completed.");
            mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - 17) * 0.5, "Puzzle completed.");
            size_t ret = clock_gettime(CLOCK_MONOTONIC, &time_end);
            assert(ret == 0);
            winfo->puzzle_completed = true;
            sd->current_score = time_taken(time_begin, time_end);
            save_score(sd);
        }
    }

    wrefresh(winfo->window);
}

void show_controls()
{
    const char *controls[] = {"Controls:",
                              "[TAB] Change Difficulty",
                              "[ H ] Highlight Same Value Cells",
                              "[ ? ] Fill Current Cell",
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

void clear_info_text(int len_init_text)
{
    mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - len_init_text) * 0.5, "%*c", len_init_text, ' ');
}

int main(int argc, char **argv)
{
    srand(time(0));

    // The number of cells to be empty for each difficulty
    // It is not exact, but at most. There is a "randomness" to it
    size_t difficulty_values[COUNT_DIFFICULTY] = {20, 40, 60};

    char *program_name = SHIFT(argv, argc);

    Score_Data sd = {
        .save_scores        = false,
        .path_score_file    = {0},
        .current_difficulty = EASY,
        .current_score      = 0.0,
        .best_scores        = {0.000000},
        .hint_used          = false,
    };
    if (setup_score_file(sd.path_score_file) == 0) {
        sd.save_scores = true;
    }
    grab_scores(&sd);

    if (argc > 0) {
        char *flag = SHIFT(argv, argc);
        if (strcmp(flag, "-times") == 0) {
            for (size_t i = 0; i < COUNT_DIFFICULTY; ++i) {
                switch (i) {
                case 0: FORMAT_TIME("Easy:  ", sd.best_scores[i]); break;
                case 1: FORMAT_TIME("Medium:", sd.best_scores[i]); break;
                case 2: FORMAT_TIME("Hard:  ", sd.best_scores[i]); break;
                default: break;
                }
            }
            return 0;
        } else if (strcmp(flag, "-version") == 0) {
            printf("%s (version %s)\n", program_name, VERSION);
            return 0;
        } else if (strcmp(flag, "-help") == 0) {
            printf("Usage: %s <option>\n", program_name);
            printf("Options:\n");
            printf("  -times:   Show best times in each difficulty category\n");
            printf("  -version: Show version\n");
            printf("  -help:    Show this help message\n");
            return 0;
        }
    }

    size_t grid_puzzle[N][N] = {0};
    fill_grid(grid_puzzle);

    size_t grid_solved[N][N] = {0};
    memcpy(&grid_solved, &grid_puzzle, sizeof(grid_puzzle));

    remove_numbers(grid_puzzle, difficulty_values[sd.current_difficulty]);

    const char *INIT_TEXT    = "Press the <ENTER> key to start...";
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
        fprintf(stderr, "Terminal size too smol ._.\n");
        fprintf(stderr, "Need minimum: 24 LINES, 39 COLUMNS\n"); // $ echo $LINES $COLUMNS
        return 1;
    }

    WINDOW *sudoku_matrix = newwin(GRID_Y, GRID_X, (LINES - GRID_Y) / 2, (COLS - GRID_X) / 2);

    Window_Info winfo = {
        .window               = sudoku_matrix,
        .puzzle_started       = false,
        .cursor_col           = 0,
        .cursor_row           = 0,
        .highlight_same_value = true,
        .number_completed     = false,
        .puzzle_completed     = false,
    };

    mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - len_init_text) * 0.5, "%s", INIT_TEXT);
    show_controls();

    size_t mistakes = 0;
    bool quit = false;
    size_t c;

    while (!winfo.puzzle_started && !quit) {
        c = getch();
        switch (c) {
        case '\n':
            winfo.puzzle_started = true;
            size_t ret = clock_gettime(CLOCK_MONOTONIC, &time_begin);
            assert(ret == 0);
            clear_info_text(len_init_text);
            break;
        case 'Q':
            quit = true;
            break;
        default:
            break;
        }
    }

    while (!quit) {
        draw_grid(&winfo, grid_puzzle, &sd);

        c = getch();
        clear_info_text(len_init_text);

        switch (c) {
        case KEY_UP:
        case 'w':
            if (winfo.cursor_row > 0) {
                --winfo.cursor_row;
            }
            break;
        case KEY_DOWN:
        case 's':
            if (winfo.cursor_row < N - 1) {
                ++winfo.cursor_row;
            }
            break;
        case KEY_LEFT:
        case 'a':
            if (winfo.cursor_col > 0) {
                --winfo.cursor_col;
            }
            break;
        case KEY_RIGHT:
        case 'd':
            if (winfo.cursor_col < N - 1) {
                ++winfo.cursor_col;
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
            if (!winfo.number_completed && grid_puzzle[winfo.cursor_row][winfo.cursor_col] == 0) {
                size_t user_input = c - '0';
                if (grid_solved[winfo.cursor_row][winfo.cursor_col] == user_input) {
                    grid_puzzle[winfo.cursor_row][winfo.cursor_col] = user_input;
                }
                else {
                    mvwprintw(stdscr, ((LINES + GRID_Y) / 2) + 1, (COLS - strlen(INVALID_MOVE)) * 0.5, "%s: %zu", INVALID_MOVE, user_input);
                    ++mistakes;
                }
            }
            break;
        }
        case '\t': // switch difficulty
            sd.current_difficulty = switch_difficulty(sd.current_difficulty); // traverse through difficulties
            sd.current_score      = 0.0;

            memset(grid_puzzle, 0, sizeof(grid_puzzle));
            fill_grid(grid_puzzle);
            memcpy(&grid_solved, &grid_puzzle, sizeof(grid_puzzle));
            remove_numbers(grid_puzzle, difficulty_values[sd.current_difficulty]);

            winfo.number_completed = false;
            winfo.puzzle_completed = false;
            mistakes = 0;
            sd.hint_used = false;

            size_t ret = clock_gettime(CLOCK_MONOTONIC, &time_begin);
            assert(ret == 0);
            break;
        case '?':
            if (grid_puzzle[winfo.cursor_row][winfo.cursor_col] == 0) {
                grid_puzzle[winfo.cursor_row][winfo.cursor_col] = grid_solved[winfo.cursor_row][winfo.cursor_col];
                sd.hint_used = true;
            }
            break;
        case 'H': // (toggle) highlight same value cells
            winfo.highlight_same_value = !winfo.highlight_same_value;
            break;
        case 'Q': // quit
            quit = true;
            if (!winfo.puzzle_completed) {
                ret = clock_gettime(CLOCK_MONOTONIC, &time_end);
                assert(ret == 0);
            }
            break;
        default:
            break;
        }
    }

    delwin(sudoku_matrix);
    endwin();

    double elapsed_time = time_taken(time_begin, time_end);
    FORMAT_TIME("Last time taken:", elapsed_time);
    printf("Mistakes: %zu\n", mistakes);

    printf("Goodbye!\n");
    return 0;
}
