/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validate.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:16:35 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:48:56 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

int	validate_signal_source(t_server_state *server, pid_t sender_pid)
{
	if (server->pid != 0 && sender_pid != server->pid)
	{
		log_msg(LOG_WARNING, "Ignoring signal from unexpected PID: %d",
			sender_pid);
		return (0);
	}
	return (1);
}

int	validate_process_exists(int pid)
{
	if (kill(pid, 0) == -1)
	{
		log_msg(LOG_ERROR, "Process PID %d is not accessible", pid);
		ft_printf("Error: Server process PID %d not found\n", pid);
		return (0);
	}
	return (1);
}

int	validate_ping_signal(t_server_state *server, siginfo_t *info)
{
	pid_t	my_pid;

	my_pid = getpid();
	if (info->si_pid == my_pid)
	{
		log_msg(LOG_ERROR, "Received signal from own process");
		return (0);
	}
	if (server->pid != 0 && info->si_pid != server->pid)
	{
		log_msg(LOG_WARNING, "Ignoring unexpected PID: %d (expected %d)",
			info->si_pid, server->pid);
		return (0);
	}
	// Remove transmission_active check - we need to receive pings during transmission
	return (1);
}

void	validate_and_init(int argc, char **argv, t_client *data)
{
	t_parser_result	parse_result;

	parse_result = parse_arguments(argc, argv);
	if (parse_result != PARSER_SUCCESS)
		print_error_and_exit(parse_result);
	init_data(argv, data);
}
