*This project was created as part of the 42 curriculum by esobrino.*

# get_next_line

## Description

`get_next_line` is a 42 project whose goal is to implement:

```c
char	*get_next_line(int fd);
```

The function returns one line at a time from a previously opened file descriptor. A returned line includes the trailing `\n` when present in the input. If EOF is reached and there is still pending content without a final newline, that last fragment must also be returned. Once there is nothing left to read, the function returns `NULL`.

The project is built around:

- `read`
- `malloc`
- `free`
- persistent state between calls
- correct handling of EOF, invalid `fd`, allocation failures, long lines, and small `BUFFER_SIZE`

## Repository Contents

### 42 deliverable files

```text
get_next_line.c
get_next_line.h
get_next_line_utils.c
get_next_line_bonus.c
get_next_line_bonus.h
get_next_line_utils_bonus.c
```

### Local development helpers

These files are included in this repository for local testing and documentation, but they are not part of the original mandatory delivery list from the subject:

- `main.c`
- `test_files/file.txt`
- `test_files/file_a.txt`
- `test_files/file_b.txt`
- `test_files/no_newline.txt`
- `test_files/empty.txt`
- `README.md`

## Build

The project is intended to compile with:

```bash
-Wall -Wextra -Werror
```

Mandatory object build:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 -c get_next_line.c get_next_line_utils.c
```

Bonus object build:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 -c get_next_line_bonus.c get_next_line_utils_bonus.c
```

Mandatory test binary with the local harness:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 main.c get_next_line.c get_next_line_utils.c -o gnl_test
```

Bonus test binary with the local harness:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 -D USE_BONUS main.c get_next_line_bonus.c get_next_line_utils_bonus.c -o gnl_test_bonus
```

## BUFFER_SIZE

`BUFFER_SIZE` defines the maximum number of bytes requested by each call to `read()`.

Default value in this repository:

```c
# define BUFFER_SIZE 100
```

Example override:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=1 main.c get_next_line.c get_next_line_utils.c -o gnl_test
```

## Local Test Harness

The repository includes a local `main.c` that acts as a small test harness. It is a simple executable used to run `get_next_line` under controlled scenarios and print the returned lines in a visible way.

### Supported modes

- `./gnl_test test_files/file.txt`
- `./gnl_test test_files/file.txt test_files/file_a.txt`
- `./gnl_test --stdin`
- `./gnl_test --invalid-fd-after-first-read test_files/file.txt`
- `./gnl_test_bonus --interleave test_files/file_a.txt test_files/file_b.txt`

### What each mode does

- `./gnl_test test_files/file.txt`: reads a single file from start to EOF
- `./gnl_test test_files/file.txt test_files/file_a.txt`: reads several files sequentially
- `./gnl_test --stdin`: reads from standard input until EOF
- `./gnl_test --invalid-fd-after-first-read test_files/file.txt`: reads one valid line, calls `get_next_line(-1)`, and then continues reading the original file
- `./gnl_test_bonus --interleave test_files/file_a.txt test_files/file_b.txt`: alternates calls between several file descriptors to verify the bonus behaviour

### Provided test files

- `test_files/file.txt`: normal multi-line file
- `test_files/file_a.txt`: short multi-line file for sequential and interleaved tests
- `test_files/file_b.txt`: second multi-line file for interleaved bonus tests
- `test_files/no_newline.txt`: single line without a trailing newline
- `test_files/empty.txt`: empty file

## Minimal Usage Example

```c
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char	*get_next_line(int fd);

