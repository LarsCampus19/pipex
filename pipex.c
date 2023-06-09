/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpeeters <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/09 17:51:03 by lpeeters          #+#    #+#             */
/*   Updated: 2023/05/18 23:27:06 by lpeeters         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

/*fetch the path needed by arg_path*/

char	*fetch_paths(char **envp)
{
	while (*envp && *envp++)
		if (ft_strncmp(*envp, "PATH=", 5) == 0)
			return (*envp + 5);
	err(PATH_ENV_ERR);
	return ("Error");
}

/*fetch the arguments needed by execve*/

char	*arg_path(char *paths, char *cmd)
{
	char	**dirs;
	char	*apath;
	char	*bpath;
	int		i;

	dirs = ft_split(paths, ':');
	i = 0;
	while (dirs[i])
	{
		bpath = ft_strjoin(dirs[i], "/");
		apath = ft_strjoin(bpath, cmd);
		free(bpath);
		if (access(apath, F_OK | X_OK) == 0 || !apath)
			break ;
		free(apath);
		i++;
	}
	free_arr(dirs);
	if (!apath)
		err(CMD_ERR);
	return (apath);
}

/*redirect input and program to first command*/

void	child(int *p_fd, int *fd, char **av, char **envp)
{
	char	**cmd;

	if (dup2(fd[0], 0) < 0)
		err(DUP2_ERR);
	if (dup2(p_fd[1], 1) < 0)
		err(DUP2_ERR);
	if (close(p_fd[0]) < 0)
		err(CLOSE_ERR);
	cmd = ft_split(av[2], ' ');
	if (execve(arg_path(fetch_paths(envp), cmd[0]), cmd, envp) < 0)
		memerr(cmd, EXECVE_MEM, EXECVE_ERR);
}

/*redirect output and program to second command*/

void	parent(int *p_fd, int *fd, char **av, char **envp)
{
	char	**cmd;

	if (dup2(fd[1], 1) < 0)
		err(DUP2_ERR);
	if (dup2(p_fd[0], 0) < 0)
		err(DUP2_ERR);
	if (close(p_fd[1]) < 0)
		err(CLOSE_ERR);
	cmd = ft_split(av[3], ' ');
	if (execve(arg_path(fetch_paths(envp), cmd[0]), cmd, envp) < 0)
		memerr(cmd, EXECVE_MEM, EXECVE_ERR);
}

/*program that will mimic a pipe like in Shell*/

int	main(int ac, char **av, char **envp)
{
	int		p_fd[2];
	int		fd[2];
	pid_t	pid;

	if (ac != 5)
		err(INPUT_ERR);
	if (pipe(p_fd) < 0)
		err(PIPE_ERR);
	fd[0] = open(av[1], O_RDONLY);
	fd[1] = open(av[4], O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd[0] < 0 || fd[1] < 0)
		err(OPEN_ERR);
	pid = fork();
	if (pid < 0)
		err(FORK_ERR);
	if (!pid)
		child(p_fd, fd, av, envp);
	waitpid(pid, NULL, 0);
	parent(p_fd, fd, av, envp);
}
