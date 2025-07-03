/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 19:00:00 by dlesieur          #+#    #+#             */
/*   Updated: 2025/07/03 19:42:00 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

static int	handle_new_client(t_client_state *client, siginfo_t *info)
{
	if (client->actual_pid == 0)
	{
		log_msg(LOG_INFO, "New client connection from PID %d", info->si_pid);
		client->client_pid = info->si_pid;
		return (pong(client->client_pid), 1);
	}
	return (0);
}

static int	handle_busy_server(t_client_state *client, siginfo_t *info)
{
	if (client->actual_pid > 0 && client->actual_pid != info->si_pid)
	{
		log_msg(LOG_WARNING, "Server busy with PID %d, rejecting signal from PID %d",
			client->actual_pid, info->si_pid);
		
		// Send busy signal multiple times to ensure client receives it
		for (int i = 0; i < 3; i++)
		{
			if (kill(info->si_pid, SERVER_BUSY) == -1)
			{
				log_msg(LOG_ERROR, "Failed to send busy signal to PID %d", info->si_pid);
				break;
			}
			usleep(1000); // Small delay between signals
		}
		return (1);
	}
	return (0);
}

static void	process_data_signal(t_client_state *client, int signum)
{
	if (client->getting_msg == 1)
		handle_msg(signum);
	else if (client->getting_header == 1)
		handle_header(signum);
	send_multiple_acks(client->client_pid);
}

void	signal_handler(int signum, siginfo_t *info, void *context)
{
	t_client_state	*client;
	pid_t original_pid;

	client = get_client_instance();
	original_pid = info->si_pid;
	info->si_pid = lost_signal(info->si_pid, signum, context);
	
	if (info->si_pid <= 0 || info->si_pid == getpid())
	{
		log_msg(LOG_ERROR, "Invalid signal source PID: %d (original: %d)", info->si_pid, original_pid);
		return;
	}
	
	if (handle_new_client(client, info))
		return;
		
	if (handle_busy_server(client, info))
		return;
	
	// Verify this signal is from the current active client
	if (client->actual_pid != info->si_pid)
	{
		log_msg(LOG_WARNING, "Ignoring signal from PID %d, current client is %d", 
			info->si_pid, client->actual_pid);
		// Send busy signal to unauthorized client
		kill(info->si_pid, SERVER_BUSY);
		return;
	}
	
	client->client_pid = info->si_pid;
	client->client_activity = 1;
	log_msg(LOG_DEBUG, "Processing signal %d from active client %d",
		signum, client->client_pid);
	process_data_signal(client, signum);
}

int	main(void)
{
	struct sigaction	sa;
	sigset_t			sigset;
	pid_t				server_pid;

	server_pid = getpid();
	ft_printf("Server PID: %d\n", server_pid);
	
	// Setup signal handler without blocking
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR1);
	sigaddset(&sigset, SIGUSR2);
	
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sa.sa_sigaction = signal_handler;
	sa.sa_mask = sigset;
	
	if (sigaction(SIGUSR1, &sa, NULL) == -1 || 
		sigaction(SIGUSR2, &sa, NULL) == -1)
	{
		log_msg(LOG_ERROR, "Failed to setup signal handlers");
		exit(EXIT_FAILURE);
	}
	
	log_msg(LOG_INFO, "Signal handlers installed successfully");
	
	keep_server_up();
	return (0);
}
