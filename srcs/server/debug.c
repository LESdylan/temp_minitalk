/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:27:16 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 11:47:37 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

void set_server_busy(pid_t client_pid)
{
	t_client_state *client;

	client = get_client_instance();
	client->transmission_active = 1;
	client->actual_pid = client_pid;
	log_msg(LOG_INFO, "Server now busy with client PID %d", client_pid);
}

int check_client_activity(t_client_state *client, int *i)
{
	if (client->client_activity)
	{
		client->client_activity = 0;
		*i = 0;
		return (1);
	}
	return (0);
}

void print_message(t_client_state *client)
{
	write(1, HEADER_MESSAGE, 18);
	write(1, client->msg.message, client->msg.size_message);
	write(1, "\n", 1);

	// Only show confirmation without repeating the message
	ft_printf("✓ Message from client %d successfully received and displayed!\n", client->client_pid);
	ft_printf("Message length: %d bytes\n", client->msg.size_message);
}

void log_character_completion(t_client_state *client)
{
	char display_char;

	if (client->char_value >= 32 && client->char_value <= 126)
		display_char = client->char_value;
	else
		display_char = '?';
	log_msg(LOG_DEBUG, "Character complete: '%c' (ASCII: %d) at position %d/%d",
			display_char, client->char_value, client->msg_pos + 1,
			client->msg.size_message);
}