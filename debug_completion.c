#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

volatile int completion_received = 0;
volatile int signal_count = 0;

void completion_handler(int signum, siginfo_t *info, void *context) {
    (void)context;
    
    signal_count++;
    printf("DEBUG: Received signal %d: %s from PID %d (count: %d)\n", 
           signal_count, signum == SIGUSR1 ? "SIGUSR1" : "SIGUSR2", 
           info->si_pid, signal_count);
           
    if (signum == SIGUSR1) {
        completion_received = 1;
        printf("DEBUG: COMPLETION SIGNAL RECEIVED!\n");
    }
}

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child - simulate server sending completion
        sleep(1);
        
        printf("Server: Sending 10 completion signals to parent\n");
        
        for (int i = 0; i < 10; i++) {
            printf("Server: Sending completion signal %d/10\n", i+1);
            
            if (kill(getppid(), SIGUSR1) == -1) {
                perror("kill");
                break;
            }
            
            usleep(5000); // 5ms delay
        }
        
        printf("Server: All completion signals sent\n");
        exit(0);
        
    } else if (pid > 0) {
        // Parent - simulate client waiting
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = completion_handler;
        sigemptyset(&sa.sa_mask);
        
        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2, &sa, NULL);
        
        printf("Client: Waiting for completion signals...\n");
        
        int timeout = 0;
        while (!completion_received && timeout < 60) {
            sleep(1);
            timeout++;
            
            if (timeout % 5 == 0) {
                printf("Client: Still waiting... (%d seconds, %d signals received)\n", 
                       timeout, signal_count);
            }
        }
        
        if (completion_received) {
            printf("SUCCESS: Completion signal received after %d seconds!\n", timeout);
        } else {
            printf("FAILURE: No completion signal received after %d seconds\n", timeout);
        }
        
        wait(NULL);
    }
    
    return 0;
}
            printf("SUCCESS: Completion signal received after %d seconds!\n", timeout);
        }
        else
        {
            printf("FAILURE: No completion signal received after %d seconds\n", timeout);
        }

        wait(NULL);
    }

    return 0;
}
