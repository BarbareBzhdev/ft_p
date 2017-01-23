/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_user_password.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: barbare <barbare@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/23 17:58:59 by barbare           #+#    #+#             */
/*   Updated: 2017/01/20 14:47:33 by barbare          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/stat.h>
#include <fcntl.h>
#include "libft.h"
#include <string.h>
#include "config.h"
#include "handle.h"
#include "message.h"
#include "error.h"

static t_cli    reset_user(t_cli cli)
{
    cli.user[0] = '\0';
    cli.pass[0] = '\0';
	snprintf(cli.home, PATH_MAX, "%s/anonymous", cli.path_server);
	cli.isconnected = FALSE;
    return (cli);
}

static t_cli	set_user(t_cli cli, char **user)
{
	snprintf(cli.user, PATH_MAX, "%s", user[0]);
	snprintf(cli.pass, PATH_MAX, "%s", user[1]);
	snprintf(cli.home, PATH_MAX, "%s%s", cli.path_server, user[2]);
	if (cli.pass[0] != '\0')
		S_MESSAGE(331, cli.fd)
	else
	{
		cli.isconnected = TRUE;
		S_MESSAGE(230, cli.fd)
	}
	return (cli);
}

t_cli           get_user(t_cli cli, char **users, char *param)
{
    char            **params;
    char            **user;
    unsigned int    i;

    i = -1;
    params = ft_strsplit2(param, ' ');
    cli = reset_user(cli);
    while (cli.isconnected == FALSE && users[++i] != NULL)
    {
        user = ft_strsplit2(users[i], ':');
        if (ft_strncmp(user[0], params[1], ft_strlen(user[0])) == 0)
			cli = set_user(cli, user);
        free(user);
    }
    free(params);
    return (cli);
}

t_cli           handle_pass(t_env UNUSED(env), t_cli cli, char *param)
{
    char        **params;
	struct stat s;

	s = (struct stat){0};
    params = ft_strsplit2(param, ' ');
    if (params[2] != NULL)
        E_MESSAGE(501, cli.fd)
    if (ft_strncmp(cli.pass, params[1], ft_strlen(cli.pass)) == 0)
	{
        S_MESSAGE(230, cli.fd)
		cli.isconnected = TRUE;
		if (stat(cli.home, &s) == -1)
			mkdir(cli.home, 0700);
		chdir(cli.home);
		return (cli);
	}
    else
        S_MESSAGE(530, cli.fd)
	reset_user(cli);
    return (cli);
}

t_cli           handle_user(t_env UNUSED(env), t_cli cli, char *param)
{
    char        users[PATH_MAX];
    char        path[PATH_MAX];
    char        **args;
    int         ret;
    int         fd;

	path[0] = '\0';
	snprintf(path, PATH_MAX, "%s/%s", cli.path_server, cli.auth);
    if (access(cli.auth, F_OK) == ERROR)
        dprintf(STDERR_FILENO, "Cannot have permission for file %s\n", cli.auth);
    else
    {
        fd = open(cli.auth, O_RDONLY);
        ret = read(fd, users, PATH_MAX);
        users[ret] = '\0';
        args = ft_strsplit2(users, '\n');
        cli = get_user(cli, args, param);
        free(args);
        close(fd);
    }
    return (cli);
}