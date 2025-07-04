#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

// Mimic exact minitalk but with minimal ACK system
volatile int received_signals[32];
volatile int signal_count = 0;
volatile unsigned int message_size = 0;
volatile int ack_ready = 0;

int get_bit_value(int signum)
{
    return (signum == SIGUSR2) ? 1 : 0;
}

void process_header_bit(int bit_value)
{
    int bit_position = (32 - 1) - signal_count;

    if (signal_count == 0)
        message_size = 0;

    if (bit_value == 1)
        message_size |= (1U << bit_position);

    signal_count++;

    if (signal_count > 24)
    {
        printf("Server: Bit %d: %d (pos %d, value: 0x%x)\n",
               signal_count, bit_value, bit_position, message_size);
    }
}

void server_signal_handler(int signum, siginfo_t *info, void *context)
{
    (void)context;

    if (signal_count < 32)
    {
        int bit_value = get_bit_value(signum);
        process_header_bit(bit_value);

        // Send immediate ACK
        kill(info->si_pid, SIGUSR2);
    }
}

void client_ack_handler(int signum, siginfo_t *info, void *context)
{
    (void)signum;
    (void)info;
    (void)context;
    ack_ready = 1;
}

void wait_for_ack()
{
    int timeout = 0;
    ack_ready = 0;

    while (!ack_ready && timeout < 10000)
    {
        usleep(100);
        timeout++;
    }

    if (timeout >= 10000)
    {
        printf("Client: ACK timeout!\n");
        exit(1);
    }
}

int main()
{
    pid_t pid = fork();

    if (pid == 0)
    {
        // Server
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = server_signal_handler;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2, &sa, NULL);

        printf("Fixed ACK server PID: %d\n", getpid());

        while (signal_count < 32)
        {
            usleep(1000);
        }

        printf("Fixed ACK result: %u (0x%x)\n", message_size, message_size);
        printf("Expected: 3 (0x3)\n");
        printf("Match: %s\n", (message_size == 3) ? "YES" : "NO");
    }
    else if (pid > 0)
    {
        // Client
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = client_ack_handler;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGUSR2, &sa, NULL);

        sleep(1); // Let server setup

        unsigned int value = 3;
        printf("Fixed ACK test: Sending value 3 with ACK system\n");

        // Send with ACK (like real minitalk)
        for (int i = 31; i >= 0; i--)
        {
            int bit = (value & (1U << i)) ? 1 : 0;
            int signal = bit ? SIGUSR2 : SIGUSR1;

            kill(pid, signal);
            wait_for_ack(); // Wait for ACK like real minitalk

            if (i <= 7)
            {
                printf("Client: Sent bit %d: %d, got ACK\n", i, bit);
            }
        }

        wait(NULL);
    }

    return 0;
}
