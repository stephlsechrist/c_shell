/****************************************************************
 * Name        :  Stephanie Sechrist                            *
 * Class       :  CSC 415                                       *
 * Date        :                                                *
 * Description :  Writing a simple bash shell program           *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

/* CANNOT BE CHANGED */
#define BUFFERSIZE 256
/* --------------------*/
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)

int
execute(char** myargv){
    pid_t pid = fork();
    if (pid < 0)
        perror("Fork failed\n");
        // in child process
    else if (pid == 0){
        int exStatus = execvp(myargv[0], myargv);
        if (exStatus < 0){
            perror("There was an error during execution");
        }
    }
        // in parent process
    else
        wait(0);

    return 0;
}

char
**getInput(){
    char buffer[BUFFERSIZE];
    char** myargv = malloc(BUFFERSIZE * sizeof(char*));

    printf(PROMPT);
    fgets(buffer, BUFFERSIZE, stdin);
    strtok(buffer, "\n");
    char* status = strtok(buffer, " ");
    int i = 0;

    while (status != NULL){
        myargv[i] = status;
        printf("myargv[%d]: %s\n", i, status);
        i++;
        status = strtok(NULL, " ");
    }

    myargv[i] = NULL;
    return myargv;
}

int
main(int argc, char** argv)
{
//    int myargc = 0;

    char** myargv = getInput();
//    execute(myargv);
    pid_t pid = fork();
    pid_t waitpid;
    if (pid < 0){
        perror("Fork failed\n");
    }
    // inside child process
    if (pid == 0){
//        printf("in child process\n");
        int exStatus = execvp(myargv[0], myargv);
        if (exStatus < 0){
            perror("There was an error during execution");
        }

    }

    // back in parent process
    if (pid > 0){
//        printf("in parent process\n");
        wait(0);
    }

//    waitpid = wait(pid);
return 0;
}