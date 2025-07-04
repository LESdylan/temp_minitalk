/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   crook.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:26:43 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:30:19 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

int check_client_disconnection(t_client_state *client)
{
	if (client->actual_pid == 0)
	{
		log_msg(LOG_DEBUG, "Client disconnected normally");
		return (1);
	}
	return (0);
}

int calculate_checksum(const char *data, int length)
{
	unsigned int checksum;
	int i;

	checksum = 0x5A5A5A5A; // Use a non-zero seed to avoid trivial checksums
	i = 0;
	while (i < length)
	{
		checksum ^= (unsigned char)data[i];
		// Use a simpler rotation to avoid overflow issues
		checksum = ((checksum << 3) | (checksum >> 29)) ^ (i & 0xFF);
		i++;
	}
	// Ensure we don't return -1 by masking off the sign bit if needed
	if ((int)checksum == -1)
		checksum = 0x7FFFFFFF;
	return (int)checksum;
}

int get_bit_value(int signum)
{
	int bit_value;

	if (signum == SIGUSR1)
		bit_value = 0; // SIGUSR1 = 0 bit
	else
		bit_value = 1; // SIGUSR2 = 1 bit

	return bit_value;
}

void memory_reserve_to_store_signals(void)
{
	t_client_state *client;

	client = get_client_instance();

	// Debug: Show the exact value received
	ft_printf("DEBUG_MEM: memory_reserve_to_store_signals() called\n");
	ft_printf("DEBUG_MEM: client->msg.size_message = %d (0x%x)\n", 
		client->msg.size_message, client->msg.size_message);
	ft_printf("SIZE_MSG: [%d] (0x%x)\n", client->msg.size_message, client->msg.size_message);
	
	log_msg(LOG_INFO, "Allocating memory for message of %d bytes (0x%x)",
			client->msg.size_message, client->msg.size_message);

	// Add validation before allocation
	if (client->msg.size_message <= 0 || client->msg.size_message > 10000000)
	{
		log_msg(LOG_ERROR, "Invalid message size received: %d (0x%x)",
				client->msg.size_message, client->msg.size_message);
		ft_printf("Error: Invalid message size: %d (0x%x)\n",
				  client->msg.size_message, client->msg.size_message);
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
	
	ft_printf("DEBUG_MEM: Memory allocated successfully for %d bytes\n", client->msg.size_message);
	ft_printf("DEBUG_MEM: Switching to message reception mode\n");
	
	client->getting_header = 0;
	client->getting_msg = 1;
	client->sig_count = 0;
	
	ft_printf("DEBUG_MEM: getting_header=%d, getting_msg=%d, sig_count=%d\n",
		client->getting_header, client->getting_msg, client->sig_count);
	
	log_msg(LOG_SUCCESS, "Memory allocated successfully for %d bytes, "
						 "switching to message reception mode",
			client->msg.size_message);
}
