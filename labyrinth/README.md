# M1 - Labyrinth

[Requirements](https://jyywiki.cn/OS/2025/labs/M1.md)

## Run

```bash
make
./labyrinth
```

## Test

```bash
make
TK_RUN=1 ./labyrinth
```

## What I've Learned

- `argc` and `argv` are how C passes command-line arguments to the program.
  - `argc` = argument count
  - `argv` = argument vector (an array of strings, one per argument)
  - e.g. If I run `./labyrinth --map test.map --player 1 --move right`, the `argv` will contain 7 elements (include `./labyrinth`). Therefore, `argc` = 7.
- `.h` is the header file in C. It is mostly used to define data structures and declare APIs.
