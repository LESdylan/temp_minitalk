/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/03 02:15:33 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:42:00 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

/**
 * know if the other machine is online
 * measure how long it tooks to get the signal to the client server
 * check if there's a packet loss is signals are missing
 */

void	ping_handler(int signum, siginfo_t *info, void *context)
{
	t_server_state	*server;

	(void)context;
	server = get_server_instance();
	log_ping_signal(signum, info->si_pid);
	if (!validate_ping_signal(server, info))
		return ;
	handle_pong(signum, server, info->si_pid);
}

int	send_ping_signal(int pid)
{
	if (kill(pid, SIGUSR1) == -1)
	{
		log_msg(LOG_ERROR, "Failed to send signal to PID %d", pid);
		ft_printf("Error: Failed to send signal to server PID %d\n", pid);
		return (0);
	}
	return (1);
}

int	attempt_ping(int pid, int attempt)
{
	t_server_state *server;
	
	log_ping_attempt(attempt, RETRY_TIMES);
	if (!validate_process_exists(pid))
		return (-1);
	if (!send_ping_signal(pid))
		return (-1);
		
	server = get_server_instance();
	if (check_server_and_sleep())
	{
		if (server->is_ready == 1)
		{
			log_ping_result(attempt, 1);
			return (1);
		}
		// Server responded but is busy
		log_msg(LOG_INFO, "Server is busy, will retry");
	}
	
	log_ping_result(attempt, 0);
	handle_retry_delay(attempt);
	return (0);
}

int	ping(int pid)
{
	struct sigaction	sa;
	sigset_t			sigset;
	t_server_state		*server;

	server = get_server_instance();
	reset_server_state();
	setup_ping_signals(&sa, &sigset);
	server->pid = pid;
	server->is_ready = 0;
	server->ready_to_proceed = 0;
	server->transmission_active = 0;
	if (handle_timeouts(pid))
	{
		ft_printf("Error: Couldn't reach server PID %d\n", pid);
		log_msg(LOG_ERROR, "Server ping timeout");
		return (0);
	}
	ft_printf("Server ready, waiting for transmission slot...\n");
	log_msg(LOG_SUCCESS, "Server connection established");
	return (1);
}
