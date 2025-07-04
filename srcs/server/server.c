/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 02:22:54 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:30:19 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

static void process_header_bit(int bit_value, t_client_state *client)
{
	int bit_position;

	// CRITICAL FIX: The issue is the server thinks it's getting extra signals
	// Check if we're getting more signals than expected
	if (client->sig_count >= HEADER_SIZE)
	{
		ft_printf("ERROR: Received more than 32 header bits! sig_count=%d\n", client->sig_count);
		log_msg(LOG_ERROR, "Signal overflow detected: sig_count=%d", client->sig_count);
		return;
	}

	// Client sends MSB first: bit 31, 30, 29, ..., 1, 0
	bit_position = (HEADER_SIZE - 1) - client->sig_count;

	ft_printf("DEBUG_BIT: Processing signal %d, position %d, value %d\n", 
		client->sig_count + 1, bit_position, bit_value);

	// Only set the bit if bit_value is 1
	if (bit_value == 1)
	{
		client->msg.size_message |= (1U << bit_position);
		ft_printf("DEBUG_BIT: Set bit at position %d, new value: 0x%x\n", 
			bit_position, client->msg.size_message);
	}

	// INCREMENT AFTER processing
	client->sig_count++;

	// Add validation to detect bit shift issues early
	if (client->sig_count == HEADER_SIZE)
	{
		ft_printf("DEBUG_BIT: Final value after %d bits: %d (0x%x)\n", 
			HEADER_SIZE, client->msg.size_message, client->msg.size_message);
		
		// Final validation - check if we got a reasonable value
		if (client->msg.size_message > 100000 || client->msg.size_message <= 0)
		{
			ft_printf("ERROR: Detected bit shift issue! Received: %d (0x%x)\n",
					  client->msg.size_message, client->msg.size_message);
			log_msg(LOG_ERROR, "Bit shift detected: received %d instead of expected range 1-100000",
					client->msg.size_message);

			// Reset and ask client to retry
			reset_client_state(client);
			return;
		}
	}
}

static int validate_message_size(int size)
{
	if (size <= 0 || size > 10000000)
	{
		log_msg(LOG_ERROR, "Invalid message size: %d bytes", size);
		ft_printf("Error: Invalid message size received: %d\n", size);
		return (0);
	}
	return (1);
}

void handle_header(int signum)
{
	const int bit_value = get_bit_value(signum);
	t_client_state *client;

	client = get_client_instance();

	if (client->sig_count == 0)
	{
		client->msg.size_message = 0;
		log_msg(LOG_DEBUG, "Starting header reception (message length)");
	}
	if (client->sig_count < HEADER_SIZE)
		process_header_bit(bit_value, client);
	if (client->sig_count == HEADER_SIZE)
	{
		ft_printf("DEBUG: Received message size: %d bytes\n", client->msg.size_message);
		if (!validate_message_size(client->msg.size_message))
		{
			reset_client_state(client);
			return;
		}
		memory_reserve_to_store_signals();
	}
}

void keep_server_up(void)
{
	while (1)
		check_clean_status();
}