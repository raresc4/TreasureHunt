#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

typedef struct {
    int x;
    int y;
} Coordonate;

typedef struct {
    int id;
    char username[50];
    Coordonate coordonate;
    char clue[100];
    int value;
} Treasure;

int monitor_running = 0;
int monitor_pid;
volatile sig_atomic_t child_exited = 0;
int monitor_status = 0;
int monitor_stopping = 0; 

void handle_sigchld(int sig) {
    int pid;
    int status;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == monitor_pid) {
            child_exited = 1;
            monitor_status = status;
            monitor_running = 0;
            monitor_stopping = 0;  
            printf("[Hub] Monitor process ended with status %d\n", 
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1);
        }
    }
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error setting up SIGCHLD handler");
        exit(EXIT_FAILURE);
    }
    
    while(1) {
        char command[100];
        printf("treasure_hub>> ");
        scanf("%s", command);
        command[strcspn(command, "\n")] = 0;
        
        if(monitor_stopping) {
            printf("Error: Waiting for monitor to stop. Please wait...\n");
            continue;
        }
        
        if(strcmp(command, "start_monitor") == 0) {
            if(monitor_running) {
                printf("Monitor already running.\n");
            } else {
                
                monitor_pid = fork();
                
                if (monitor_pid < 0) {
                    perror("Unable to fork process");
                    exit(EXIT_FAILURE);
                }
                
                if (monitor_pid == 0) {
                    struct sigaction monitor_sa;
                    memset(&monitor_sa, 0, sizeof(monitor_sa));   
                    sigemptyset(&monitor_sa.sa_mask);
                    monitor_sa.sa_flags = 0;
                    if(sigaction(SIGUSR1, &monitor_sa, NULL) == -1) {
                        perror("[Monitor] Error setting up SIGUSR1 handler");
                        exit(EXIT_FAILURE);
                    }
                    
                    void handle_sigterm(int sig) {
                        printf("[Monitor] Cleaning up...\n");
                        usleep(2000000);  
                        printf("[Monitor] Terminating.\n");
                        exit(EXIT_SUCCESS);
                    }
                    
                    monitor_sa.sa_handler = handle_sigterm;
                    if(sigaction(SIGTERM, &monitor_sa, NULL) == -1) {
                        perror("[Monitor] Error setting up SIGTERM handler");
                        exit(EXIT_FAILURE);
                    }
                    
                    printf("[Monitor] Started with PID %d\n", getpid());
                    
                    while(1) {
                        pause();
                    }
                    
                    exit(EXIT_SUCCESS);
                }
                else {
                    monitor_running = 1;
                    printf("[Hub] Monitor started with PID %d\n", monitor_pid);
                }
            }
        }
        else if(strcmp(command, "stop_monitor") == 0) {
            if(!monitor_running) {
                printf("Error: Monitor not running.\n");
            } else {
                if(kill(monitor_pid, SIGTERM) == -1) {
                    perror("Error sending termination signal to monitor");
                } else {
                    printf("[Hub] Stop signal sent to monitor. Waiting for termination...\n");
                    monitor_stopping = 1;  
                }
            }
        }
        else if(strcmp(command, "exit") == 0) {
            if(monitor_running) {
                printf("Error: Monitor still running. Stop the monitor first.\n");
            } else {
                printf("Exiting treasure_hub...\n");
                exit(EXIT_SUCCESS);
            }
        }
        else {
            printf("Unknown command: %s\n", command);
        }
        
        if (child_exited) {
            printf("[Hub] Monitor process terminated with status: %d\n", 
                   WIFEXITED(monitor_status) ? WEXITSTATUS(monitor_status) : -1);
            child_exited = 0;
        }
    }
    
    return 0;
}
