/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   auto_parser.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 18:31:55 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 18:57:33 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AUTO_PARSER_H
# define AUTO_PARSER_H
# include "libft.h"

// Note: Color definitions are provided by libft/render/ft_colors.h

typedef char			t_buffer[1024];

typedef enum s_log_level
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_DEBUG,
	LOG_SUCCESS
}	t_log_level;

# ifndef MINITALK_DEBUG
#  define MINITALK_DEBUG 0
# endif
# define CHAR_TABLE_SIZE    256
# define CHAR_DIGIT         0x01
# define CHAR_FLAG          0x02
# define CHAR_SPEC          0x04
# define CHAR_LENGTH        0x08
# define CHAR_DOT           0x10
# define CHAR_PERCENT       0x20
# define STATE_COUNT        8

typedef enum s_format_state
{
	STATE_NORMAL,
	STATE_PERCENT,
	STATE_FLAGS,
	STATE_WIDTH,
	STATE_PRECISION,
	STATE_LENGTH,
	STATE_SPECIFIER,
	STATE_ERROR
}	t_format_state;

typedef struct s_format_spec
{
	int		width;
	int		precision;
	int		left_align;
	int		show_sign;
	int		space_prefix;
	int		alt_form;
	int		zero_pad;
	int		has_precision;
	char	length_mod;
	char	specifier;
}			t_format_spec;

typedef struct s_parser_ctx
{
	t_format_state			state;
	t_format_spec			spec;
	const char				*fmt_ptr;
	int						temp_num;
	va_list					*args;
	t_buffer				buffer;
	int						*pos;
	struct s_parser_tables	*tables;
}							t_parser_ctx;

// Function pointer typedefs (must come after struct definitions)
typedef t_format_state	(*t_state_action)(t_parser_ctx *ctx, char c);
typedef void			(*t_flag_action)(t_format_spec *spec);
typedef void			(*t_spec_action)(t_parser_ctx *ctx);

typedef struct s_parser_tables
{
	t_flag			initialized;
	unsigned char	char_class_table[CHAR_TABLE_SIZE];
	t_state_action	state_transition_table[STATE_COUNT];
	t_flag_action	flag_table[CHAR_TABLE_SIZE];
	t_spec_action	spec_table[CHAR_TABLE_SIZE];
	const char		*level_strings[5];
}					t_parser_tables;

typedef struct s_format_data
{
	char			*buffer;
	int				*pos;
	long long		num;
	t_format_spec	*spec;
}	t_format_data;

// Function declarations
t_format_state	action_normal(t_parser_ctx *ctx, char c);
t_format_state	action_percent(t_parser_ctx *ctx, char c);
t_format_state	action_flags(t_parser_ctx *ctx, char c);
t_format_state	action_width(t_parser_ctx *ctx, char c);
t_format_state	action_precision(t_parser_ctx *ctx, char c);
t_format_state	action_length(t_parser_ctx *ctx, char c);
t_format_state	action_specifier(t_parser_ctx *ctx, char c);
t_format_state	action_error(t_parser_ctx *ctx, char c);
t_parser_tables	*get_parser_tables(void);

void			flag_left_align(t_format_spec *spec);
void			flag_show_sign(t_format_spec *spec);
void			flag_space_prefix(t_format_spec *spec);
void			flag_alt_form(t_format_spec *spec);
void			flag_zero_pad(t_format_spec *spec);

void			spec_string(t_parser_ctx *ctx);
void			spec_decimal(t_parser_ctx *ctx);
void			spec_integer(t_parser_ctx *ctx);
void			spec_char(t_parser_ctx *ctx);
void			spec_hex_lower(t_parser_ctx *ctx);
void			spec_hex_upper(t_parser_ctx *ctx);
void			spec_octal(t_parser_ctx *ctx);
void			spec_unsigned(t_parser_ctx *ctx);
void			spec_float(t_parser_ctx *ctx);
void			spec_pointer(t_parser_ctx *ctx);

void			buffer_append_char(t_buffer buffer, int *pos, char c);
void			buffer_append_str(t_buffer buffer, int *pos, const char *str);
void			buffer_append_str_formatted(t_buffer buffer, int *pos,
					const char *str, t_format_spec *spec);
void			buffer_append_int_formatted(t_buffer buffer, int *pos,
					long long num, t_format_spec *spec);
void			put_level_buffered(t_buffer buffer, int *pos,
					t_log_level level);
void			log_msg(t_log_level level, const char *fmt, ...);
void			vlog_msg_automata(const char *fmt, va_list args,
					t_buffer buffer, int *pos);					

// Additional function declarations for new files
int				verify_buffer_integrity(t_buffer buffer, int pos);
int				safe_buffer_append(t_buffer buffer, int *pos,
					const char *str, int len);
int				convert_number_to_string(char *dest, long num,
					int base, int uppercase);
int				format_with_precision(char *dest, const char *src,
					t_format_spec *spec);
int				convert_to_string(long num, char *temp,
					int base, int uppercase);
void			init_class(t_parser_tables *tables);
void			init_char_table(t_parser_tables *tables);

#endif
