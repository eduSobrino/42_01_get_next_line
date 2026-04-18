/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils_bonus.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: esobrino <esobrino@student.42barcelona.co  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 11:29:44 by esobrino          #+#    #+#             */
/*   Updated: 2026/04/18 21:05:04 by esobrino         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

int	find_trigger(char *stash)
{
	int	i;

	if (stash == NULL)
		return (-1);
	i = 0;
	while (*(stash + i) != TRIGGER && *(stash + i) != '\0')
		i++;
	if (*(stash + i) == '\0')
		return (-1);
	else
		return (i + 1);
}

char	*solve_line(char *stash, int len)
{
	char	*line;
	int		i;

	line = malloc((len + 1) * sizeof(char));
	if (!line)
		return (NULL);
	i = 0;
	while (i < len)
	{
		*(line + i) = *(stash + i);
		i++;
	}
	line[len] = '\0';
	return (line);
}

ssize_t	extract(int fd, char **stash)
{
	char	*buffer;
	ssize_t	read_bytes;

	buffer = malloc((BUFFER_SIZE + 1) * sizeof(char));
	if (!buffer)
		return (-1);
	read_bytes = read(fd, buffer, BUFFER_SIZE);
	if (read_bytes > 0)
	{
		buffer[read_bytes] = '\0';
		if (append_stash(stash, buffer))
		{
			free(buffer);
			return (-1);
		}
	}
	free(buffer);
	return (read_bytes);
}

int	append_stash(char **stash, char *buffer)
{
	char	*new_stash;
	int		stash_len;
	int		buffer_len;
	int		i;

	stash_len = get_len(*stash);
	buffer_len = get_len(buffer);
	new_stash = malloc((stash_len + buffer_len + 1) * sizeof(char));
	if (!new_stash)
		return (-1);
	i = 0;
	while (i < stash_len)
	{
		*(new_stash + i) = *(*stash + i);
		i++;
	}
	while (i - stash_len < buffer_len)
	{
		*(new_stash + i) = *(buffer + i - stash_len);
		i++;
	}
	*(new_stash + i) = '\0';
	clear_stash(stash);
	*stash = new_stash;
	return (0);
}

int	trim_stash(char **stash, char *line)
{
	char	*new_stash;
	int		new_len;
	int		line_len;
	int		i;

	line_len = get_len(line);
	new_len = get_len(*stash) - line_len;
	if (new_len <= 0)
	{
		clear_stash(stash);
		return (0);
	}
	new_stash = malloc((new_len + 1) * sizeof(char));
	if (!new_stash)
		return (-1);
	i = 0;
	while (*(*stash + line_len + i))
	{
		*(new_stash + i) = *(*stash + line_len + i);
		i++;
	}
	new_stash[new_len] = '\0';
	clear_stash(stash);
	*stash = new_stash;
	return (0);
}
