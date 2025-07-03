/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 02:22:54 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:59:54 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

static void	log_bit_details(int bit_position, int bit_value,
	t_client_state *client)
{
	unsigned int	mask;

	mask = 0;
	if (bit_value)
		mask = (1U << bit_position);
	log_msg(LOG_DEBUG, "Bit position: %d, value: %d, mask: 0x%x, size: %u",
		bit_position, bit_value, mask,
		(unsigned int)client->msg.size_message);
}

static void	process_header_bit(int bit_value, t_client_state *client)
{
	int	bit_position;

	bit_position = HEADER_SIZE - 1 - client->sig_count;
	if (bit_value == 1)
		client->msg.size_message |= (1U << bit_position);
	client->sig_count++;
	log_msg(LOG_DEBUG, "Header bit %d/%d: %d (bit_pos: %d, currsize: %u)",
		client->sig_count, HEADER_SIZE, bit_value, bit_position,
		(unsigned int)client->msg.size_message);
	if (client->sig_count <= 8)
		log_bit_details(bit_position, bit_value, client);
}

static int	validate_message_size(int size)
{
	if (size <= 0 || size > 10000000)
	{
		log_msg(LOG_ERROR, "Invalid message size: %d bytes", size);
		ft_printf("Error: Invalid message size received: %d\n", size);
		return (0);
	}
	return (1);
}

void	handle_header(int signum)
{
	const int		bit_value = get_bit_value(signum);
	t_client_state	*client;

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
		log_msg(LOG_INFO, "Header complete: message size = %d bytes (0x%x)",
			client->msg.size_message, (unsigned int)client->msg.size_message);
		ft_printf("DEBUG: Received message size: %d bytes\n", client->msg.size_message);
		if (!validate_message_size(client->msg.size_message))
		{
			reset_client_state(client);
			return ;
		}
		memory_reserve_to_store_signals();
	}
}

void	keep_server_up(void)
{
	while (1)
		check_clean_status();
}
