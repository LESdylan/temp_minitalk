/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 19:00:00 by dlesieur          #+#    #+#             */
/*   Updated: 2025/07/03 19:41:59 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

int	pong(int pid)
{
	t_client_state	*client;

	client = get_client_instance();
	if (is_server_busy() && client->actual_pid != pid)
	{
		log_msg(LOG_INFO, "Server busy with PID %d, sending busy signal to client %d", client->actual_pid, pid);
		if (kill(pid, SERVER_BUSY) == -1)
		{
			log_msg(LOG_ERROR, "Failed to send busy signal to PID %d", pid);
		}
		return (EXIT_SUCCESS);
	}
	
	// Clean any previous state before accepting new client
	if (client->msg.message)
	{
		free(client->msg.message);
		client->msg.message = NULL;
	}
	
	ft_memset(client, 0, sizeof(*client));
	client->actual_pid = pid;
	client->client_pid = pid;
	client->getting_header = 1;
	client->getting_msg = 0;
	client->sig_count = 0;
	client->msg_pos = 0;
	client->char_value = 0;
	client->client_activity = 1;
	set_server_busy(pid);
	log_msg(LOG_INFO, "Client %d connected, sending ready signal", pid);
	
	if (kill(pid, SERVER_READY) == -1)
	{
		log_msg(LOG_ERROR, "Failed to send ready signal to PID %d", pid);
		clean_global();
		return (EXIT_FAILURE);
	}
	
	log_msg(LOG_SUCCESS, "Server ready signal sent to client %d", pid);
	return (EXIT_SUCCESS);
}

void	send_multiple_acks(pid_t client_pid)
{
	if (client_pid <= 0)
	{
		log_msg(LOG_ERROR, "Invalid client PID for acknowledgment: %d", client_pid);
		return;
	}
	
	if (kill(client_pid, SIGUSR2) == -1)
	{
		log_msg(LOG_ERROR, "Failed to send acknowledgment to client %d", client_pid);
		return;
	}
	log_msg(LOG_DEBUG, "Sent acknowledgment signal to client %d", client_pid);
}

int	lost_signal(int s_si_pid, int signum, void *context)
{
	t_client_state	*client;

	(void)context;
	client = get_client_instance();
	
	// Handle invalid PID cases
	if (s_si_pid <= 0)
	{
		if (client->actual_pid > 0)
		{
			log_msg(LOG_WARNING, "Received signal %d with PID %d, using current client: %d", 
				signum, s_si_pid, client->actual_pid);
			return (client->actual_pid);
		}
		log_msg(LOG_ERROR, "Invalid sender PID: %d for signal %d", s_si_pid, signum);
		return (0);
	}
	
	return (s_si_pid);
}

void	send_completion_signal(pid_t client_pid)
{
	if (client_pid > 0)
	{
		log_msg(LOG_INFO, "Sending completion signal to client %d",
			client_pid);
		kill(client_pid, SIGUSR1);
	}
}
