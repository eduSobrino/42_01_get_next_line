/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: esobrino <esobrino@student.42barcelona.co  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/12 21:13:08 by esobrino          #+#    #+#             */
/*   Updated: 2026/04/27 21:10:45 by esobrino         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

static int	resolve_line_len(int fd, char **stash, ssize_t *line_len);
static char	*solve_and_trim(char **stash, ssize_t line_len);

char	*get_next_line(int fd)
{
	static char	*stash;
	ssize_t		line_len;
	int			status;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	status = resolve_line_len(fd, &stash, &line_len);
	if (status <= 0)
		return (NULL);
	return (solve_and_trim(&stash, line_len));
}

int	get_len(char *s)
{
	int	i;

	if (!s)
		return (0);
	i = 0;
	while (*(s + i) != '\0')
		i++;
	return (i);
}

void	clear_stash(char **stash)
{
	free(*stash);
	*stash = NULL;
}

static char	*solve_and_trim(char **stash, ssize_t line_len)
{
	char		*line;

	line = solve_line(*stash, line_len);
	if (!line)
	{
		clear_stash(stash);
		return (NULL);
	}
	if (trim_stash(stash, line) < 0)
	{
		free(line);
		clear_stash(stash);
		return (NULL);
	}
	return (line);
}

static int	resolve_line_len(int fd, char **stash, ssize_t *line_len)
{
	ssize_t		read_bytes;

	*line_len = find_trigger(*stash);
	read_bytes = 1;
	while (*line_len == -1 && read_bytes > 0)
	{
		read_bytes = extract(fd, stash);
		if (read_bytes < 0)
		{
			clear_stash(stash);
			return (-1);
		}
		*line_len = find_trigger(*stash);
	}
	if (*line_len == -1 && read_bytes == 0)
	{
		if (get_len(*stash) == 0)
			return (0);
		*line_len = get_len(*stash);
	}
	return (1);
}
