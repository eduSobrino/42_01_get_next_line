*This project was created as part of the 42 curriculum by esobrino.*

# get_next_line

## Description

`get_next_line` is a 42 project whose goal is to implement the function:

```c
char	*get_next_line(int fd);
```

The function must return one line at a time from a previously opened file descriptor. A line includes the line break character if it is present in the input. If the end of file is reached and there is still remaining content without a trailing `\n`, the function must still return that last line. Once there is nothing left to read, it must return `NULL`.

The main interest of the project is building a line-reading abstraction using only `read`, `malloc`, and `free`, while correctly handling dynamic memory, persistent state between calls, and edge cases such as:

- very small `BUFFER_SIZE`
- empty files
- long lines requiring multiple `read()` calls
- a last line without a final newline
- reading from `stdin`

This repository includes:

- the mandatory version
- the bonus version with support for multiple file descriptors at the same time

Delivered files:

```text
get_next_line.c
get_next_line.h
get_next_line_utils.c
get_next_line_bonus.c
get_next_line_bonus.h
get_next_line_utils_bonus.c
README.md
```

## Instructions

### Compilation

The project is intended to be compiled with:

```bash
-Wall -Wextra -Werror
```

Mandatory example:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c
```

Bonus example:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c
```

### BUFFER_SIZE

`BUFFER_SIZE` defines the maximum number of bytes requested by each call to `read()`.

Default value in this repository:

```c
# define BUFFER_SIZE 100
```

It can be overridden at compile time:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=1 get_next_line.c get_next_line_utils.c
```

### Minimal usage example

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

### Expected behaviour

The function:

- returns a newly allocated string on each successful call
- includes `\n` when that character is part of the line being read
- returns the last line even if it does not end with `\n`
- returns `NULL` when nothing is left to read
- returns `NULL` on invalid `fd`, `read()` error, or allocation failure

The caller is responsible for calling `free()` on every returned line.

## Selected Algorithm

### General idea

The selected algorithm is based on keeping a persistent **stash** between calls. This stash contains all bytes already read from the descriptor but not yet returned to the caller.

The logic is:

1. Check whether the stash already contains a complete line.
2. If it does not, read from the `fd` in blocks of `BUFFER_SIZE`.
3. Append each newly read block to the stash.
4. Repeat until a line delimiter is found or EOF is reached.
5. Compute the exact length of the next line.
6. Allocate memory for that line and copy it.
7. Trim the stash so that only the unread remainder is kept.

### Justification

This approach is well suited to the project for several reasons:

- `read()` does not return lines, it returns arbitrary byte blocks, so a persistent buffer is necessary.
- a line may be longer than `BUFFER_SIZE`, so a single read is not enough.
- a single `read()` may contain several lines, so the unread suffix must be preserved for later calls.
- memory usage remains controlled because only the still-unconsumed content is kept.

### Mandatory

In the mandatory version, a single static stash is used:

```c
static char *stash;
```

This is enough because the state is preserved for one active reading stream.

### Bonus

In the bonus version, one independent stash is kept per file descriptor:

```c
static char *stash[MAX_FD];
```

This allows interleaved reads from several descriptors without mixing their internal state.

### Helper roles

The algorithm relies on the following helper responsibilities:

- `find_trigger`: locate the line delimiter inside the stash
- `extract`: perform one `read()` call and append its content
- `append_stash`: concatenate the newly read buffer to the stash
- `solve_line`: allocate and build the line to return
- `trim_stash`: keep only the unread suffix
- `clear_stash`: free and reset one stash pointer

### Practical complexity

The most delicate part is not the read itself, but the consistency of the state between calls:

- deciding when to return `NULL`
- handling EOF without a trailing `\n`
- avoiding loss of already read bytes
- freeing memory correctly on failure

The chosen algorithm addresses those points in a direct and readable way.

## Technical Decisions

### Error handling

When a read or allocation error is detected:

- the function returns `NULL`
- the corresponding stash is cleared in order to avoid keeping inconsistent state

### Bonus and descriptor limit

In the bonus version, the repository uses:

```c
# define MAX_FD 1024
```

as an internal limit for indexing the stash array.

### Local extension: TRIGGER

This repository includes one local extension that is **not part of the original 42 subject**:

```c
# define TRIGGER '\n'
```

It allows changing the line delimiter at compile time:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=4 -D "TRIGGER='|'" get_next_line.c get_next_line_utils.c
```

It is validated to accept only non-NUL ASCII characters:

```c
# if TRIGGER < 0 || TRIGGER > 127 || TRIGGER == '\0'
#  error "TRIGGER must be a non-NUL ASCII character"
# endif
```

For 42 evaluation, the expected delimiter remains `'\n'`.

## Resources

### Technical references

- `read(2)` manual
- `open(2)` manual
- `close(2)` manual
- `malloc(3)` and `free(3)` manuals
- POSIX documentation about file descriptors
- standard C references on strings and dynamic memory management

### AI usage

AI assistance was used during development as support for reasoning and review, specifically for:

- clarifying the behaviour of `read`, `EOF`, `stdin`, and file descriptors
- reviewing the overall logic of the mandatory and bonus versions
- identifying design errors in stash handling
- validating refactoring decisions required to pass norminette
- preparing functional and memory test batches
- drafting this `README.md`

The implementation, refactoring, and final code decisions were integrated and adjusted manually inside the project.

## Project Status

- Mandatory implemented
- Bonus implemented
- Compilation with `-Wall -Wextra -Werror` verified
- `norminette` verified
- Functional tests executed
- `valgrind` memory checks executed
