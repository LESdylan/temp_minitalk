/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bits.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 02:11:03 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:59:52 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void	send_signal(pid_t pid, int signal)
{
	const char	*signal_name;

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

void	send_data_bit(int bit_value, t_client *info)
{
	if (bit_value)
		send_signal(info->server_pid, CHAR_1);
	else
		send_signal(info->server_pid, CHAR_0);
}

void	send_bit(unsigned long long value, int i, t_client *info)
{
	int	bit_value;
	int retry_count = 0;
	int max_retries = 3;

	bit_value = 0;
	if (value &(1ULL << i))
		bit_value = 1;
	log_msg(LOG_DEBUG, "Sending bit %d: %d to server PID %d", i, bit_value,
		info->server_pid);
	
	while (retry_count < max_retries)
	{
		send_data_bit(bit_value, info);
		
		// Try to get acknowledgment
		t_server_state *server = get_server_instance();
		server->ready_to_proceed = 0;
		
		// Wait for ACK with timeout
		int timeout = 0;
		while (!server->ready_to_proceed && timeout < 50000) // 5 second timeout
		{
			usleep(100);
			timeout++;
		}
		
		if (server->ready_to_proceed)
		{
			server->ready_to_proceed = 0;
			return; // Success
		}
		
		retry_count++;
		if (retry_count < max_retries)
		{
			log_msg(LOG_WARNING, "No ACK received for bit %d, retrying (%d/%d)", 
				i, retry_count, max_retries);
			usleep(10000); // Wait 10ms before retry
		}
	}
	
	// If we get here, all retries failed
	ft_printf("Error: Failed to send bit after %d retries\n", max_retries);
	log_msg(LOG_ERROR, "Bit transmission failed after %d retries", max_retries);
	exit(EXIT_FAILURE);
}

void	send_signals(void *data, size_t bit_length, t_client *info)
{
	unsigned long long	value;
	int					i;

	value = 0;
	if (bit_length == 8)
		value = *((unsigned char *)data);
	else if (bit_length == 32)
	{
		value = *((unsigned int *)data);
		// Add validation for message length
		if (value > 10000000 || value == 0)
		{
			ft_printf("Error: Invalid message length: %llu\n", value);
			log_msg(LOG_ERROR, "Invalid message length: %llu", value);
			exit(EXIT_FAILURE);
		}
	}
	log_msg(LOG_INFO, "Sending %zu-bit value: %llu (0x%llx) to server PID %d",
		bit_length, value, value, info->server_pid);
	i = bit_length - 1;
	while (i >= 0)
	{
		send_bit(value, i, info);
		i--;
	}
	log_msg(LOG_SUCCESS, "Successfully sent %zu bits (value: %llu)", bit_length, value);
}
