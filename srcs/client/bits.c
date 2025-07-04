/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bits.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 02:11:03 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:30:18 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void send_signal(pid_t pid, int signal)
{
	const char *signal_name;

	if (signal == SIGUSR1)
		signal_name = "SIGUSR1 (0)";
	else
		signal_name = "SIGUSR2 (1)";

	if (pid <= 0)
	{
		ft_printf("Error: Invalid server PID: %d\n", pid);
		log_msg(LOG_ERROR, "Invalid server PID: %d", pid);
		exit(EXIT_FAILURE);
	}

	if (kill(pid, 0) == -1)
	{
		ft_printf("Error: Server process PID %d no longer exists\n", pid);
		log_msg(LOG_ERROR, "Server process PID %d disappeared", pid);
		exit(EXIT_FAILURE);
	}
	if (kill(pid, signal) == -1)
	{
		ft_printf("Error: Signal sending failed to PID %d\n", pid);
		log_msg(LOG_ERROR, "Failed to send signal %d to PID %d: %s", signal, pid, strerror(errno));
		exit(EXIT_FAILURE);
	}
	log_msg(LOG_DEBUG, "Sent signal %s to server PID %d", signal_name, pid);
}

void send_data_bit(int bit_value, t_client *info)
{
	int signal_to_send;

	if (bit_value)
		signal_to_send = CHAR_1; // SIGUSR2
	else
		signal_to_send = CHAR_0; // SIGUSR1

	send_signal(info->server_pid, signal_to_send);
}

void send_bit(unsigned long long value, int i, t_client *info)
{
	int bit_value;

	bit_value = 0;
	if (value & (1ULL << i))
		bit_value = 1;

	// Debug output for all bits when sending header (32-bit value)
	if (value <= 10000000) // This is likely a header (message length)
	{
		ft_printf("Sending bit position %d: %d (value 0x%llx, mask 0x%llx) -> %s\n",
				  i, bit_value, value, (1ULL << i), bit_value ? "SIGUSR2" : "SIGUSR1");
	}

	send_data_bit(bit_value, info);

	// Wait for acknowledgment from server immediately
	wait_for_server_ack();
}

void send_signals(void *data, size_t bit_length, t_client *info)
{
	unsigned long long value;
	int i;

	value = 0;
	if (bit_length == 8)
		value = *((unsigned char *)data);
	else if (bit_length == 32)
	{
		value = *((unsigned int *)data);
		if (value > 10000000 || value == 0)
		{
			ft_printf("Error: Invalid message length: %llu\n", value);
			log_msg(LOG_ERROR, "Invalid message length: %llu", value);
			exit(EXIT_FAILURE);
		}
	}

	// Show binary representation only for 32-bit values
	if (bit_length == 32)
	{
		ft_printf("Sending binary: ");
		for (int j = 31; j >= 0; j--)
		{
			int bit = (value & (1ULL << j)) ? 1 : 0;
			ft_printf("%d", bit);
			if (j % 8 == 0 && j > 0)
				ft_printf(" ");
		}
		ft_printf(" (MSB first)\n");
		ft_printf("Value to send: %llu (0x%llx)\n", value, value);

		// Add longer synchronization delay to prevent race conditions
		usleep(50000); // 50ms delay before header transmission
	}

	// Send MSB first: for 32-bit, send bits 31, 30, 29, ..., 1, 0
	i = bit_length - 1;
	while (i >= 0)
	{
		send_bit(value, i, info);
		i--;
	}

	if (bit_length == 32)
	{
		ft_printf("Header transmission complete.\n");
		// Add small delay after header to ensure server processes correctly
		usleep(10000); // 10ms delay
	}

	log_msg(LOG_SUCCESS, "Successfully sent %zu bits (value: %llu)", bit_length, value);
}