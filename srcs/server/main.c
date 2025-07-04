/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 19:00:00 by dlesieur          #+#    #+#             */
/*   Updated: 2025/07/04 12:30:19 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

static int handle_new_client(t_client_state *client, siginfo_t *info)
{
	if (client->actual_pid == 0)
	{
		log_msg(LOG_INFO, "New client connection from PID %d", info->si_pid);
		client->client_pid = info->si_pid;
		pong(client->client_pid);
		return (1);
	}
	return (0);
}

static int handle_busy_server(t_client_state *client, siginfo_t *info)
{
	// If server is busy with a different client, add to queue
	if (client->actual_pid > 0 && client->actual_pid != info->si_pid)
	{
		// Check if this client is already in queue
		for (int i = 0; i < client->queue_size; i++)
		{
			int queue_index = (client->queue_head + i) % MAX_QUEUE_SIZE;
			if (client->client_queue[queue_index] == info->si_pid)
			{
				log_msg(LOG_DEBUG, "Client %d already in queue, sending busy signal", info->si_pid);
				// Send busy signal to remind client to wait
				kill(info->si_pid, SERVER_BUSY);
				return (1);
			}
		}

		// Add new client to queue
		if (enqueue_client(info->si_pid))
		{
			log_msg(LOG_INFO, "Server busy with PID %d, added client %d to queue (position %d)",
					client->actual_pid, info->si_pid, client->queue_size);
			kill(info->si_pid, SIGUSR1); // Use SIGUSR1 for SERVER_BUSY
		}
		else
		{
			log_msg(LOG_ERROR, "Queue full, rejecting client %d", info->si_pid);
			kill(info->si_pid, SIGUSR1); // Use SIGUSR1 for SERVER_BUSY
		}
		return (1);
	}
	return (0);
}

static void process_data_signal(t_client_state *client, int signum)
{
	static volatile int processing = 0;

	ft_printf("DEBUG_SIGNAL: process_data_signal() called with signal %s\n",
		signum == SIGUSR1 ? "SIGUSR1" : "SIGUSR2");
	ft_printf("DEBUG_SIGNAL: getting_header=%d, getting_msg=%d\n",
		client->getting_header, client->getting_msg);

	// Prevent re-entrant signal processing that could cause bit shifts
	if (processing)
	{
		ft_printf("DEBUG_SIGNAL: Signal processing already in progress, ignoring\n");
		log_msg(LOG_WARNING, "Signal processing already in progress, ignoring signal");
		return;
	}

	processing = 1;

	// Process the signal
	if (client->getting_msg == 1)
	{
		ft_printf("DEBUG_SIGNAL: Processing message signal\n");
		handle_msg(signum);
	}
	else if (client->getting_header == 1)
	{
		ft_printf("DEBUG_SIGNAL: Processing header signal\n");
		handle_header(signum);
	}
	else
	{
		ft_printf("DEBUG_SIGNAL: WARNING: Neither getting_header nor getting_msg is set!\n");
	}

	processing = 0;

	// Send immediate acknowledgment - but check if client still exists
	if (client->client_pid > 0 && kill(client->client_pid, 0) == 0)
	{
		if (kill(client->client_pid, SIGUSR2) == -1)
		{
			ft_printf("DEBUG_SIGNAL: Failed to send ACK to client %d\n", client->client_pid);
			log_msg(LOG_ERROR, "Failed to send acknowledgment to client %d", client->client_pid);
		}
		else
		{
			ft_printf("DEBUG_SIGNAL: ACK sent to client %d\n", client->client_pid);
		}
	}
	else
	{
		ft_printf("DEBUG_SIGNAL: Client %d no longer exists\n", client->client_pid);
		log_msg(LOG_WARNING, "Client %d no longer exists, cleaning up", client->client_pid);
		clean_global();
	}
}

void signal_handler(int signum, siginfo_t *info, void *context)
{
	t_client_state *client;
	pid_t sender_pid;

	(void)context;
	client = get_client_instance();

	// CRITICAL FIX: Use direct PID instead of lost_signal() to avoid bit shift
	sender_pid = info->si_pid;
	
	ft_printf("DEBUG_DIRECT: Direct PID processing: original=%d, using=%d\n", 
		info->si_pid, sender_pid);

	if (sender_pid <= 0 || sender_pid == getpid())
	{
		log_msg(LOG_ERROR, "Invalid signal source PID: %d", sender_pid);
		return;
	}

	// Handle new client connection first
	if (handle_new_client(client, info))
		return;

	// Handle busy server
	if (handle_busy_server(client, info))
		return;

	// Process data from authorized client only
	if (client->actual_pid == sender_pid)
	{
		client->client_pid = sender_pid;
		client->client_activity = 1;
		process_data_signal(client, signum);
	}
	else
	{
		log_msg(LOG_WARNING, "Ignoring signal from unauthorized PID %d (current: %d)",
				sender_pid, client->actual_pid);
	}
}

int main(void)
{
	struct sigaction sa;
	sigset_t sigset;
	pid_t server_pid;

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