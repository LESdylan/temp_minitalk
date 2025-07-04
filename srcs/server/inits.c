/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inits.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:32:46 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:30:25 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

t_client_state *get_client_instance(void)
{
	static t_client_state instance;
	static int initialized = 0;

	if (!initialized)
	{
		ft_memset(&instance, 0, sizeof(t_client_state));
		instance.getting_header = 1;
		instance.msg.size_message = 0;
		instance.transmission_active = 0;
		instance.queue_position = 0;
		instance.sequence_number = 0;
		instance.expected_checksum = 0;
		instance.calculated_checksum = 0;
		// Initialize queue
		instance.queue_size = 0;
		instance.queue_head = 0;
		instance.queue_tail = 0;
		initialized = 1;
	}
	return (&instance);
}

void reset_client_state(t_client_state *client)
{
	pid_t old_pid;

	old_pid = client->actual_pid;

	if (client->msg.message)
	{
		free(client->msg.message);
		client->msg.message = NULL;
	}

	// Complete reset of all client state
	client->actual_pid = 0;
	client->client_pid = 0;
	client->getting_header = 1;
	client->getting_msg = 0;
	client->sig_count = 0;
	client->msg_pos = 0;
	client->char_value = 0;
	client->client_activity = 0;
	client->transmission_active = 0;
	client->queue_position = 0;
	client->sequence_number = 0;
	client->expected_checksum = 0;
	client->calculated_checksum = 0;
	client->msg.size_message = 0;

	// Only reset queue if it's empty
	if (client->queue_size == 0)
	{
		client->queue_head = 0;
		client->queue_tail = 0;
	}

	if (old_pid > 0)
		log_msg(LOG_INFO, "Released transmission slot from client PID %d", old_pid);
}

void clean_global(void)
{
	t_client_state *client;
	pid_t next_client;

	client = get_client_instance();
	if (client->actual_pid > 0)
		log_msg(LOG_WARNING, "Cleaning up client state for PID %d",
				client->actual_pid);
	client->client_activity = 0;
	if (client->msg.message != NULL)
	{
		write(2, "Cleaning up incomplete message from client\n", 43);
		log_msg(LOG_ERROR, "Incomplete message cleanup: %d bytes",
				client->msg.size_message);
		free(client->msg.message);
		client->msg.message = NULL;
	}

	// Check if there's a next client in queue
	next_client = dequeue_client();
	if (next_client > 0)
	{
		log_msg(LOG_INFO, "Serving next client in queue: PID %d", next_client);
		reset_client_state(client);
		client->actual_pid = next_client;
		client->client_pid = next_client;
		client->getting_header = 1;
		client->transmission_active = 1;
		kill(next_client, SERVER_READY);
	}
	else
	{
		reset_client_state(client);
		log_msg(LOG_INFO, "Client state cleaned, ready for new connections");
	}
}
