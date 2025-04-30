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

#define COMMAND_FILE "monitor_command.txt"

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

void write_command(const char* cmd, const char* param1, const char* param2) {
    FILE* cmd_file = fopen(COMMAND_FILE, "w");
    if (!cmd_file) {
        perror("Error opening command file");
        return;
    }
    
    if (param1 && param2) {
        fprintf(cmd_file, "%s %s %s", cmd, param1, param2);
    } else if (param1) {
        fprintf(cmd_file, "%s %s", cmd, param1);
    } else {
        fprintf(cmd_file, "%s", cmd);
    }
    
    fclose(cmd_file);
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
                unlink(COMMAND_FILE);
                monitor_pid = fork();
                
                if (monitor_pid < 0) {
                    perror("Unable to fork process");
                    exit(EXIT_FAILURE);
                }
                
                if (monitor_pid == 0) {
                    struct sigaction monitor_sa;
                    memset(&monitor_sa, 0, sizeof(monitor_sa));
                    void handle_sigusr1(int sig) {
                        FILE* cmd_file = fopen(COMMAND_FILE, "r");
                        if (!cmd_file) {
                            perror("[Monitor] Error opening command file");
                            return;
                        }
                        
                        char cmd[100];
                        char param1[50] = "";
                        char param2[50] = "";
                        
                        fscanf(cmd_file, "%s %s %s", cmd, param1, param2);
                        fclose(cmd_file);
                        
                        if (strcmp(cmd, "list_hunts") == 0) {
                            printf("[Monitor] Listing all hunts and their treasure counts:\n");
                            
                            DIR* dir = opendir(".");
                            if (!dir) {
                                perror("[Monitor] Error opening current directory");
                                return;
                            }
                            
                            struct dirent* entry;
                            int hunt_count = 0;
                            
                            while ((entry = readdir(dir)) != NULL) {
                                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                                    continue;
                                }
                                
                                DIR* hunt_dir = opendir(entry->d_name);
                                if (hunt_dir) {
                                    char treasure_path[256];
                                    sprintf(treasure_path, "%s/treasures", entry->d_name);
                                    
                                    int pid = fork();
                                    if (pid == 0) {
                                        int dev_null = open("/dev/null", O_WRONLY);
                                        dup2(dev_null, STDOUT_FILENO);
                                        close(dev_null);
                                        
                                        execlp("./p", "p", "list", entry->d_name, NULL);
                                        
                                        perror("[Monitor] Error executing p list command");
                                        exit(EXIT_FAILURE);
                                    } else if (pid > 0) {
                                        int status;
                                        waitpid(pid, &status, 0);
                                        
                                        FILE* treasure_file = fopen(treasure_path, "r");
                                        if (treasure_file) {
                                            fseek(treasure_file, 0, SEEK_END);
                                            long file_size = ftell(treasure_file);
                                            int treasure_count = file_size / sizeof(Treasure);
                                            
                                            printf("Hunt: %s, Treasures: %d\n", entry->d_name, treasure_count);
                                            hunt_count++;
                                            
                                            fclose(treasure_file);
                                        }
                                    }
                                    
                                    closedir(hunt_dir);
                                }
                            }
                            
                            closedir(dir);
                            printf("[Monitor] Total hunts found: %d\n", hunt_count);
                        } 
                        else if (strcmp(cmd, "list_treasures") == 0) {
                            if (param1[0] != '\0') {
                                printf("[Monitor] Listing treasures for hunt: %s\n", param1);
                                
                                int pid = fork();
                                if (pid == 0) {
                                    execlp("./p", "p", "list", param1, NULL);
                                    
                                    perror("[Monitor] Error executing p list command");
                                    exit(EXIT_FAILURE);
                                } else if (pid > 0) {
                                    int status;
                                    waitpid(pid, &status, 0);
                                }
                            } else {
                                printf("[Monitor] Error: Hunt name not provided\n");
                            }
                        }
                    }
                    monitor_sa.sa_handler = handle_sigusr1;   
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
        else if(strcmp(command, "list_hunts") == 0) {
            if(!monitor_running) {
                printf("Error: Monitor not running.\n");
            } else {
                write_command("list_hunts", NULL, NULL);
                
                if(kill(monitor_pid, SIGUSR1) == -1) {
                    perror("Error sending signal to monitor");
                } else {
                    printf("[Hub] Requested hunt listing from monitor.\n");
                }
            }
        }
        else if(strcmp(command, "list_treasures") == 0) {
            if(!monitor_running) {
                printf("Error: Monitor not running.\n");
            } else {
                char hunt_name[50];
                printf("Enter hunt name: ");
                scanf("%s", hunt_name);
                
                write_command("list_treasures", hunt_name, NULL);
                
                if(kill(monitor_pid, SIGUSR1) == -1) {
                    perror("Error sending signal to monitor");
                } else {
                    printf("[Hub] Requested treasure listing for hunt %s from monitor.\n", hunt_name);
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
