/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.h                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: esobrino <esobrino@student.42barcelona.co  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:02:08 by esobrino          #+#    #+#             */
/*   Updated: 2026/04/20 19:13:49 by esobrino         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_BONUS_H
# define GET_NEXT_LINE_BONUS_H

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

# define MAX_FD 1024

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
int		resolve_line_len(int fd, char **stash, ssize_t *line_len);
char	*solve_and_trim(char **stash, ssize_t line_len);

#endif
