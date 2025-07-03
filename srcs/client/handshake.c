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

	server = get_server_instance();
	timeout_count = 0;
	max_timeout = 50000;  // 5 seconds
	log_msg(LOG_DEBUG, "Waiting for server acknowledgment...");
	
	while (!server->ready_to_proceed && timeout_count < max_timeout)
	{
		usleep(100);
		timeout_count++;
		
		if (timeout_count >= max_timeout)
		{
			ft_printf("Error: Server acknowledgment timeout\n");
			log_msg(LOG_ERROR, "Timeout waiting for server acknowledgment");
			exit(EXIT_FAILURE);
		}
	}
	
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