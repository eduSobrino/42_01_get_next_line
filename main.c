#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef USE_BONUS
# include "get_next_line_bonus.h"
#else
# include "get_next_line.h"
#endif

static void	print_escaped(const char *line)
{
	while (*line)
	{
		if (*line == '\n')
			printf("\\n");
		else if (*line == '\t')
			printf("\\t");
		else if (*line == '\\')
			printf("\\\\");
		else
			putchar(*line);
		line++;
	}
}

static void	print_result(const char *label, int line_no, char *line)
{
	if (!line)
	{
		printf("%s -> NULL\n", label);
		return ;
	}
	printf("%s line %d: \"", label, line_no);
	print_escaped(line);
	printf("\"\n");
	free(line);
}

static int	open_file(const char *path)
{
	int	fd;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		perror(path);
	return (fd);
}

static void	read_all_from_fd(const char *label, int fd)
{
	char	*line;
	int		line_no;

	line_no = 1;
	line = get_next_line(fd);
	while (line)
	{
		print_result(label, line_no, line);
		line_no++;
		line = get_next_line(fd);
	}
	printf("%s -> EOF\n", label);
}

static int	read_file(const char *path)
{
	int	fd;

	fd = open_file(path);
	if (fd < 0)
		return (1);
	read_all_from_fd(path, fd);
	close(fd);
	return (0);
}

static int	test_invalid_fd_after_first_read(const char *path)
{
	int		fd;
	char	*line;

	fd = open_file(path);
	if (fd < 0)
		return (1);
	printf("Testing invalid fd after first successful read on %s\n", path);
	line = get_next_line(fd);
	print_result("first call", 1, line);
	line = get_next_line(-1);
	print_result("invalid fd", 0, line);
	read_all_from_fd("continuation", fd);
	close(fd);
	return (0);
}

#ifdef USE_BONUS
static int	read_interleaved(int count, char **paths)
{
	int		*fds;
	int		*line_no;
	int		active;
	int		i;
	char	*line;

	fds = malloc(sizeof(int) * count);
	line_no = malloc(sizeof(int) * count);
	if (!fds || !line_no)
		return (free(fds), free(line_no), 1);
	active = count;
	i = 0;
	while (i < count)
	{
		fds[i] = open_file(paths[i]);
		if (fds[i] < 0)
			active--;
		line_no[i] = 1;
		i++;
	}
	while (active > 0)
	{
		i = 0;
		while (i < count)
		{
			if (fds[i] >= 0)
			{
				line = get_next_line(fds[i]);
				if (line)
					print_result(paths[i], line_no[i]++, line);
				else
				{
					printf("%s -> EOF\n", paths[i]);
					close(fds[i]);
					fds[i] = -1;
					active--;
				}
			}
			i++;
		}
	}
	free(fds);
	free(line_no);
	return (0);
}
#endif

static void	print_usage(const char *prog)
{
	printf("Usage:\n");
	printf("  %s --stdin\n", prog);
	printf("  %s <file1> [file2 ...]\n", prog);
	printf("  %s --invalid-fd-after-first-read <file>\n", prog);
#ifdef USE_BONUS
	printf("  %s --interleave <file1> <file2> [file3 ...]\n", prog);
#endif
}

int	main(int argc, char **argv)
{
	int	i;
	int	status;

	if (argc == 1 || (argc == 2 && strcmp(argv[1], "--stdin") == 0))
	{
		read_all_from_fd("stdin", STDIN_FILENO);
		return (0);
	}
	if (argc == 3 && strcmp(argv[1], "--invalid-fd-after-first-read") == 0)
		return (test_invalid_fd_after_first_read(argv[2]));
#ifdef USE_BONUS
	if (argc >= 4 && strcmp(argv[1], "--interleave") == 0)
		return (read_interleaved(argc - 2, argv + 2));
#endif
	status = 0;
	i = 1;
	while (i < argc)
	{
		status |= read_file(argv[i]);
		i++;
	}
	if (status != 0)
		print_usage(argv[0]);
	return (status);
}
