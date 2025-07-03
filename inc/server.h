/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 11:19:40 by dlesieur          #+#    #+#             */
<<<<<<< Updated upstream
/*   Updated: 2025/07/03 18:12:01 by codespace        ###   ########.fr       */
=======
/*   Updated: 2025/07/03 17:33:24 by codespace        ###   ########.fr       */
>>>>>>> Stashed changes
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
# define SERVER_H

# include "libft.h"
# include "auto_parser.h"

# define HEADER_SIZE 32
# define HEADER_MESSAGE "Message received: "
# define SIGNAL_RECEIVED SIGUSR1
# define SERVER_READY SIGUSR1
# define SERVER_BUSY SIGUSR2
# define EXIT_SUCCESS 0
# define EXIT_FAILURE 1
<<<<<<< Updated upstream
# define TIMEOUT	50000
# define SLEEP_TIME 50000

=======
# define TIMEOUT 5
# define SLEEP_TIME 500000
# define TIMEOUT 5
# define SLEEP_TIME 500000
>>>>>>> Stashed changes

typedef struct s_message
{
	char	*message;
	int		size_message;
}	t_message;

typedef struct s_client_state
{
	pid_t		actual_pid;
	pid_t		client_pid;
	int			getting_header;
	int			getting_msg;
	int			sig_count;
	int			char_value;
	int			msg_pos;
	int			client_activity;
	int			transmission_active;
	int			queue_position;
	int			sequence_number;
	int			expected_checksum;
	int			calculated_checksum;
	t_message	msg;
}	t_client_state;

/* Singleton functions */
t_client_state	*get_client_instance(void);
void			reset_client_state(t_client_state *client);

/* Queue management functions */
int				is_server_busy(void);
void			set_server_busy(pid_t client_pid);

/* Signal handling functions */
void			signal_handler(int signum, siginfo_t *info, void *context);
int				pong(int pid);
void			handle_header(int signum);
void			handle_msg(int signum);
void			memory_reserve_to_store_signals(void);

/* Server loop functions */
void			keep_server_up(void);
int				check_clean_status(void);
void			clean_global(void);

/* Utility functions */
int				ft_printf(const char *format, ...);
void			*ft_memset(void *s, int c, size_t n);
void			ft_bzero(void *s, size_t n);
int				get_bit_value(int signum);
void			process_character(t_client_state *client, int bit_value, int bit_pos);
void			print_message(t_client_state *client);
void			handle_complete_message(t_client_state *client);
int				check_client_disconnection(t_client_state *client);
int				check_client_activity(t_client_state *client, int *i);
void			handle_timeout(t_client_state *client, int i);
void			send_completion_signal(pid_t client_pid);

/* Checksum functions */
int				calculate_checksum(const char *data, int length);
void			send_multiple_acks(pid_t client_pid);
int				lost_signal(int s_si_pid, int signum, void *context);
void			send_completion_signal(pid_t client_pid);
#endif
