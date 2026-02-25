// Names: Landon Carderara and Nico Relle
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

// a struct for the command 
struct Command{
    char **args;        // array of string for execvp
    char *inputFile;    // file name for <
    char *outputFile;   // file name for >
    int redirectIn;     // 1 if < else 0
    int redirectOut;    // 1 if > else 0
};

// Functional Prototype
void display(void);
void input(char *buf, size_t size);
struct Command ParseInput (char *input);
void executeCommand(struct Command cmd);
void freeCommand(struct Command *cmd);

int main(void){
    char cmd_line[1024];

    // infinite loop only breaks when exit is input
    while(1){
        display();
        input(cmd_line, sizeof(cmd_line));
        // if enter is hit skips to the next iteration of the loop
        if (cmd_line[0] == '\0'){
            continue;
        } 

        struct Command cmd = ParseInput(cmd_line);

        // if exit is input frees the struct then breaks the loop
        if (cmd.args[0] != NULL && strcmp(cmd.args[0], "exit") == 0){
            freeCommand(&cmd);
            break;
        }

        // executes the code then free the struct
        executeCommand(cmd);
        freeCommand(&cmd);

    }
    return 0;
}

// function to display the current path
void display(){
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL){
        printf("%s$ ", cwd);
    }
    else{
        perror("cwd error");
    }

    
}

// function to get the input from the user
void input(char *buf, size_t size) {
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0';
}

// this function is used to parse through the users input 
// it detects the redirect input and output flags and updates them in the struct
// it add the regular commands/arguments to the args array to be used in the execvp function
// it returns the Command struct with the needed information such as an array of arguments
// input and output file if needed along with change the value of redirectin or out if needed
struct Command ParseInput (char *input){
    // initializes the command struct along with creating the variables
    struct Command cmd;
    cmd.args = malloc(64*sizeof(char*));
    cmd.inputFile = NULL;
    cmd.outputFile = NULL;
    cmd.redirectIn = 0;
    cmd.redirectOut= 0; 

    int argCount = 0;
    int i = 0;
    char buffer[1024]; // temp spot for the words
    int b = 0;         //index for buffer
    int inQuotes = 0;

    // loop that runs until a new line or end of line
    while (input[i] != '\0' && input[i] !='\n'){
        
        // skips leading spaces
        if (!inQuotes && (input[i] == ' ' || input[i] == '\t')){
            i++;
            continue;
        }

        // handle redirection sybmbols
        if (!inQuotes && (input[i] == '<' || input[i] == '>')){
            char type = input[i++];

            // goes throught the spaces
            while (input[i] == ' ' || input[i] == '\t'){
                i++;
            }

            b = 0;

            // grabs the file name
            // checks for escape characters 
            while (input[i] != '\0' && (inQuotes || (!isspace(input[i]) && input[i] != '<' && input[i] != '>'))) {
                if (input[i] == '\\') { 
                    i++; 
                    if (input[i] != '\0'){
                        buffer[b++] = input[i++];
                    }
                }

                else if (input[i] == '\"'){
                    inQuotes = !inQuotes;
                    i++;
                }
                else {
                    buffer[b++] = input[i++];
                }
            }

            buffer[b] = '\0';

            // adds the file name to the correct variable
            if (type == '<'){
                cmd.redirectIn = 1;
                cmd.inputFile = strdup(buffer);
            }

            else{
                cmd.redirectOut = 1;
                cmd.outputFile = strdup(buffer);
            }
            continue;
        }

        b = 0;
        // adds agruments with no redirection symbol to the buffer
        while (input[i] != '\0'){
            if (input[i] == '\\'){
                i++;
                if (input[i] != '\0'){
                    buffer[b++] = input[i++];
                }
            }
            else if (input[i] == '\"'){
                inQuotes = !inQuotes;
                i++;
            }
            else if (!inQuotes && (input[i] == ' ' || input[i] == '\t' || input[i] == '<' || input[i] == '>')){
                break;
            }
            else{
                buffer[b++] = input[i++];
            }
        }

        // adds the buffer to the args array
        buffer[b] = '\0';
        if (b > 0){
            cmd.args[argCount++] = strdup(buffer);
        }
    }

    // adds NULL to the end of args so it can be used with execvp
    cmd.args[argCount] = NULL;
    return cmd;
}

// function to execute the command and redirect the input and output
void executeCommand(struct Command cmd){
    // handle cd 
    if (cmd.args[0] != NULL && strcmp(cmd.args[0], "cd") == 0){
        // check whether cd is by itself or if a directory is with it 
        char *dir = cmd.args[1] ? cmd.args[1] : getenv("HOME");
        if (chdir(dir) != 0){
            // error
            printf("Error %d (%s)\n", errno, strerror(errno));
        }
        return;
    }

    // fork process
    pid_t pid = fork();

    if (pid < 0){
        printf("Error %d (%s)\n", errno, strerror(errno));
    }
    else if (pid == 0){
        // handle input redirection
        if (cmd.redirectIn && cmd.inputFile != NULL){
            int fd = open(cmd.inputFile, O_RDONLY);
            if (fd < 0){
                printf("Error %d (%s)\n", errno, strerror(errno));
                exit(1);
            }
            dup2(fd, STDIN_FILENO); // redirect the stdin to the new input file
            close(fd);
        }

        // handle output redirect
        if (cmd.redirectOut && cmd.outputFile != NULL){
            int fd = open(cmd.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0){
                printf("Error %d (%s)\n", errno, strerror(errno));
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // redirect stdout to the new file
            close(fd);
        }

        // execute commands
        if (execvp(cmd.args[0], cmd.args) == -1){
            printf("Error %d (%s)\n", errno, strerror(errno));
            exit(1);
        }
    }
    else{
        wait(NULL);
    }
}

void freeCommand(struct Command *cmd){
    // free each individual argument
    if (cmd ->args != NULL){
        for (int i = 0; cmd->args[i]; i++){
            free(cmd->args[i]);
        }

        // free the array of pointerss
        free(cmd->args);
    }
    // free redirect filenames
    if (cmd->inputFile != NULL){
        free(cmd->inputFile);
    }

    if (cmd ->outputFile != NULL){
        free(cmd->outputFile);
    }

}
