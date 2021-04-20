
This project consists of designing a C program to serve as a shell interface that accepts user commands and then executes
each command in a separate process. Its implementation will support input and output redirection, as well as pipes, as 
a form of IPC between a pair of commands. Project uses UNIX fork(), exec(), wait(),
dup2(), and pipe() system calls and can be completed on any Linux, UNIX, or macOS system.

A shell interface gives the user a prompt, after which the next command is entered.
The example below illustrates the prompt osh> and the user's next command: cat prog.c.  
(This command displays the file prog.c on the terminal using the UNIX cat command)

osh>cat prog.c




This project is organized into several parts:
    I.   Creating the child process and executing the command in the child
    II.  Providing a history feature
    III. Adding support of input and output redirection
    IV.  Allowing the parent and child processes to communicate via a pipe
