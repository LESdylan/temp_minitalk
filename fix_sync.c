#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

// Mimic the exact structure of your minitalk
volatile int received_signals[32];
volatile int signal_count = 0;
volatile unsigned int message_size = 0;

// Exact copy of your server's get_bit_value function
int get_bit_value(int signum)
{
    if (signum == SIGUSR1)
        return 0; // SIGUSR1 = 0 bit
    return 1;     // SIGUSR2 = 1 bit
}

// Exact copy of your server's process_header_bit function
void process_header_bit(int bit_value)
{
    int bit_position;

    // Client sends MSB first: bit 31, 30, 29, ..., 1, 0
    bit_position = (32 - 1) - signal_count;

    // Initialize to 0 on first bit
    if (signal_count == 0)
        message_size = 0;

    // Only set the bit if bit_value is 1
    if (bit_value == 1)
        message_size |= (1U << bit_position);

    signal_count++;

    // Debug output for last 8 bits
    if (signal_count > 24)
    {
        printf("Minitalk: Bit %d: %d (pos %d, value: 0x%x)\n",
               signal_count, bit_value, bit_position, message_size);
    }
}

void minitalk_signal_handler(int signum, siginfo_t *info, void *context)
{
    (void)context;
    (void)info;

    if (signal_count < 32)
    {
        received_signals[signal_count] = signum;

        // Process using minitalk logic
        int bit_value = get_bit_value(signum);
        process_header_bit(bit_value);

        printf("Minitalk received signal %d: %s (count: %d)\n",
               signal_count, signum == SIGUSR1 ? "SIGUSR1" : "SIGUSR2", signal_count);
    }
}

int main()
{
    pid_t pid = fork();

    if (pid == 0)
    {
        // Child - minitalk server simulation
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = minitalk_signal_handler;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2, &sa, NULL);

        printf("Minitalk test server PID: %d\n", getpid());

        // Wait for signals
        while (signal_count < 32)
        {
            usleep(1000);
        }

        printf("\nMinitalk result: %u (0x%x)\n", message_size, message_size);
        printf("Expected: 3 (0x3)\n");
        printf("Match: %s\n", (message_size == 3) ? "YES" : "NO");

        if (message_size != 3)
        {
            printf("ERROR: Minitalk logic has a bug!\n");
            printf("Received bits: ");
            for (int i = 31; i >= 0; i--)
            {
                if (message_size & (1U << i))
                {
                    printf("bit_%d ", i);
                }
            }
            printf("\n");
        }
    }
    else if (pid > 0)
    {
        // Parent - client
        sleep(1); // Let server setup

        unsigned int value = 3; // Test value
        printf("Minitalk test: Sending value 3\n");

        // Send MSB first (exactly like minitalk client)
        for (int i = 31; i >= 0; i--)
        {
            int bit = (value & (1U << i)) ? 1 : 0;
            int signal = bit ? SIGUSR2 : SIGUSR1;

            if (kill(pid, signal) == -1)
            {
                perror("kill");
                break;
            }

            if (i <= 7)
            {
                printf("Minitalk sent bit %d: %d (%s)\n", i, bit,
                       signal == SIGUSR1 ? "SIGUSR1" : "SIGUSR2");
            }

            usleep(5000); // 5ms delay between signals
        }

        wait(NULL);
    }

    return 0;
}
