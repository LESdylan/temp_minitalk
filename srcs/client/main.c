/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 02:15:33 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 12:35:56 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

static void handle_completion(void)
{
	ft_printf("DEBUG: handle_completion() called - server confirmed message!\n");
	ft_printf("✓ Message successfully received and confirmed by server!\n");
	log_msg(LOG_SUCCESS, "Message delivery confirmed by server");

	// Add a small delay to ensure all signals are processed
	usleep(100000); // 100ms

	ft_printf("DEBUG: Exiting after successful completion\n");
	exit(EXIT_SUCCESS);
}

void signal_handler(int signum, siginfo_t *info, void *context)
{
	t_server_state *server;

	(void)context;
	server = get_server_instance();

	ft_printf("DEBUG: Client signal_handler() called with signal %s from PID %d\n",
			  signum == SIGUSR1 ? "SIGUSR1" : "SIGUSR2", info->si_pid);

	// Validate signal source
	if (!validate_signal_source(server, info->si_pid))
	{
		ft_printf("DEBUG: Signal validation failed, ignoring signal\n");
		return;
	}

	if (signum == SIGUSR2)
	{
		ft_printf("DEBUG: Received SIGUSR2, transmission_active = %d\n", server->transmission_active);
		// SIGUSR2 is used for acknowledgments and ready signals
		if (server->transmission_active)
		{
			// Only handle if we're waiting for an ACK
			if (server->ready_to_proceed == 0)
			{
				ft_printf("DEBUG: Handling ACK signal\n");
				handle_acknowledgment(server);
			}
			else
			{
				ft_printf("DEBUG: Ignoring SIGUSR2 - not waiting for ACK\n");
			}
		}
		else
		{
			ft_printf("DEBUG: Handling PONG signal during ping\n");
			// This is a ready signal during ping
			handle_pong(signum, server, info->si_pid);
		}
	}
	else if (signum == SIGUSR1)
	{
		ft_printf("DEBUG: Received SIGUSR1, transmission_active = %d\n", server->transmission_active);
		// SIGUSR1 is used for completion signals and busy signals
		if (server->transmission_active)
		{
			ft_printf("DEBUG: This is a COMPLETION signal! Handling completion...\n");
			// This is a completion signal - handle immediately
			log_msg(LOG_SUCCESS, "Received completion signal from server");
			handle_completion();
		}
		else
		{
			ft_printf("DEBUG: This is a BUSY signal during ping\n");
			// This is a busy signal during ping
			handle_pong(signum, server, info->si_pid);
		}
	}

	ft_printf("DEBUG: signal_handler() finished\n");
}

int main(int argc, char **argv)
{
	t_client data;
	int msg_len;

	validate_and_init(argc, argv, &data);
	establish_connection(&data);
	msg_len = ft_strlen(argv[2]);
	prepare_transmission(&data, msg_len);
	while (1)
		pause();
	return (EXIT_SUCCESS);
}
