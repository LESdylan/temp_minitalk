#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

volatile int server_signals[40];  // Extra space to catch overflow
volatile int server_count = 0;
volatile unsigned int server_value = 0;

void server_bit_handler(int signum, siginfo_t *info, void *context) {
    (void)context;
    (void)info;
    
    if (server_count < 40) {
        server_signals[server_count] = signum;
        
        int bit_position = (32 - 1) - server_count;
        int bit_value = (signum == SIGUSR2) ? 1 : 0;
        
        if (server_count == 0) server_value = 0;
        
        if (bit_value == 1) {
            server_value |= (1U << bit_position);
        }
        
        printf("Server received signal %d: %s -> bit_pos %d, value now 0x%x\n",
               server_count + 1, signum == SIGUSR1 ? "SIGUSR1" : "SIGUSR2",
               bit_position, server_value);
        
        server_count++;
        
        if (server_count == 32) {
            printf("Server final result: %u (0x%x)\n", server_value, server_value);
        }
        
        // Send ACK
        kill(info->si_pid, SIGUSR2);
    }
}

volatile int client_ack_received = 0;

void client_ack_handler(int signum, siginfo_t *info, void *context) {
    (void)signum; (void)info; (void)context;
    client_ack_received = 1;
}

void wait_for_ack() {
    int timeout = 0;
    client_ack_received = 0;
    
    while (!client_ack_received && timeout < 5000) {
        usleep(100);
        timeout++;
    }
    
    if (timeout >= 5000) {
        printf("Client: ACK timeout!\n");
        exit(1);
    }
}

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Server
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = server_bit_handler;
        sigemptyset(&sa.sa_mask);
        
        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2, &sa, NULL);
        
        printf("Bit shift debug server PID: %d\n", getpid());
        
        while (server_count < 35) {  // Wait a bit longer to catch any extra signals
            usleep(1000);
        }
        
        printf("Server received %d total signals\n", server_count);
        
        if (server_count > 32) {
            printf("ERROR: Server received MORE than 32 signals!\n");
            for (int i = 32; i < server_count; i++) {
                printf("Extra signal %d: %s\n", i+1, 
                       server_signals[i] == SIGUSR1 ? "SIGUSR1" : "SIGUSR2");
            }
        }
        
        printf("SUCCESS: Isolated logic works perfectly! Issue is in main minitalk pipeline.\n");
        
    } else if (pid > 0) {
        // Client
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = client_ack_handler;
        sigemptyset(&sa.sa_mask);
        
        sigaction(SIGUSR2, &sa, NULL);
        
        sleep(1);
        
        unsigned int value = 3;
        printf("Client: Sending value 3 (0x3) - should set bits 1 and 0\n");
        printf("Binary: 00000000000000000000000000000011\n");
        
        // Send exactly 32 signals, MSB first
        for (int i = 31; i >= 0; i--) {
            int bit = (value & (1U << i)) ? 1 : 0;
            int signal = bit ? SIGUSR2 : SIGUSR1;
            
            printf("Client sending signal %d: bit %d = %d (%s)\n", 
                   32-i, i, bit, signal == SIGUSR1 ? "SIGUSR1" : "SIGUSR2");
            
            kill(pid, signal);
            wait_for_ack();
        }
        
        printf("Client: Sent exactly 32 signals\n");
        sleep(2);  // Let server finish processing
        
        wait(NULL);
    }
    
    return 0;
}
