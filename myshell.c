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
#include <stdbool.h>

/* CANNOT BE CHANGED */
#define BUFFERSIZE 256
/* --------------------*/
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)
#define BUFF_MAX 21

char
**getInput(char **arguments, char *input) {
    printf(PROMPT);
    fgets(input, BUFFERSIZE, stdin);

    strtok(input, "\n");
    char *status = strtok(input, " ");
    int i = 0;

    while (status != NULL) {
        arguments[i] = status;
        i++;
        status = strtok(NULL, " ");
    }

    arguments[i] = NULL;
    return arguments;
}

int
getArgCount(char **currArgs) {
    int argCount = 1;
    for (int i = 1; currArgs[i] != NULL; i++) {
        argCount++;
    }

    return argCount;
}

// most of code taken from my HW2: filecopy.c
// takes a file from user input
// and copies into a file specified by the user
// in the current working directory.
// If the file to be copied to does not exist,
// it is created with read and write permissions.
// If the file to be copied to does exist, it is
// overwritten with the new content.
//void
//outputRedirect(char sourceFile[255]) {
//    char buffer[BUFF_MAX];
////    char sourceFile[255];
//    char destFile[255];
//
//    int fout;
//    int destfd;
//
//    fout = open(sourceFile, O_RDONLY);
//    if (fout < 0) {
//        perror("Open file to be copied failed");
//        exit(77);
//    }
//
//    destfd = open(destFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
//    if (destfd < 0) {
//        perror("Open file to be copied to failed");
//        exit(77);
//    }
//
//    ssize_t bytesRead = read(sourcefd, buffer, sizeof(buffer));
//    while (bytesRead > 0) {
//        ssize_t bytesCopied = write(destfd, buffer, (size_t) bytesRead);
//        if (bytesCopied != bytesRead) {
//            perror("There was an error during copy");
//            exit(11);
//        }
//        bytesRead = read(sourcefd, buffer, sizeof(buffer));
//    }
//
//    if (bytesRead < 0) {
//        perror("There was an error during copy");
//        exit(11);
//    }
//
//    close(sourcefd);
//    close(destfd);
//}

// this is where commands will execute using execvp
// if input or ouput redirection is found,
// the child process will call other methods
// relied heavily on stack overflow to implement redirection
// https://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection
void
executeArgs(char **currArgs) {
    // int flags set to 0 for false by default
    // flag for <
    int inRedirect = 0;
    // flag for >
    int outRedirect = 0;
    // flag for >>
    int outRedirectCat = 0;

    pid_t pid = fork();
    if (pid < 0)
        perror("Fork failed\n");
        // in parent process
    else if (pid > 0) {
        wait(0);
    }
        // in child process
        // here is where I will implement input or output redirection
    else {
        char inFile[BUFFERSIZE];
        char outFile[BUFFERSIZE];

        for (int i = 0; currArgs[i] != NULL; i++) {
            if (strcmp(currArgs[i], ">") == 0 || strcmp(currArgs[i], ">>") == 0) {
                if (strcmp(currArgs[i], ">>") == 0) {
                    // concatenate instead of overwrite
                    outRedirectCat = 1;
                }
                strcpy(outFile, currArgs[i + 1]);
                // change > to NULL
                currArgs[i] = NULL;
                outRedirect = 1;
            }

            else if (strcmp(currArgs[i], "<") == 0){
                strcpy(inFile, currArgs[i+1]);
                currArgs[i] = NULL;
                inRedirect = 1;
            }
        }

        if (outRedirect) {
            int outputfd;
            if (outRedirectCat) {
                outputfd = open(outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
            } else {
                outputfd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            }
            if (outputfd < 0) {
                perror("Open file to be copied to failed\n");
                exit(5);
            }

            dup2(outputfd, STDOUT_FILENO);
            close(outputfd);
        }

        if (inRedirect){
            int inputfd;
            inputfd = open(inFile, O_RDONLY);
            dup2(inputfd, STDIN_FILENO);
            close(inputfd);
            if (inputfd < 0 ){
                perror("File not found");
                exit(2);
            }
        }

//        printf("%s\n", currArgs[0]);
        execvp(currArgs[0], currArgs);
        if (strcmp(currArgs[0], "\n") == 0){
            exit(127);
        }
        printf("%s: command not found\n", currArgs[0]);
        exit(127);
    }

}

void
getpwd() {
    char tempwd[BUFFERSIZE];
    getcwd(tempwd, sizeof(tempwd));
    printf("%s\n", tempwd);
}

void
changeDir(char **thing) {
//    if(strcmp(thing[1],"\0") == 0){
    // if statement if cd is only argument
    // will change to home directory
    if (thing[1] == NULL) {
        const char *homeName = getenv("HOME");
        chdir(homeName);
    } else if (chdir(thing[1]) < 0) {
        printf("No such directory\n");
    } else;
}

// was not familiar with & operator prior to this project
// implemented it based on description from
// https://bashitout.com/2013/05/18/Ampersands-on-the-command-line.html
// sargv passed in should be parsed already, separating what is before the pipe from the rest
int
subshell(char **sargv){
//    sargv = malloc(BUFFERSIZE * sizeof(char *));

//    sargc = getArgCount(sargv);
    printf("in subshell: %s", sargv[0]);
    printf("before execute in subshell\n");
    executeArgs(sargv);
    printf("after execute in subshell\n");

    return 0;
}

int
main(int argc, char **argv) {
    char buffer[BUFFERSIZE];
    char **myargv = malloc(BUFFERSIZE * sizeof(char *));

//    char **myargv;
    // int flag default is false. changed to nonzero in for loop
    int ampLoc = 0;
    int myargc;

    while (1) {
        myargv = getInput(myargv, buffer);

        for (int i = 0; myargv[i] != NULL; i++){
            if (strcmp(myargv[i], "&") == 0){
                ampLoc = i;
                printf("Ampersand location at index %d\n",ampLoc);
//                myargv[i] = NULL;
            }
        }

        if (strcmp(myargv[0], "exit") == 0) {
            break;
        }
        myargc = getArgCount(myargv);
//    printf("Number of arguments: %d",myargc);

        if (ampLoc){
            printf("Entered ampLoc not zero if statement\n");
            char **ampCommand = malloc(BUFFERSIZE * sizeof(char *));
            printf("Allocated memory for ampcommand\n");
            for (int i = 0; i < ampLoc; i++){
                printf("in for loop: i = %d\n",i);
                printf("Entered for loop to try to parse out &");
                strcpy(ampCommand[i], myargv[i]);
                printf("Copying %s to ampCommand\n", myargv[i]);
                printf("ampCommand[%d] now has %s\n",i,ampCommand[i]);
            }

            if(subshell(ampCommand) == 0 ){
                printf("it worked");
            }
        }

        if (strcmp(myargv[0], "pwd") == 0) {
            getpwd();
        } else if (strcmp(myargv[0], "cd") == 0) {
            changeDir(myargv);
            // wanted to put in separate execute function. haven't figured out yet
        } else {
            executeArgs(myargv);
//            pid_t pid = fork();
//            if (pid < 0) {
//                perror("Fork failed\n");
//            }
//                // inside child process
//            else if (pid == 0) {
//                int exStatus = execvp(myargv[0], myargv);
//                if (exStatus < 0) {
//                    perror("There was an error during execution");
//                    exit(96);
//                }
//            }
//
//                // back in parent process
//            else {
//                wait(0);
//            }
        }
    }
    return 0;
}