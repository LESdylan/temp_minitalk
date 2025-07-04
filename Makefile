# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/02 19:00:00 by dlesieur          #+#    #+#              #
#    Updated: 2025/07/04 12:35:57 by dlesieur         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME_SERVER = server
NAME_CLIENT = client
TEST_SIMPLE = test_simple
DEBUG_TRANSMISSION = debug_transmission
SYNC_TEST = sync_test
FIX_SYNC = fix_sync
FIX_ACK_SYSTEM = fix_ack_system
DEBUG_COMPLETION = debug_completion
DEBUG_BIT_SHIFT = debug_bit_shift

FLAG_DEBUG = 0
CC = cc
CFLAGS = -Wall -Wextra -Werror -I./inc -I./inc/libft
CFLAGS += -DMINITALK_DEBUG=$(FLAG_DEBUG)
RM = rm -f

# Directories
LIBFT_DIR = inc/libft
SERVER_DIR = srcs/server
CLIENT_DIR = srcs/client
AUTOMATA_DIR = srcs/automata

# Source files
SERVER_SRCS = $(SERVER_DIR)/main.c $(SERVER_DIR)/server.c $(SERVER_DIR)/signals.c $(SERVER_DIR)/server_loop.c $(SERVER_DIR)/server_message.c \
				$(SERVER_DIR)/crook.c $(SERVER_DIR)/debug.c $(SERVER_DIR)/inits.c $(SERVER_DIR)/time.c $(SERVER_DIR)/check.c

CLIENT_SRCS = 	$(CLIENT_DIR)/main.c $(CLIENT_DIR)/bits.c $(CLIENT_DIR)/ping.c $(CLIENT_DIR)/parser.c $(CLIENT_DIR)/singleton.c $(CLIENT_DIR)/ping_utils.c \
				$(CLIENT_DIR)/time.c $(CLIENT_DIR)/inits.c $(CLIENT_DIR)/hook.c $(CLIENT_DIR)/handshake.c $(CLIENT_DIR)/debug.c $(CLIENT_DIR)/checksum.c $(CLIENT_DIR)/check.c \
				$(CLIENT_DIR)/validate.c

AUTOMATA_SRCS = $(AUTOMATA_DIR)/singleton.c $(AUTOMATA_DIR)/log.c $(AUTOMATA_DIR)/format_state.c \
				$(AUTOMATA_DIR)/spec.c $(AUTOMATA_DIR)/conversion.c $(AUTOMATA_DIR)/bufferization.c \
				$(AUTOMATA_DIR)/verify.c $(AUTOMATA_DIR)/format_helper.c $(AUTOMATA_DIR)/action.c \
				$(AUTOMATA_DIR)/action_extra.c $(AUTOMATA_DIR)/char_table.c  $(AUTOMATA_DIR)/utils.c \
				$(AUTOMATA_DIR)/spec2.c $(AUTOMATA_DIR)/singleton_helper.c $(AUTOMATA_DIR)/action_more.c

# Object files
SERVER_OBJS = $(SERVER_SRCS:.c=.o) $(AUTOMATA_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o) $(AUTOMATA_SRCS:.c=.o)

# Default rule
all: $(NAME_SERVER) $(NAME_CLIENT)

# Build libft
$(LIBFT_DIR)/libft.a:
	$(MAKE) -C $(LIBFT_DIR)

# Build server
$(NAME_SERVER): $(SERVER_OBJS) $(LIBFT_DIR)/libft.a
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS) -L$(LIBFT_DIR) -lft

# Build client
$(NAME_CLIENT): $(CLIENT_OBJS) $(LIBFT_DIR)/libft.a
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS) -L$(LIBFT_DIR) -lft

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test targets
$(TEST_SIMPLE): test_simple.c
	$(CC) $(CFLAGS) -o $@ $<

$(DEBUG_TRANSMISSION): debug_transmission.c
	$(CC) $(CFLAGS) -o $@ $<

$(SYNC_TEST): sync_test.c
	$(CC) $(CFLAGS) -o $@ $<

$(FIX_SYNC): fix_sync.c
	$(CC) $(CFLAGS) -o $@ $<

$(FIX_ACK_SYSTEM): fix_ack_system.c
	$(CC) $(CFLAGS) -o $@ $<

$(DEBUG_COMPLETION): debug_completion.c
	$(CC) $(CFLAGS) -o $@ $<

$(DEBUG_BIT_SHIFT): debug_bit_shift.c
	$(CC) $(CFLAGS) -o $@ $<

test: $(TEST_SIMPLE)
	@echo "Running bit test..."
	./$(TEST_SIMPLE)

test_bit: $(TEST_SIMPLE)
	@echo "Testing bit patterns for values 3, 4, 5..."
	@echo "Value 3:"
	@sed 's/unsigned int value = 4/unsigned int value = 3/' test_simple.c > test_temp.c && $(CC) $(CFLAGS) -o test_temp test_temp.c && ./test_temp
	@echo "\nValue 4:"
	./$(TEST_SIMPLE)
	@echo "\nValue 5:"
	@sed 's/unsigned int value = 4/unsigned int value = 5/' test_simple.c > test_temp.c && $(CC) $(CFLAGS) -o test_temp test_temp.c && ./test_temp
	@rm -f test_temp test_temp.c

run_debug_transmission: $(DEBUG_TRANSMISSION)
	@echo "Running transmission debug analysis..."
	./$(DEBUG_TRANSMISSION)

run_sync_test: $(SYNC_TEST)
	@echo "Running synchronization test..."
	./$(SYNC_TEST)

run_fix_sync: $(FIX_SYNC)
	@echo "Running minitalk logic test..."
	./$(FIX_SYNC)

run_fix_ack: $(FIX_ACK_SYSTEM)
	@echo "Testing minitalk with simplified ACK system..."
	./$(FIX_ACK_SYSTEM)

run_debug_completion: $(DEBUG_COMPLETION)
	@echo "Testing completion signal delivery..."
	./$(DEBUG_COMPLETION)

run_debug_bit_shift: $(DEBUG_BIT_SHIFT)
	@echo "Testing bit shift issue..."
	./$(DEBUG_BIT_SHIFT)

# Clean test files
clean: 
	$(RM) $(SERVER_OBJS) $(CLIENT_OBJS) $(TEST_SIMPLE) $(DEBUG_TRANSMISSION) $(SYNC_TEST) $(FIX_SYNC) $(FIX_ACK_SYSTEM) $(DEBUG_COMPLETION) $(DEBUG_BIT_SHIFT) test_temp test_temp.c
	$(MAKE) -C $(LIBFT_DIR) clean

fclean: clean
	$(RM) $(NAME_SERVER) $(NAME_CLIENT)
	$(MAKE) -C $(LIBFT_DIR) fclean

re: fclean all

.PHONY: all clean fclean re test test_bit run_debug_transmission run_sync_test run_fix_sync run_fix_ack run_debug_completion run_debug_bit_shift speed_test