/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/03 02:15:33 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:30:20 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

/**
 * know if the other machine is online
 * measure how long it tooks to get the signal to the client server
 * check if there's a packet loss is signals are missing
 */

void ping_handler(int signum, siginfo_t *info, void *context)
{
	t_server_state *server;

	(void)context;
	server = get_server_instance();
	log_ping_signal(signum, info->si_pid);
	if (!validate_ping_signal(server, info))
		return;
	handle_pong(signum, server, info->si_pid);
}

int send_ping_signal(int pid)
{
	if (kill(pid, SIGUSR1) == -1)
	{
		log_msg(LOG_ERROR, "Failed to send signal to PID %d", pid);
		ft_printf("Error: Failed to send signal to server PID %d\n", pid);
		return (0);
	}
	return (1);
}

int attempt_ping(int pid, int attempt)
{
	t_server_state *server;
	int timeout_count;
	int queue_wait_time = 0;
	int max_queue_wait = 60000000; // 60 seconds maximum queue wait
	int last_status_time = 0;

	log_ping_attempt(attempt, RETRY_TIMES);
	if (!validate_process_exists(pid))
		return (-1);
	if (!send_ping_signal(pid))
		return (-1);

	server = get_server_instance();
	server->is_ready = 0; // Reset before waiting

	// Wait for response with extended timeout for queue waiting
	timeout_count = 0;
	while (timeout_count < max_queue_wait)
	{
		usleep(1000);
		timeout_count += 1000;

		if (server->is_ready == 1)
		{
			log_ping_result(attempt, 1);
			return (1);
		}
		else if (server->is_ready == -1) // Server busy - we're in queue
		{
			if (queue_wait_time == 0)
			{
				ft_printf("Server is busy, added to queue. Waiting for turn...\n");
				log_msg(LOG_INFO, "Client added to server queue, waiting...");
			}
			queue_wait_time += 1000;

			// Show status every 10 seconds
			if (queue_wait_time - last_status_time >= 10000000)
			{
				ft_printf("Still in queue, waiting... (%ds)\n", queue_wait_time / 1000000);
				last_status_time = queue_wait_time;

				// Check if server is still responsive
				if (kill(pid, 0) == -1)
				{
					ft_printf("Error: Server process no longer exists\n");
					return (-1);
				}
			}

			server->is_ready = 0; // Reset to continue waiting
		}

		// After 30 seconds of queue waiting, try a fresh ping
		if (queue_wait_time > 30000000 && queue_wait_time % 30000000 == 0)
		{
			ft_printf("Long queue wait detected, sending fresh ping...\n");
			send_ping_signal(pid);
			server->is_ready = 0;
		}
	}

	log_ping_result(attempt, 0);
	ft_printf("Error: Queue wait timeout - server may be stuck\n");
	return (0);
}

int ping(int pid)
{
	struct sigaction sa;
	sigset_t sigset;
	t_server_state *server;

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
