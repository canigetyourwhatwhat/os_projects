// Created by Daichi Ando

/*
Description about this program
    This program contains two methods to make shell work: readInput() 
    and main(). This program first prints "osh>", then user enters a command
    that he or she wants, and execute the command. It keeps looping until 
    the user manually stops the program. 
*/

#include <stdio.h>    // printf
#include <unistd.h>   // pipe, fork, dup2, close, read, exec
#include <string.h>   // strtok
#include <fcntl.h>    // open
#include <wait.h>     // wait


// ---------------------------------------------------------------------------
/*This funtion reads the command that user typed. Steps are below
    1. It reads input and switch \n and \0. (we need the change to check)
    2. If the command was !!, it returns because we execute the previous 
        command that is stored already in the args[] in main().
    3. If it is not !!, then I free up the memory that is stored in commands[]
    4. Now it starts storing the entered command. It will separately store by 
        using delimiter. If a user typed "&", then it also stores the latter
        command in secondCommand[]. Note that the latter command must be only
        one word such as whoami and ls. 

*/
// ---------------------------------------------------------------------------
void readInput(char *commands[], int* first_command_length, 
               char *secondCommand[], int *second_command_length)
{
    // Set the buffer to store input
    char user_input[80]; 

    // It will be used for delimiter
    const char s[] = " ";
    
    // Get user's input
    int length = read(0, user_input, 80);
    
    // Make sure the command ends with \0
    if (user_input[length - 1] == '\n')
    {
        user_input[length - 1] = '\0';
    }
    
    // Check if the command was !!, then return back 
    //                  because no nedd to proceed anymore. 
    if (strcmp(user_input, "!!") == 0)
    {
        // returns message if there was no command before 
        if (*first_command_length == 0) 
        {
            printf("There is no command before.\n");
        }
        return;
    }

    // set to 0 to all the variables. 
    *first_command_length = 0;
    *second_command_length = 0;

    // Take the first command from the user input
    char *temp = strtok(user_input, s);
    
    // Keep looping until finish taking all the input.
    while( temp != NULL)
    {
        // If this sign is catched, the latter command 
        //              will be stored in secondCommand[].
        if (temp[0] == '&')
        {
            // take a command
            temp = strtok(NULL, s);
            *second_command_length+= 1;
            // Store it
            secondCommand[*second_command_length-1] = strdup(temp);
            // End the secondCommand[] with NULL
            secondCommand[*second_command_length] = NULL;
            // firstCommand is also supposed to be end, so store NULL as well
            commands[*first_command_length] = NULL;
            // Back to main
            return;
        }
        *first_command_length+= 1;
        // Store it
        commands[*first_command_length-1] = strdup(temp);
        // Get the new command. If it is null, this 
        //          while loop will be terminated at the top. 
        temp = strtok(NULL, s);
    }
    // If '&' didn't appear, make sure to store NULL at the end. 
    commands[*first_command_length] = NULL;    

}

