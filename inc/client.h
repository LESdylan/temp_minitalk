/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 11:19:45 by dlesieur          #+#    #+#             */
/*   Updated: 2025/07/04 12:30:14 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H
#define CLIENT_H

#include "libft.h"
#include "auto_parser.h"

#define RETRY_TIMES 20
#define RETRY_TIME 1
#define RETRY_INTERVALS 100
#define SERVER_READY SIGUSR2
#define SERVER_BUSY SIGUSR1
#define CHAR_0 SIGUSR1
#define CHAR_1 SIGUSR2
#define USAGE "Usage: ./client <server_pid> <message>"
#define BAD_SIGNAL "Invalid server PID"

typedef enum e_parser_result
{
	PARSER_SUCCESS,
	PARSER_INVALID_ARGC,
	PARSER_INVALID_PID,
	PARSER_EMPTY_MESSAGE
} t_parser_result;

typedef struct s_client
{
	pid_t server_pid;
	pid_t client_pid;
	char *msg;
} t_client;

typedef struct s_server_state
{
	pid_t pid;
	int is_ready;
	int ready_to_proceed;
	int transmission_active;
	pid_t current_client_pid;
	int transmission_id;
	int last_sequence;
	int ack_count;
} t_server_state;

/* Singleton functions */
t_server_state *get_server_instance(void);
void reset_server_state(void);
void set_transmission_active(pid_t client_pid);
int is_transmission_owner(pid_t client_pid);
void end_transmission(void);

/* Parser functions */
t_parser_result parse_arguments(int argc, char **argv);
int validate_pid_string(const char *str);

/* Communication functions */
int ping(int pid);
void send_signals(void *data, size_t bit_length, t_client *info);
void send_message(char *str, t_client *data);
void wait_for_transmission_slot(t_client *data);
int connect_to_server(t_client *data);
void setup_signal_handlers(sigset_t *sigset, struct sigaction *sa);
void start_transmission(t_client *data, int msg_len);
int check_server_and_sleep(void);
int handle_timeouts(int pid);

/* Utility functions */
void init_data(char **argv, t_client *data);
void print_error_and_exit(t_parser_result result);
int ft_printf(const char *format, ...);
void log_ping_attempt(int attempt, int max_attempts);
void log_ping_result(int attempt, int success);
void setup_ping_signals(struct sigaction *sa, sigset_t *sigset);
void ping_handler(int signum, siginfo_t *info, void *context);
int calculate_checksum(const char *data, int length);

/* Time and retry functions */
int attempt_ping(int pid, int attempt);
void handle_retry_delay(int attempt);
int handle_timeout(int timeout_count, int max_timeout);

/* Initialization functions */
void setup_ping_signals(struct sigaction *sa, sigset_t *sigset);
void setup_signal_handlers(sigset_t *sigset, struct sigaction *sa);
void reset_server_state(void);
void prepare_transmission(t_client *data, int msg_len);

/* Connection and handshake functions */
void establish_connection(t_client *data);
void handle_acknowledgment(t_server_state *server);
void wait_for_server_ack(void);
void signal_handler(int signum, siginfo_t *info, void *context);

/* Debug and logging functions */
void log_ping_signal(int signum, pid_t sender_pid);

/* Validation functions */
void validate_and_init(int argc, char **argv, t_client *data);
int validate_signal_source(t_server_state *server,
						   pid_t sender_pid);
int validate_process_exists(int pid);
void check_transmission_ownership(pid_t my_pid, int total_chars,
								  int i);
int validate_ping_signal(t_server_state *server, siginfo_t *info);

/* Bit manipulation functions */
void send_signal(pid_t pid, int signal);
void send_data_bit(int bit_value, t_client *info);
void send_bit(unsigned long long value, int i, t_client *info);

/* Ping utility functions */
void handle_pong(int signum, t_server_state *server, pid_t pid);
int send_ping_signal(int pid);
#endif
