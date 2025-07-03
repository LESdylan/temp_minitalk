/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handshake.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 16:19:47 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:59:54 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void	handle_acknowledgment(t_server_state *server)
{
	server->ready_to_proceed = 1;
	log_msg(LOG_DEBUG, "Server ready to receive next bit");
}

void	wait_for_server_ack(void)
{
	t_server_state	*server;
	int				timeout_count;
	int				max_timeout;
	static int		consecutive_timeouts = 0;

	server = get_server_instance();
	timeout_count = 0;
	max_timeout = 100000;  // 10 seconds instead of 5
	log_msg(LOG_DEBUG, "Waiting for server acknowledgment...");
	
	while (!server->ready_to_proceed && timeout_count < max_timeout)
	{
		usleep(100);
		timeout_count++;
		
		// Check if server process still exists every second
		if (timeout_count % 10000 == 0)
		{
			if (kill(server->pid, 0) == -1)
			{
				ft_printf("Error: Server process no longer exists\n");
				log_msg(LOG_ERROR, "Server process disappeared during transmission");
				exit(EXIT_FAILURE);
			}
			log_msg(LOG_DEBUG, "Still waiting for server ACK... (%d/10s)", timeout_count / 10000);
		}
	}
	
	if (timeout_count >= max_timeout)
	{
		consecutive_timeouts++;
		if (consecutive_timeouts >= 3)
		{
			ft_printf("Error: Multiple consecutive server timeouts, connection lost\n");
			log_msg(LOG_ERROR, "Multiple consecutive timeouts, aborting transmission");
			exit(EXIT_FAILURE);
		}
		ft_printf("Error: Server acknowledgment timeout (attempt %d/3)\n", consecutive_timeouts);
		log_msg(LOG_ERROR, "Timeout waiting for server acknowledgment (attempt %d)", consecutive_timeouts);
		exit(EXIT_FAILURE);
	}
	
	consecutive_timeouts = 0;  // Reset on successful ACK
	server->ready_to_proceed = 0;
	log_msg(LOG_DEBUG, "Server acknowledgment received");
}

void	wait_for_transmission_slot(t_client *data)
{
	pid_t	my_pid;

	(void)data;
	my_pid = getpid();
	
	log_msg(LOG_INFO, "Acquiring transmission slot...");
	
	// Since we already established connection, we should be able to proceed
	set_transmission_active(my_pid);
	log_msg(LOG_SUCCESS, "Transmission slot acquired");
}