int	main(void)
{
	int		fd;
	char	*line;

	fd = open("example.txt", O_RDONLY);
	if (fd < 0)
		return (1);
	line = get_next_line(fd);
	while (line)
	{
		printf("%s", line);
		free(line);
		line = get_next_line(fd);
	}
	close(fd);
	return (0);
}
```

## Selected Algorithm

### General idea

The implementation keeps a persistent **stash** between calls. The stash stores bytes that were already read from the descriptor but not yet returned to the caller.

General flow:

1. Check whether the stash already contains a complete line.
2. If it does not, read more bytes from the `fd` in blocks of `BUFFER_SIZE`.
3. Append each block to the stash.
4. Stop when a delimiter is found or EOF is reached.
5. Compute the exact line length.
6. Allocate the result line and copy it.
7. Trim the stash so that only the unread suffix remains.

### Helper roles

- `find_trigger`: locates the delimiter inside the stash
- `extract`: performs one `read()` and appends the new block
- `append_stash`: concatenates the read buffer to the stash
- `solve_line`: allocates and copies the next line to return
- `trim_stash`: keeps only the unread suffix
- `clear_stash`: frees and resets a stash pointer

## Current Implementation Notes

### Mandatory

The mandatory version keeps one persistent stash:

```c
static char *stash;
```

This repository also applies one local hardening change in `get_next_line.c`:

```c
static int stash_fd = -1;
```

This extra state tracks which file descriptor currently owns the mandatory stash. Its purpose is to prevent unread bytes from being lost when a different invalid descriptor is checked between calls. In practice:

- if the current owner `fd` becomes invalid, the stash is cleared
- if another unrelated invalid `fd` is checked, the active stash is preserved
- if a different valid `fd` starts using the mandatory version, the previous stash is discarded and ownership moves to the new descriptor

This is a local robustness improvement beyond the most minimal classical mandatory approach.

### Bonus

The bonus version keeps one independent stash per descriptor:

```c
static char *stash[MAX_FD];
```

This allows interleaved reads from multiple descriptors without mixing their state.

In this repository, `resolve_line_len` and `solve_and_trim` are declared in `get_next_line_bonus.h` and defined without the `static` keyword. This was done as a local compatibility adjustment for checker tooling that was counting every `static` token, even when used on helper functions rather than variables. The runtime behaviour of the bonus algorithm is unchanged by that visibility change.

## Technical Decisions

### Error handling

When a read or allocation error is detected:

- the function returns `NULL`
- the relevant stash is cleared when the current stream state would otherwise become inconsistent

### Bonus descriptor limit

The bonus version uses:

```c
# define MAX_FD 1024
```

as an internal implementation limit for indexing the stash array.

This means:

- the bonus implementation supports independent stash storage only for file descriptors in the range `0` to `1023`
- a valid file descriptor greater than or equal to `1024` will return `NULL` in this implementation
- this fixed limit is a pragmatic repository decision, not a property of `get_next_line` itself

### Local extension: TRIGGER

This repository includes one local extension that is not part of the original 42 subject:

```c
# define TRIGGER '\n'
```

It allows changing the delimiter at compile time:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=4 -D "TRIGGER='|'" main.c get_next_line.c get_next_line_utils.c -o gnl_test_trigger
```

The macro is validated to accept only non-NUL ASCII characters:

```c
# if TRIGGER < 0 || TRIGGER > 127 || TRIGGER == '\0'
#  error "TRIGGER must be a non-NUL ASCII character"
# endif
```

For 42 evaluation, the expected delimiter remains `'\n'`.

## Final Evaluation

The following checks were executed on the current repository state:

- `norminette get_next_line.c get_next_line_utils.c get_next_line.h get_next_line_bonus.c get_next_line_utils_bonus.c get_next_line_bonus.h`
- `cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 -c get_next_line.c get_next_line_utils.c`
- `cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 -c get_next_line_bonus.c get_next_line_utils_bonus.c`
- `cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 main.c get_next_line.c get_next_line_utils.c -o gnl_test`
- `cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 -D USE_BONUS main.c get_next_line_bonus.c get_next_line_utils_bonus.c -o gnl_test_bonus`

Functional checks executed with the harness:

- normal multi-line read with `test_files/file.txt`
- empty file with `test_files/empty.txt`
- last line without trailing newline with `test_files/no_newline.txt`
- mandatory invalid descriptor regression test with `--invalid-fd-after-first-read`
- bonus interleaved read with `--interleave test_files/file_a.txt test_files/file_b.txt`

Memory checks executed:

- `valgrind --leak-check=full --show-leak-kinds=all /tmp/gnl_test_final test_files/file.txt`
- `valgrind --leak-check=full --show-leak-kinds=all /tmp/gnl_test_bonus_final --interleave test_files/file_a.txt test_files/file_b.txt`

Results of the final evaluation:

- mandatory compiles with `-Wall -Wextra -Werror`
- bonus compiles with `-Wall -Wextra -Werror`
- deliverable files pass `norminette`
- mandatory functional checks passed
- bonus functional checks passed
- `valgrind` reported no leaks and no memory errors in the executed cases

## Resources

### Technical references

- `read(2)` manual
- `open(2)` manual
- `close(2)` manual
- `malloc(3)` and `free(3)` manuals
- POSIX documentation about file descriptors
- standard C references on strings and dynamic memory management

### AI usage

AI assistance was used during development as support for reasoning, review, test design, and documentation. The implementation and the final code decisions were integrated and adjusted manually inside the project.
