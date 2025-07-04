#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

volatile int received_signals[32];
volatile int signal_count = 0;

void test_signal_handler(int signum, siginfo_t *info, void *context)
{
    (void)context;
    (void)info;

    if (signal_count < 32)
    {
        received_signals[signal_count] = signum;
        signal_count++;
        printf("Received signal %d: %s (count: %d)\n",
               signal_count, signum == SIGUSR1 ? "SIGUSR1" : "SIGUSR2", signal_count);
    }
}

int main()
{
    pid_t pid = fork();

    if (pid == 0)
    {
        // Child - server
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = test_signal_handler;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2, &sa, NULL);

        printf("Test server PID: %d\n", getpid());

        // Wait for signals
        while (signal_count < 32)
        {
            usleep(1000);
        }

        // Reconstruct the value
        unsigned int reconstructed = 0;
        printf("\nSignal reception analysis:\n");
        for (int i = 0; i < 32; i++)
        {
            int bit_pos = 31 - i;
            int bit_value = (received_signals[i] == SIGUSR2) ? 1 : 0;
            if (bit_value)
            {
                reconstructed |= (1U << bit_pos);
            }
            if (i >= 24)
            {
                printf("Signal %d: %s -> bit_pos %d, value: 0x%x\n",
                       i + 1, received_signals[i] == SIGUSR1 ? "SIGUSR1" : "SIGUSR2",
                       bit_pos, reconstructed);
            }
        }

        printf("Final reconstructed: %u (0x%x)\n", reconstructed, reconstructed);
        printf("Expected: 3 (0x3)\n");
    }
    else if (pid > 0)
    {
        // Parent - client
        sleep(1); // Let server setup

        unsigned int value = 3; // Test value
        printf("Sending value 3 (binary: 00000000000000000000000000000011)\n");

        // Send MSB first
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
                printf("Sent bit %d: %d (%s)\n", i, bit,
                       signal == SIGUSR1 ? "SIGUSR1" : "SIGUSR2");
            }

            usleep(5000); // 5ms delay between signals
        }

        wait(NULL);
    }

    return 0;
}
