/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handshake.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 16:19:47 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:35:56 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void handle_acknowledgment(t_server_state *server)
{
	server->ready_to_proceed = 1;
	log_msg(LOG_DEBUG, "Server ready to receive next bit");
}

void wait_for_server_ack(void)
{
	t_server_state *server;
	int timeout_count;
	int max_timeout;

	server = get_server_instance();
	timeout_count = 0;
	max_timeout = 10000; // 1 second timeout - reduced for better responsiveness

	// Reset the flag before waiting
	server->ready_to_proceed = 0;

	while (!server->ready_to_proceed && timeout_count < max_timeout)
	{
		usleep(100);
		timeout_count++;

		// Simple existence check every 0.5 seconds
		if (timeout_count % 5000 == 0)
		{
			if (kill(server->pid, 0) == -1)
			{
				ft_printf("Error: Server process no longer exists\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	if (timeout_count >= max_timeout)
	{
		ft_printf("Error: Server acknowledgment timeout\n");
		log_msg(LOG_ERROR, "ACK timeout after %dms", timeout_count / 10);
		exit(EXIT_FAILURE);
	}

	// Reset the flag after receiving acknowledgment
	server->ready_to_proceed = 0;
}

void wait_for_transmission_slot(t_client *data)
{
	pid_t my_pid;

	(void)data;
	my_pid = getpid();

	log_msg(LOG_INFO, "Acquiring transmission slot...");

	// Since we already established connection, we should be able to proceed
	set_transmission_active(my_pid);
	log_msg(LOG_SUCCESS, "Transmission slot acquired");
}