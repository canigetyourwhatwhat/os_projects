#include <stdio.h>  // printf
#include <assert.h> //assert
#include <unistd.h> // fork and exec
#include <string.h>   // strtok
//#include <wait.h>
#define MAX_LINE 80

void readInput(char *commands[], int* first_command_length, char *secondCommand[], int *second_command_length)
{
    char user_input[MAX_LINE]; 
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
    char *args[MAX_LINE / 2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */
    pid_t pid = 0;
    int command_count = 0;
    int num_using_pipe = 0;
    char* secondCommand[MAX_LINE / 2 + 1];
    int hasSecond = 0;
    while(should_run)
    {
        num_using_pipe = 0;  // reset 
        fflush(stdout);
        printf("osh>");

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
                int redirect_case = 0;
                //int file;
                for (int i = 1; i <= command_count-1; i++)
                {
                    if (strcmp(args[i], "<") == 0)
                    {

                    }
                    else if (strcmp(args[i], ">") == 0)
                    {
                        
                    }
                    else if (strcmp(args[i], "|") == 0)
                    {

                    }
                }

                // Areguments don't have redirect
                if(num_using_pipe == 0)
                {
                    if(execvp(args[0], args) == -1)
                    {
                        printf("First command is an invalid command\n");
                        //return 1;
                    }
                }
                /*
                if (redirectCase == 1) {
                    close(STDIN_FILENO);

                } else if (redirectCase == 2) {
                    close(STDOUT_FILENO);
                }
                close(file);
                */
            }
            //exit(1);            
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
            printf("Error fork!!");
    }
    return 0;
}