/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   crook.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:26:43 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:59:53 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

int	check_client_disconnection(t_client_state *client)
{
	if (client->actual_pid == 0)
	{
		log_msg(LOG_DEBUG, "Client disconnected normally");
		return (1);
	}
	return (0);
}

int	calculate_checksum(const char *data, int length)
{
	int	checksum;
	int	i;

	checksum = 0;
	i = 0;
	while (i < length)
	{
		checksum ^= data[i];
		checksum = (checksum << 1) | (checksum >> 31);
		i++;
	}
	return (checksum);
}

int	get_bit_value(int signum)
{
	if (signum == SIGUSR1)
		return (0);
	return (1);
}

void	memory_reserve_to_store_signals(void)
{
	t_client_state	*client;

	client = get_client_instance();
	ft_printf("SIZE_MSG: [%d]\n", client->msg.size_message);
	log_msg(LOG_INFO, "Allocating memory for message of %d bytes",
		client->msg.size_message);
	
	// Add validation before allocation
	if (client->msg.size_message <= 0 || client->msg.size_message > 10000000)
	{
		log_msg(LOG_ERROR, "Invalid message size received: %d", client->msg.size_message);
		ft_printf("Error: Invalid message size: %d\n", client->msg.size_message);
		reset_client_state(client);
		return;
	}
	
	client->msg.message = malloc((client->msg.size_message + 1) * 1);
	if (client->msg.message == NULL)
	{
		log_msg(LOG_ERROR, "Memory allocation failed for %d bytes",
			client->msg.size_message);
		write(2, "Error: Malloc failed\n", 21);
		exit(EXIT_FAILURE);
	}
	client->getting_header = 0;
	client->getting_msg = 1;
	client->sig_count = 0;
	log_msg(LOG_SUCCESS, "Memory allocated successfully for %d bytes, "
		"switching to message reception mode", client->msg.size_message);
}
