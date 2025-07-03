/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 15:45:49 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:59:55 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

int	check_server_and_sleep(void)
{
	const int		total_sleep = RETRY_TIME * 1000 * 1000;
	const int		sleep_time = total_sleep / RETRY_INTERVALS;
	int				i;
	t_server_state	*server;

	server = get_server_instance();
	i = 0;
	while (i <= total_sleep)
	{
		usleep(sleep_time);
		if (server->is_ready == 1)
			return (1);
		i += sleep_time;
	}
	return (0);
}

int	is_transmission_owner(pid_t client_pid)
{
	t_server_state	*server;

	server = get_server_instance();
	return (server->transmission_active
		&& server->current_client_pid == client_pid);
}

void	check_transmission_ownership(pid_t my_pid, int total_chars, int i)
{
	int	percentage;
	int	progress_interval;

	if (!is_transmission_owner(my_pid))
	{
		log_msg(LOG_ERROR, "Lost transmission ownership during message send");
		exit(EXIT_FAILURE);
	}
	
	// Dynamic progress interval based on message size
	if (total_chars > 50000)
		progress_interval = 1000;  // Every 1000 chars for very large messages
	else if (total_chars > 10000)
		progress_interval = 500;   // Every 500 chars for large messages
	else if (total_chars > 1000)
		progress_interval = 100;   // Every 100 chars for medium messages
	else
		progress_interval = 50;    // Every 50 chars for small messages
	
	if (total_chars > 1000 && (i % progress_interval == 0) && i > 0)
	{
		percentage = (i * 100) / total_chars;
		ft_printf("Progress: %d%% (%d/%d characters)\n", percentage, i,
			total_chars);
	}
}