// ---------------------------------------------------------------------------
/*This funtion exectues the command(s) that user typed. Steps are below
    1. It reads the user's input with readInput().
    2. Start while loop by printing out "osh>". There are multiple if-else 
        statements. 
        (1) If ">" was caught, open the file, connect to dup2(), and execute
        (2) If "<" was caught, open the file, connect to dup2(), and execute
        (3) If "|" was caught, store two commands to the new one, and create a 
                pipe, and do fork() to execute two commands. 
    3. If none of the above cases happened, then simply call execvp().
    4. It also closes the reading and writing if "<" or ">" is cateched. 
    5. The last if-statement catches the case if there "&" appeared. the latter
        command is executed at here.
    6. At the end, it prints out the error if the fork() was incorrect. 

*/
// ---------------------------------------------------------------------------
int main(void)
{
    // There is a limit length of arguments
    char *args[80 / 2 + 1];
    // get the id of the process
    pid_t pid = 0;
    // Stores the length of the command 
    int command_count = 0;
    // If used pipe, it will be 1. It is used to skip additional execvp().
    int used_pipe = 0;
    // There is a limit length of arguments
    char* secondCommand[80 / 2 + 1];
    // If it has "&", then it will be 1. 
    int hasSecond = 0;

    // It will keeps looping forever
    while(1)
    {
        // reset the value
        used_pipe = 0;

        printf("osh>");
        fflush(stdout);

        // read the user's input 
        readInput(args, &command_count, secondCommand, &hasSecond);          

        // If there was no input such as single tab, it skips. 
        if(command_count == 0)
        {
            continue;
        }        

        // Get the fork here!         
        pid_t pid = fork();

        // Enter to a child process
        if(pid == 0)
        {
            // If there "<" or ">" is catched, it will redirect to 1 or 2.
            int redirect = 0;
            // Stores the id of file here 
            int file = 0;

            // It checks whether the user's input contains some key words.
            for (int i = 1; i < command_count; i++)
            {
                if (strcmp(args[i], ">") == 0)
                {
                    // It gives all the permission with 0666.
                    // It creates a new file and write the targeting file
                    //      to the new file. 
                    file = open(args[i+1], O_WRONLY | O_CREAT, 0666); 

                    // If thre was an issue with the file, exit. 
                    if(file == -1)
                    {
                        printf("There is an issue on this file");
                        _exit(1);
                    }

                    // It connects to the dup2. The output of execvp() later
                    //      on will be stored in this file. 
                    dup2(file, 1); 
                    // Set NULL since we only need the first command
                    args[i] = NULL;
                    // Set NULL since we only need the first command
                    args[i+1] = NULL;
                    // It redirects to 2!
                    redirect = 1;
                    break;
                }
                // Ditto to the "<" case
                else if (strcmp(args[i], "<") == 0)
                {
                    // Open the file but read only
                    file = open(args[i+1], O_RDONLY); 
                    if(file == -1)
                    {
                        printf("There is an issue on this file");
                        _exit(1);
                    }
                    // This time is input 
                    dup2(file, 0); 
                    args[i] = NULL;
                    args[i+1] = NULL;
                    redirect = 2;
                    break;
                }                    
                else if (strcmp(args[i], "|") == 0)
                {
                    // Former command 
                    char* firstCommand [i+1];
                    // Latter one
                    char* secondCommand [command_count-i];

                    // Store the command in args to firstCommand
                    for(int j=0; j<i; j++)
                    {
                        firstCommand[j] = args[j];
                    }
                    firstCommand[i] = NULL;                    

                    // Ditto to secondCommand
                    for(int j=0; j<command_count-i-1; j++)
                    {
                        secondCommand[j] = args[i+j+1];
                    }
                    secondCommand[command_count-i-1] = NULL;
                        
                    // Set Pipe 
                    int fd[2];
                    if(pipe(fd) < 0)
                    {
                        printf("pipe error");
                        _exit(1);
                    }
                    // Mark that it used pipe! 
                    used_pipe = 1;    
                    // Fork again to execute second comamnd!                                           
                    int pid2 = fork();                        
                    if(pid2 == 0)   // Grandchild process
                    {
                        // not gonna read       
                        close(fd[0]); 
                        // connect to the writing
                        dup2(fd[1], 1); 
                        if(execvp(firstCommand[0], firstCommand) == -1)
                        {
                            printf("pipe failed due to a first command");
                            _exit(1);
                        }
                        // close writing
                        close(fd[1]);
                    }
                    else    // Child process
                    {
                        // wait until grandchild process completes the writing
                        wait(NULL);  
                        // not gonna write  
                        close(fd[1]);
                        // connects to reading
                        dup2(fd[0],0);
                        if(execvp(secondCommand[0], secondCommand) == -1)
                        {
                            printf("pipe failed due to a second command");
                            _exit(1);
                        }
                        close(fd[0]);
                    }
                    break;                        
                }
            }

            // Any input command that doesn't have "|" goes here.
            if(!used_pipe)
            {
                if(execvp(args[0], args) == -1)
                {
                    printf("First command is an invalid command\n");
                    _exit(1);
                }
            }
            // If ">" was caught, close writing after execvp()
            if (redirect == 1)
            {
                 close(1);
            }
            // If "<" was caught, close reading after execvp()
            else if (redirect == 2)
            {
                close(0);
            }
            // Make sure to close the file also. 
            close(file); 
        }
        else if(pid > 0)
        {
            // If the user's input contained "&", executes the latter command
            if(hasSecond != 0)
            {
                // Simply call fork() and execvp() for the second command.
                pid_t pid2 = fork();
                if(pid2 == 0)
                {
                    if(execvp(secondCommand[0], secondCommand) == -1)
                    {
                        printf("Second command is an invalid command\n");
                        return 1;
                    }
                }
            }
        }
        else
        {
            printf("There is an error with fork()\n");
        }
         
    }
}