/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 17:25:12 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:41:59 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

void	handle_timeout(t_client_state *client, int i)
{
	if (i >= TIMEOUT && client->actual_pid > 0)
	{
		log_msg(LOG_WARNING, "Client timeout after %d seconds for PID %d", TIMEOUT, client->actual_pid);
		clean_global();
	}
}

int	monitor_client_timeout(t_client_state *client)
{
	int	i;
	int saved_pid;

	saved_pid = client->actual_pid;
	i = 0;
	while (i < TIMEOUT)
	{
		usleep(SLEEP_TIME);
		
		// Check if client changed during monitoring
		if (client->actual_pid != saved_pid)
		{
			log_msg(LOG_DEBUG, "Client changed during timeout monitoring, stopping");
			return (0);
		}
		
		if (check_client_disconnection(client))
			return (0);
		if (check_client_activity(client, &i))
		{
			saved_pid = client->actual_pid; // Update saved PID if client is active
			continue;
		}
		i++;
	}
	
	// Only handle timeout if we're still monitoring the same client
	if (client->actual_pid == saved_pid)
		handle_timeout(client, i);
	return (0);
}
