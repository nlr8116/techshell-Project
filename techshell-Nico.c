#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

// Function to display the prompt
void display_prompt() {
    char cwd[MAX_COMMAND_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s$ ", cwd);
    } else {
        perror("getcwd");
        printf("$ "); 
    }
}

// Function to parse the command line into arguments and handle redirection
int parse_command(char* cmd_line, char* args[], char** input_file, char** output_file) {
    int arg_count = 0;
    *input_file = NULL;
    *output_file = NULL;

    char* token = strtok(cmd_line, " \n\t"); 
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        if (strcmp(token, "<") == 0) { 
            token = strtok(NULL, " \n\t");
            *input_file = token;
        } else if (strcmp(token, ">") == 0) { 
            token = strtok(NULL, " \n\t");
            *output_file = token;
        } else {
            args[arg_count++] = token;
        }
        token = strtok(NULL, " \n\t");
    }
    args[arg_count] = NULL; 

    return arg_count;
}

// Function to execute the command using fork and execvp
void execute_command(char* args[], char* input_file, char* output_file) {
    pid_t pid = fork(); 

    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) {
        // Child process
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno)); 
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        
        if (output_file != NULL) {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
            if (fd_out < 0) {
                fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(args[0], args); 
        fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno)); 
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        waitpid(pid, NULL, 0); 
    }
}

int main() {
    char command_line[MAX_COMMAND_LENGTH];
    char* args[MAX_ARGS];
    char* input_file;
    char* output_file;

    // Main loop of the shell
    while (1) {
        display_prompt();
        
        if (fgets(command_line, MAX_COMMAND_LENGTH, stdin) == NULL) {
            break;
        }

	// Remove newline character
        command_line[strcspn(command_line, "\n")] = '\0';

        if (strlen(command_line) == 0) continue; 

	// Check for exit command
        if (strcmp(command_line, "exit") == 0) {
            break; 
        }

        // Parse the command line
        int arg_count = parse_command(command_line, args, &input_file, &output_file);
	// check for cd command
        if (arg_count > 0 && strcmp(args[0], "cd") == 0) {
            const char* path = (arg_count > 1) ? args[1] : getenv("HOME");
            if (chdir(path) != 0) {
                fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
            }
        } else if (arg_count > 0) {
	    // Execute commands
            execute_command(args, input_file, output_file);
        }
    }

    return 0;
}

