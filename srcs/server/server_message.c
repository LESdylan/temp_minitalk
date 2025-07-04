/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_message.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 18:24:27 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:35:56 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

void handle_complete_message(t_client_state *client)
{
	pid_t client_pid;
	pid_t next_client;
	int calculated_checksum;

	ft_printf("DEBUG: handle_complete_message() called\n");
	ft_printf("DEBUG: client->msg_pos = %d, client->msg.size_message = %d\n",
			  client->msg_pos, client->msg.size_message);

	client->msg.message[client->msg_pos] = '\0';

	// Calculate checksum for validation
	calculated_checksum = calculate_checksum(client->msg.message, client->msg_pos);

	log_msg(LOG_SUCCESS, "Message reception complete: %d characters received", client->msg_pos);

	// Display the message immediately
	print_message(client);

	// Store client PID before any cleanup - CRITICAL
	client_pid = client->client_pid;
	ft_printf("DEBUG: Stored client_pid = %d for completion signals\n", client_pid);

	// Check if client process still exists before sending signals
	if (kill(client_pid, 0) == -1)
	{
		ft_printf("ERROR: Client process %d no longer exists! Cannot send completion signals.\n", client_pid);
		log_msg(LOG_ERROR, "Client process %d disappeared before completion signals", client_pid);
		clean_global();
		return;
	}

	// Display confirmation message
	ft_printf("✓ Message successfully received and confirmed to client %d (checksum: %d/0x%x)\n",
			  client_pid, calculated_checksum, (unsigned int)calculated_checksum);

	// Send completion signal IMMEDIATELY and RELIABLY
	ft_printf("DEBUG: Starting completion signal transmission to client %d\n", client_pid);
	log_msg(LOG_INFO, "Sending completion confirmation (SIGUSR1) to client %d", client_pid);

	// CRITICAL: Send completion signals with immediate delivery
	for (int i = 0; i < 10; i++)
	{
		ft_printf("DEBUG: Sending completion signal %d/10 to client %d\n", i + 1, client_pid);

		if (kill(client_pid, SIGUSR1) == -1)
		{
			ft_printf("ERROR: Failed to send completion signal %d to client %d: %s\n",
					  i + 1, client_pid, strerror(errno));
			log_msg(LOG_ERROR, "Failed to send completion signal %d to client %d: %s",
					i + 1, client_pid, strerror(errno));
			break;
		}
		else
		{
			ft_printf("DEBUG: Successfully sent completion signal %d to client %d\n", i + 1, client_pid);
		}

		usleep(5000); // 5ms delay between signals for reliable delivery
	}

	ft_printf("DEBUG: Completed sending all completion signals to client %d\n", client_pid);

	// Force flush output to ensure message is displayed
	fflush(stdout);

	log_msg(LOG_SUCCESS, "Completion signals sent to client %d", client_pid);

	// Cleanup current client message
	if (client->msg.message)
	{
		free(client->msg.message);
		client->msg.message = NULL;
	}

	// Check for next client in queue BEFORE resetting state
	next_client = dequeue_client();

	if (next_client > 0)
	{
		ft_printf("DEBUG: Found next client %d in queue\n", next_client);
		log_msg(LOG_INFO, "Serving next client in queue: PID %d (queue size: %d)", next_client, client->queue_size);

		// Reset transmission state but preserve queue
		client->actual_pid = next_client;
		client->client_pid = next_client;
		client->getting_header = 1;
		client->getting_msg = 0;
		client->sig_count = 0;
		client->msg_pos = 0;
		client->char_value = 0;
		client->client_activity = 1;
		client->transmission_active = 1;
		client->msg.size_message = 0;

		// Give client time to be ready for the signal
		usleep(50000); // 50ms delay to ensure client is ready

		// Send ready signal to queued client
		for (int i = 0; i < 7; i++)
		{
			if (kill(next_client, SIGUSR2) == -1)
			{
				log_msg(LOG_ERROR, "Failed to send ready signal to queued client %d", next_client);
				clean_global();
				return;
			}
			usleep(5000); // 5ms between ready signals
		}

		log_msg(LOG_SUCCESS, "Ready signal sent to queued client %d", next_client);
	}
	else
	{
		ft_printf("DEBUG: No more clients in queue, resetting server state\n");
		log_msg(LOG_INFO, "No more clients in queue, server released");
		// Completely reset the server state
		reset_client_state(client);
		ft_printf("Server is now free and ready for new connections.\n");
	}

	ft_printf("DEBUG: handle_complete_message() finished\n");
}

void process_character(t_client_state *client, int bit_value, int bit_pos)
{
	client->char_value |= (bit_value << (7 - bit_pos));
	client->sig_count++;
	log_msg(LOG_DEBUG, "Bit %d/8: %d (char value: %d)",
			bit_pos + 1, bit_value, client->char_value);
	if (client->sig_count % 8 == 0)
	{
		client->msg.message[client->msg_pos] = client->char_value;
		log_msg(LOG_DEBUG, "Character complete: '%c'"
						   " (ASCII: %d) at position %d/%d",
				client->char_value, client->char_value, client->msg_pos + 1,
				client->msg.size_message);
		client->msg_pos++;
		client->char_value = 0;
		if (client->msg_pos >= client->msg.size_message)
			handle_complete_message(client);
	}
}

// Set the bit in the correct position (MSB first: 7, 6, 5, ..., 0)
void handle_msg(int signum)
{
	t_client_state *client;
	const int bit_value = get_bit_value(signum);
	int bit_pos;

	client = get_client_instance();
	bit_pos = client->sig_count % 8;

	ft_printf("DEBUG_MSG: handle_msg() called, sig_count=%d, bit_pos=%d, bit_value=%d\n", 
		client->sig_count, bit_pos, bit_value);
	ft_printf("DEBUG_MSG: msg_pos=%d/%d, char_value=0x%x\n", 
		client->msg_pos, client->msg.size_message, client->char_value);

	if (bit_pos == 0)
	{
		client->char_value = 0;
		ft_printf("DEBUG_MSG: Starting new character %d/%d\n",
				client->msg_pos + 1, client->msg.size_message);
	}

	// Client sends MSB first: bit 7, 6, 5, 4, 3, 2, 1, 0
	// Set the bit at position (7 - bit_pos)
	if (bit_value == 1)
		client->char_value |= (1 << (7 - bit_pos));

	client->sig_count++;

	ft_printf("DEBUG_MSG: After processing bit: char_value=0x%x, sig_count=%d\n",
		client->char_value, client->sig_count);

	// Complete character after 8 bits
	if (bit_pos == 7)
	{
		client->msg.message[client->msg_pos] = client->char_value;
		ft_printf("DEBUG_MSG: Character complete: '%c' (0x%x) at position %d/%d\n",
			client->char_value, client->char_value, client->msg_pos + 1, client->msg.size_message);
		
		client->msg_pos++;
		client->char_value = 0;

		ft_printf("DEBUG_MSG: Checking completion: msg_pos=%d, size_message=%d\n",
			client->msg_pos, client->msg.size_message);

		if (client->msg_pos >= client->msg.size_message)
		{
			ft_printf("DEBUG_MSG: Message complete! Calling handle_complete_message()\n");
			handle_complete_message(client);
		}
		else
		{
			ft_printf("DEBUG_MSG: Message not yet complete, waiting for more data\n");
		}
	}
}