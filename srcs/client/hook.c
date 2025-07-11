/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hook.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 16:21:00 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:35:56 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void establish_connection(t_client *data)
{
	if (!connect_to_server(data))
	{
		ft_printf("Failed to connect to server\n");
		exit(EXIT_FAILURE);
	}
}

int connect_to_server(t_client *data)
{
	log_msg(LOG_INFO, "Attempting to connect to server PID %d",
			data->server_pid);
	if (kill(data->server_pid, 0) == -1)
	{
		ft_printf("Error: Server process PID %d not accessible\n",
				  data->server_pid);
		log_msg(LOG_ERROR, "Server process validation failed");
		return (0);
	}
	if (ping(data->server_pid) == 0)
		return (0);
	return (1);
}

void start_transmission(t_client *data, int msg_len)
{
	int actual_len;
	int message_checksum;

	wait_for_transmission_slot(data);

	// Double-check the message length
	actual_len = ft_strlen(data->msg);
	if (actual_len != msg_len)
	{
		ft_printf("Error: Message length mismatch! Expected %d, got %d\n", msg_len, actual_len);
		log_msg(LOG_ERROR, "Message length mismatch: expected %d, actual %d", msg_len, actual_len);
		exit(EXIT_FAILURE);
	}

	// Calculate message checksum for validation
	message_checksum = calculate_checksum(data->msg, msg_len);

	ft_printf("Starting transmission (%d characters, checksum: %d/0x%x)...\n",
			  msg_len, message_checksum, (unsigned int)message_checksum);
	log_msg(LOG_INFO, "Starting header transmission (message length: %d, checksum: %d/0x%x)",
			msg_len, message_checksum, (unsigned int)message_checksum);

	// Validate message length before sending
	if (msg_len <= 0 || msg_len > 10000000)
	{
		ft_printf("Error: Invalid message length: %d\n", msg_len);
		log_msg(LOG_ERROR, "Invalid message length: %d", msg_len);
		exit(EXIT_FAILURE);
	}

	// Debug: Show what we're about to send
	ft_printf("DEBUG: Sending header with value %d (0x%x)\n", msg_len, msg_len);

	// Send header (message length)
	send_signals(&msg_len, 32, data);
	log_msg(LOG_INFO, "Header transmission complete, starting message content");

	// Send message content
	send_message(data->msg, data);
	log_msg(LOG_INFO, "All data transmitted, waiting for final confirmation");

	// Wait for completion confirmation with debug info
	ft_printf("DEBUG: All message data sent successfully\n");
	ft_printf("DEBUG: Client PID %d is now waiting for completion signal from server PID %d\n",
			  getpid(), data->server_pid);
	ft_printf("Waiting for server confirmation...\n");

	// Add periodic debug output while waiting
	static int wait_count = 0;
	wait_count = 0;

	// Instead of alarm, use a loop with debug output
	while (1)
	{
		sleep(5);
		wait_count += 5;
		ft_printf("DEBUG: Still waiting for completion signal... (%d seconds)\n", wait_count);

		// Check if server is still alive
		if (kill(data->server_pid, 0) == -1)
		{
			ft_printf("ERROR: Server process %d no longer exists!\n", data->server_pid);
			exit(EXIT_FAILURE);
		}

		if (wait_count >= 30)
		{
			ft_printf("ERROR: Timeout waiting for completion signal after 30 seconds\n");
			exit(EXIT_FAILURE);
		}
	}
}

void set_transmission_active(pid_t client_pid)
{
	t_server_state *server;

	server = get_server_instance();
	server->transmission_active = 1;
	server->current_client_pid = client_pid;
	server->transmission_id++;
	log_msg(LOG_INFO, "Transmission activated for client PID %d (ID: %d)",
			client_pid, server->transmission_id);
}

void end_transmission(void)
{
	t_server_state *server;

	server = get_server_instance();
	log_msg(LOG_INFO, "Ending transmission for client PID %d",
			server->current_client_pid);
	server->transmission_active = 0;
	server->current_client_pid = 0;
	server->ready_to_proceed = 0;
}
