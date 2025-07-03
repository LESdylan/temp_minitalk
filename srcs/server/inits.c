/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inits.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:32:46 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 17:33:50 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

t_client_state	*get_client_instance(void)
{
	static t_client_state	instance;
	static int				initialized = 0;

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
		initialized = 1;
	}
	return (&instance);
}

void	reset_client_state(t_client_state *client)
{
	pid_t	old_pid = client->actual_pid;
	int		was_active = client->transmission_active;
	
	if (client->msg.message)
	{
		free(client->msg.message);
		client->msg.message = NULL;
	}
	ft_bzero(client, sizeof(t_client_state));
	client->getting_header = 1;
	client->msg.size_message = 0;
	client->transmission_active = 0;
	
	if (was_active && old_pid > 0)
		log_msg(LOG_INFO, "Released transmission slot from client PID %d", old_pid);
}

void	clean_global(void)
{
	t_client_state	*client;

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
	reset_client_state(client);
	log_msg(LOG_INFO, "Client state cleaned, ready for new connections");
}