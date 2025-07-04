/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/03 03:15:33 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:30:14 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void handle_pong(int signum, t_server_state *server, pid_t pid)
{
	if (signum == SIGUSR2)
	{
		server->is_ready = 1;
		log_msg(LOG_SUCCESS, "Server ready signal received from PID %d", pid);
	}
	else if (signum == SIGUSR1)
	{
		server->is_ready = -1; // Use -1 to indicate busy
		log_msg(LOG_WARNING, "Server busy signal received from PID %d", pid);
	}
}

void send_message(char *str, t_client *data)
{
	int i;
	pid_t my_pid;
	int total_chars;

	i = 0;
	my_pid = getpid();
	total_chars = ft_strlen(str);
	log_msg(LOG_INFO, "Starting message transmission: %d characters",
			total_chars);
	while (str[i])
	{
		check_transmission_ownership(my_pid, total_chars, i);
		log_msg(LOG_DEBUG, "Sending character %d: '%c' (ASCII: %d)",
				i + 1, str[i], str[i]);
		send_signals(&str[i], 8, data);
		i++;
	}
	if (total_chars > 1000)
		ft_printf("Progress: 100%% (%d/%d characters) - Complete!\n",
				  i, total_chars);
	log_msg(LOG_SUCCESS, "Message transmission complete: %d characters sent",
			i);
}
