#include <stdio.h>  // printf
#include <assert.h> //assert
#include <unistd.h> // pipe, fork, dup2, close, read, exec
#include <string.h>   // strtok
#include <fcntl.h>
#include <stdbool.h>
//#include <wait.h>

void readInput(char *commands[], int* first_command_length, char *secondCommand[], int *second_command_length)
{
    char user_input[80]; 
    const char s[1] = " ";
    
    int length = read(STDIN_FILENO, user_input, 80);

    
    if (user_input[length - 1] == '\n')
    {
        user_input[length - 1] = '\0';
    }
    

    if (strcmp(user_input, "!!") == 0)
    {
        if (*first_command_length == 0) 
        {
            printf("No commands in history.\n");
        }
        return;
    }

    // Make sure previous commands should be cleared out
    int i = 0;
    while(commands[i] != NULL && (i < *first_command_length))
    {
        free(commands[i]);
        i++;
        if(i == 80)
            break;
    }
    i=0;
    while(secondCommand[i] != NULL && (i < *second_command_length))
    {
        free(secondCommand[i]);
        i++;
        if(i == 80)
            break;
    }
    *first_command_length = 0;
    *second_command_length = 0;

    // Take the first command from the user input
    char *temp = strtok(user_input, s);
    
    
    while( temp != NULL)
    {
        if (temp[0] == '&')
        {
            temp = strtok(NULL, s);
            *second_command_length+=1;
            secondCommand[*second_command_length-1] = strdup(temp);
            secondCommand[*second_command_length] = NULL;
            commands[*first_command_length] = NULL;
            return;
        }
        *first_command_length+=1;
        commands[*first_command_length-1] = strdup(temp);
        temp = strtok(NULL, s);
    }
    commands[*first_command_length] = NULL;
    
}

int main(void)
{
    char *args[80 / 2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */
    pid_t pid = 0;
    int command_count = 0;
    bool used_pipe;
    char* secondCommand[80 / 2 + 1];
    int hasSecond = 0;
    while(should_run)
    {
        used_pipe = false;  // reset         
        printf("osh>");
        fflush(stdout);

        // read the user's input 
        readInput(args, &command_count, secondCommand, &hasSecond);   

        pid_t pid = fork();
        if(pid == 0)
        {
            if(command_count == 0)
            {
                continue;
            }
            else
            {
                int redirect = 0;
                int file = 0;

                for (int i = 1; i <= command_count-1; i++)
                {
                    if (strcmp(args[i], ">") == 0)
                    {
                        // It gives all the permission with 0666.
                        // It creates a new file and write the targeting file
                        // to the new file. 
                        file = open(args[i+1], O_WRONLY | O_CREAT, 0666); 
                        if(file == -1)
                        {
                            printf("There is an issue on this file");
                            exit(1);
                        }
                        dup2(file, 1); // output
                        args[i] = NULL;
                        args[i+1] = NULL;
                        redirect = 2;
                        break;
                    }
                    else if (strcmp(args[i], "<") == 0)
                    {
                        file = open(args[i+1], O_RDONLY); // for read only
                        if(file == -1)
                        {
                            printf("There is an issue on this file");
                            exit(1);
                        }
                        dup2(file, 0); // input
                        args[i] = NULL;
                        args[i+1] = NULL;
                        redirect = 1;
                        break;
                    }                    
                    else if (strcmp(args[i], "|") == 0)
                    {

                        // These below chars will have one additonal 
                        //      length to store NULL at the end of the array.
                        char* firstCommand [i-1]; // Former command 
                        char* secondCommand [command_count-i]; // Latter one

                        // Store the command in args to firstCommand
                        for(int j=0; j<i; j++)
                        {
                            firstCommand[j] = args[j];
                        }
                        firstCommand[i] = NULL;

                        // Ditto to secondCommand
                        for(int j=1; j<command_count-i; j++)
                        {
                            secondCommand[j] = args[i+j];
                        }
                        secondCommand[command_count-i] = NULL;

                        // Set Pipe 
                        int fd[2];
                        if(pipe(fd) < 0)
                        {
                            printf("pipe error");
                            exit(1);
                        }
                        used_pipe = true;
                        int pid2 = fork();

                        if(pid2 == 0)
                        {
                            // Child process
                            close(fd[0]); // not gonna read
                            dup2[fd[1], 1];
                            if(execvp(firstCommand[0], firstCommand) == -1)
                            {
                                printf("pipe failed due to a first command");
                                exit(1);
                            }
                            close(fd[1]);
                        }
                        else
                        {
                            // Parent proces
                            wait(NULL);    // wait until child writes 
                            close(fd[1]);  // not gonna write
                            dup2(fd[0],0);
                            if(execvp(secondCommand[0], secondCommand) == -1)
                            {
                                printf("pipe failed due to a second command");
                                exit(1);
                            }
                            close(fd[0]);
                        }
                    }
                }

                // Areguments don't have redirect such as "ls"
                if(used_pipe)
                {
                    if(execvp(args[0], args) == -1)
                    {
                        printf("First command is an invalid command\n");
                        return 1;
                    }
                }
                if (redirect == 1) {
                    close(0); // close the reading

                } else if (redirect == 2) {
                    close(1); // close the writing
                }
                close(file); // close the entire file
            }
            exit(1); // exit when there is an issue
        }
        else if(pid > 0)
        {
            if(hasSecond != 0)
            {
                pid_t pid2 = fork();
                if(pid2 == 0)
                {
                    if(execvp(secondCommand[0], secondCommand) == -1)
                    {
                        printf("Second command is an invalid command\n");
                        return 1;
                    }
                }
                
                //waitpid(pid, NULL, 0);
            }
        }
        else
            printf("There is an error with fork()");
    }
    return 0;
}