#include <stdio.h>  // printf
#include <assert.h> //assert
#include <unistd.h> // fork and exec
#include <string.h>   // strtok
#define MAX_LINE 80

void readInput(char *commands[])
{
    char *args[MAX_LINE]; /* command line arguments */  
    const char s[1] = " ";
    
    int length = read(STDIN_FILENO, args, 80);
    commands[0] = strtok(args, s);
    int i = 0;
    while( commands[i] != NULL)
    {
        //printf(" %s\n", commands[i]);
        i++;
        commands[i] = strtok(NULL, s);
    }    
}

int main(void)
{
    char *input [MAX_LINE];
    printf("osh>");
    fflush(stdout);


    // read the user's input 
    readInput(input);   


    // Check whether the input is successfully stored 
    int i = 0;
    while(input[i] != NULL)
    {
        printf(" %s\n", input[i]);
        i++;
    }
    

    // Make a child by fork()
    pid_t pid = fork();
    assert(pid != -1);
    if(pid == 0)
    {
        printf("it is in child now \n");
        //int execret = execlp("/bin/ls", "ls", "-A", NULL);
        //char* concat = "/bin/";
        int execret = execvp(input[0], input);
        //char* concat = strcat("/bin/", input[0]);
        //int execret = execvp(concat, input);
        assert(execret >= 0);
        return 0;
    }
    else
    {
        printf("entered parent now \n");
        int psret;
        wait(&psret);
        printf("ps command returned %d \n", psret);
    }

    printf("Parent: back in main thread \n");
    return 0;
}