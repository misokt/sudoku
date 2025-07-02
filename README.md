# sudoku

Sudoku in your terminal emulator.

![Puzzle screen](./images/puzzle-screen.png "Puzzle screen")

More images [here](./images).

## Controls

| Controls               | Description                |
| ---------------------- | -------------------------- |
| `wasd` or `ARROW keys` | Move cursor                |
| `H`                    | Toggle highlighted numbers |
| `?`                    | Fill current cell          |
| `TAB key`              | Change difficulty          |
| `Q`                    | Quit                       |


## To Build

```console
$ cc -o sudoku ./sudoku.c -lncurses
$ ./sudoku
```

Or, add the directory to your PATH, then use the build script to build and run:

```console
$ ./build.sh run
```

Depending on the day, you'll be in for a fun time! :v

## Dependency

- Ncurses

---
> Init day: 20250401
