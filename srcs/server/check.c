/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:27:58 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 17:32:15 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

int	check_clean_status(void)
{
	t_client_state	*client;
	static int		last_pid = 0;

	client = get_client_instance();
	if (client->actual_pid == 0)
	{
		last_pid = 0;
		sleep(1);
		return (1);
	}
	if (last_pid != client->actual_pid)
	{
		last_pid = client->actual_pid;
		log_msg(LOG_DEBUG, "Starting monitoring for client %d", last_pid);
	}
	return (monitor_client_timeout(client));
}

int	is_server_busy(void)
{
	t_client_state	*client;

	client = get_client_instance();
	return (client->transmission_active && client->actual_pid > 0);
}

int	enqueue_client(pid_t client_pid)
{
	t_client_state	*client;

	client = get_client_instance();
	if (client->queue_size >= MAX_QUEUE_SIZE)
	{
		log_msg(LOG_ERROR, "Queue full, cannot add client %d", client_pid);
		return (0);
	}
	
	client->client_queue[client->queue_tail] = client_pid;
	client->queue_tail = (client->queue_tail + 1) % MAX_QUEUE_SIZE;
	client->queue_size++;
	log_msg(LOG_INFO, "Client %d added to queue at position %d", client_pid, client->queue_size);
	return (1);
}

pid_t	dequeue_client(void)
{
	t_client_state	*client;
	pid_t			next_pid;

	client = get_client_instance();
	if (client->queue_size == 0)
		return (0);
	
	next_pid = client->client_queue[client->queue_head];
	client->queue_head = (client->queue_head + 1) % MAX_QUEUE_SIZE;
	client->queue_size--;
	log_msg(LOG_INFO, "Dequeued client %d, queue size now: %d", next_pid, client->queue_size);
	return (next_pid);
}
