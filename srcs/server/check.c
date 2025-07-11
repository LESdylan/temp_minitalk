/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:27:58 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 11:47:39 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

int check_clean_status(void)
{
	t_client_state *client;
	static int last_pid = 0;
	static int consecutive_checks = 0;

	client = get_client_instance();
	if (client->actual_pid == 0)
	{
		last_pid = 0;
		consecutive_checks = 0;
		sleep(1);
		return (1);
	}

	// Verify client process still exists
	if (client->actual_pid > 0 && kill(client->actual_pid, 0) == -1)
	{
		log_msg(LOG_WARNING, "Client process %d no longer exists, cleaning up", client->actual_pid);
		clean_global();
		return (1);
	}

	if (last_pid != client->actual_pid)
	{
		last_pid = client->actual_pid;
		consecutive_checks = 0;
		log_msg(LOG_DEBUG, "Starting monitoring for client %d", last_pid);
	}

	consecutive_checks++;
	// Reset client activity check every 100 iterations to prevent false timeouts
	if (consecutive_checks % 100 == 0)
	{
		client->client_activity = 1;
		log_msg(LOG_DEBUG, "Periodic activity reset for client %d", client->actual_pid);
	}

	return (monitor_client_timeout(client));
}

int is_server_busy(void)
{
	t_client_state *client;

	client = get_client_instance();
	return (client->transmission_active && client->actual_pid > 0);
}

int enqueue_client(pid_t client_pid)
{
	t_client_state *client;

	client = get_client_instance();
	if (client->queue_size >= MAX_QUEUE_SIZE)
	{
		log_msg(LOG_ERROR, "Queue full, cannot add client %d", client_pid);
		return (0);
	}

	// Check if client is already in queue
	for (int i = 0; i < client->queue_size; i++)
	{
		int queue_index = (client->queue_head + i) % MAX_QUEUE_SIZE;
		if (client->client_queue[queue_index] == client_pid)
		{
			log_msg(LOG_DEBUG, "Client %d already in queue", client_pid);
			return (1); // Already in queue, consider it success
		}
	}

	client->client_queue[client->queue_tail] = client_pid;
	client->queue_tail = (client->queue_tail + 1) % MAX_QUEUE_SIZE;
	client->queue_size++;
	log_msg(LOG_INFO, "Client %d added to queue at position %d", client_pid, client->queue_size);
	return (1);
}

pid_t dequeue_client(void)
{
	t_client_state *client;
	pid_t next_pid;
	int attempts = 0;

	client = get_client_instance();

	while (client->queue_size > 0 && attempts < MAX_QUEUE_SIZE)
	{
		next_pid = client->client_queue[client->queue_head];

		// Verify the client process still exists
		if (kill(next_pid, 0) == -1)
		{
			log_msg(LOG_WARNING, "Queued client %d no longer exists, removing from queue", next_pid);
			client->queue_head = (client->queue_head + 1) % MAX_QUEUE_SIZE;
			client->queue_size--;
			attempts++;
			continue; // Try next client in queue
		}

		// Valid client found
		client->queue_head = (client->queue_head + 1) % MAX_QUEUE_SIZE;
		client->queue_size--;
		log_msg(LOG_INFO, "Dequeued client %d, queue size now: %d", next_pid, client->queue_size);
		return (next_pid);
	}

	// No valid clients found
	if (attempts > 0)
		log_msg(LOG_WARNING, "Cleaned %d dead clients from queue", attempts);

	return (0);
}
