/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: esobrino <esobrino@student.42barcelona.co  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:02:08 by esobrino          #+#    #+#             */
/*   Updated: 2026/04/18 20:35:40 by esobrino         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

# include <unistd.h>
# include <stdlib.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 100
# endif

# ifndef TRIGGER
#  define TRIGGER '\n'
# endif

# if TRIGGER < 0 || TRIGGER > 127 || TRIGGER == '\0'
#  error "TRIGGER must be a non-NUL ASCII character"
# endif

/* -----MAIN----------------------------------------------------------------- */
char	*get_next_line(int fd);

/* -----UTILS---------------------------------------------------------------- */
int		find_trigger(char *stash);
ssize_t	extract(int fd, char **stash);
int		append_stash(char **stash, char *buffer);
int		trim_stash(char **stash, char *line);
char	*solve_line(char *stash, int len);
int		get_len(char *s);
void	clear_stash(char **stash);

#endif
