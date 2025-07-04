/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 19:00:00 by dlesieur          #+#    #+#             */
/*   Updated: 2025/07/04 12:30:18 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

int pong(int pid)
{
	t_client_state *client;

	client = get_client_instance();

	// If server is busy with another client, add to queue
	if (is_server_busy() && client->actual_pid != pid)
	{
		if (enqueue_client(pid))
		{
			log_msg(LOG_INFO, "Server busy with PID %d, adding client %d to queue (position %d)",
					client->actual_pid, pid, client->queue_size);

			// Send busy signal multiple times to ensure delivery - use SIGUSR1 for SERVER_BUSY
			for (int i = 0; i < 3; i++)
			{
				if (kill(pid, SIGUSR1) == -1)
				{
					log_msg(LOG_ERROR, "Failed to send busy signal to client %d", pid);
					break;
				}
				usleep(1000);
			}
			return (EXIT_SUCCESS);
		}
		else
		{
			log_msg(LOG_ERROR, "Queue full, rejecting client %d", pid);
			kill(pid, SIGUSR1); // Use SIGUSR1 for SERVER_BUSY
			return (EXIT_SUCCESS);
		}
	}

	// Clean any previous state before accepting new client
	if (client->msg.message)
	{
		free(client->msg.message);
		client->msg.message = NULL;
	}

	// Reset for new client but preserve queue
	int saved_queue_size = client->queue_size;
	int saved_queue_head = client->queue_head;
	int saved_queue_tail = client->queue_tail;
	pid_t saved_queue[MAX_QUEUE_SIZE];
	for (int i = 0; i < MAX_QUEUE_SIZE; i++)
		saved_queue[i] = client->client_queue[i];

	ft_memset(client, 0, sizeof(*client));

	// Restore queue
	client->queue_size = saved_queue_size;
	client->queue_head = saved_queue_head;
	client->queue_tail = saved_queue_tail;
	for (int i = 0; i < MAX_QUEUE_SIZE; i++)
		client->client_queue[i] = saved_queue[i];

	client->actual_pid = pid;
	client->client_pid = pid;
	client->getting_header = 1;
	client->getting_msg = 0;
	client->sig_count = 0;
	client->msg_pos = 0;
	client->char_value = 0;
	client->client_activity = 1;
	set_server_busy(pid);
	log_msg(LOG_INFO, "Client %d connected, sending ready signal", pid);

	// Send ready signal multiple times - use SIGUSR2 for SERVER_READY
	for (int i = 0; i < 3; i++)
	{
		if (kill(pid, SIGUSR2) == -1)
		{
			log_msg(LOG_ERROR, "Failed to send ready signal to PID %d", pid);
			clean_global();
			return (EXIT_FAILURE);
		}
		usleep(1000);
	}

	log_msg(LOG_SUCCESS, "Server ready signal (SIGUSR2) sent to client %d", pid);
	return (EXIT_SUCCESS);
}

void send_multiple_acks(pid_t client_pid)
{
	int i;

	if (client_pid <= 0)
	{
		log_msg(LOG_ERROR, "Invalid client PID for acknowledgment: %d", client_pid);
		return;
	}

	// Send ACK signal multiple times to ensure delivery
	for (i = 0; i < 2; i++)
	{
		if (kill(client_pid, SIGUSR2) == -1)
		{
			log_msg(LOG_ERROR, "Failed to send acknowledgment to client %d", client_pid);
			return;
		}
		if (i < 1) // Small delay between signals
			usleep(500);
	}
	log_msg(LOG_DEBUG, "Sent acknowledgment signals to client %d", client_pid);
}

int lost_signal(int s_si_pid, int signum, void *context)
{
	t_client_state *client;

	(void)context;
	client = get_client_instance();

	// Handle invalid PID cases
	if (s_si_pid <= 0)
	{
		if (client->actual_pid > 0)
		{
			log_msg(LOG_WARNING, "Received signal %d with PID %d, using current client: %d",
					signum, s_si_pid, client->actual_pid);
			return (client->actual_pid);
		}
		log_msg(LOG_ERROR, "Invalid sender PID: %d for signal %d", s_si_pid, signum);
		return (0);
	}

	return (s_si_pid);
}

void send_completion_signal(pid_t client_pid)
{
	if (client_pid > 0)
	{
		log_msg(LOG_INFO, "Sending completion signal to client %d",
				client_pid);
		kill(client_pid, SIGUSR1);
	}
}
