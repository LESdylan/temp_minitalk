/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inits.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 15:46:56 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 19:59:55 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void	init_data(char **argv, t_client *data)
{
	int msg_len;

	ft_memset(data, 0, sizeof(t_client));
	data->server_pid = ft_atoi(argv[1]);
	data->client_pid = getpid();
	data->msg = argv[2];
	
	if (data->server_pid <= 0)
	{
		ft_printf("Error: %s\n", BAD_SIGNAL);
		log_msg(LOG_ERROR, "Invalid server PID: %s", argv[1]);
		exit(EXIT_FAILURE);
	}
	
	// Debug: Show actual message length and content preview
	msg_len = ft_strlen(data->msg);
	ft_printf("DEBUG: Message length is %d characters\n", msg_len);
	if (msg_len <= 50)
		ft_printf("DEBUG: Message content: '%s'\n", data->msg);
	else
		ft_printf("DEBUG: Message preview (first 50 chars): '%.50s...'\n", data->msg);
	
	log_msg(LOG_DEBUG, "Client PID: %d, Server PID: %d, Message length: %d", 
		data->client_pid, data->server_pid, msg_len);
}

void	setup_ping_signals(struct sigaction *sa, sigset_t *sigset)
{
	sigemptyset(sigset);
	sigaddset(sigset, SIGUSR1);
	sigaddset(sigset, SIGUSR2);
	sa->sa_flags = SA_SIGINFO | SA_RESTART;
	sa->sa_sigaction = ping_handler;
	sa->sa_mask = *sigset;

	if (sigaction(SIGUSR1, sa, NULL) == -1 || sigaction(SIGUSR2, sa, NULL) == -1)
	{
		log_msg(LOG_ERROR, "Failed to setup ping signal handlers");
		exit(EXIT_FAILURE);
	}
}

void	setup_signal_handlers(sigset_t *sigset, struct sigaction *sa)
{
	sigemptyset(sigset);
	sigaddset(sigset, SIGUSR1);
	sigaddset(sigset, SIGUSR2);
	sa->sa_flags = SA_SIGINFO | SA_RESTART;
	sa->sa_sigaction = signal_handler;
	sa->sa_mask = *sigset;

	if (sigaction(SIGUSR1, sa, NULL) == -1 || sigaction(SIGUSR2, sa, NULL) == -1)
	{
		log_msg(LOG_ERROR, "Failed to setup signal handlers");
		exit(EXIT_FAILURE);
	}
}

void	reset_server_state(void)
{
	t_server_state	*server;

	server = get_server_instance();
	server->pid = 0;
	server->is_ready = 0;
	server->ready_to_proceed = 0;
	server->transmission_active = 0;
	server->current_client_pid = 0;
	server->transmission_id = 0;
	server->last_sequence = 0;
	server->ack_count = 0;
}

void	prepare_transmission(t_client *data, int msg_len)
{
	struct sigaction	sa;
	sigset_t			sigset;

	ft_printf("Sending message (%d characters)\n", msg_len);
	log_msg(LOG_INFO, "Message length: %d characters", msg_len);
	setup_signal_handlers(&sigset, &sa);
	start_transmission(data, msg_len);
}
