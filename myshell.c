/*****************************************************************
 * Name         :  Stephanie Sechrist                            *
 * Class        :  CSC 415                                       *
 * Date Due     :  March 19, 2019                                *
 * Last Modified:  March 19, 2019                                *
 * Description  :  Writing a simple bash shell program           *
 *                 that will execute simple commands. The main   *
 *                 goal of the assignment is working with        *
 *                 fork, pipes and exec system calls.            *
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

// prompts user
// parses the string entered by the user with strtok()
// first, removes new line character
// tokenizes until NULL reached
// saves each token into **arguments
// NULL terminates **arguments
// called from main() so returns **arguments to main to
// initialize myargv
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

    // null terminate 2d array
    arguments[i] = NULL;
    return arguments;
}

// gets argument count of what was entered at prompt; called by main
// to initialize myargc; never used
int
getArgCount(char **currArgs) {
    int argCount = 1;
    for (int i = 1; currArgs[i] != NULL; i++) {
        argCount++;
    }

    return argCount;
}

// this is where commands will execute using execvp
// if input or ouput redirection is found,
// the child process will call other methods
// relied heavily on stack overflow to implement redirection
// https://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection
//
// wanted to do all the checking for built-ins like cd, pwd, or exit in this function, but couldn't get it to work
// wasted too much time on it
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

        // look for <, >, >>
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
            else if (strcmp(currArgs[i], "<") == 0) {
                strcpy(inFile, currArgs[i + 1]);
                currArgs[i] = NULL;
                inRedirect = 1;
            }
        }

        // logic from filecopy program (last assignment)
        if (outRedirect) {
            int outputfd;
            // concatenate instead of truncate
            if (outRedirectCat) {
                outputfd = open(outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
            }
                // truncate
            else {
                outputfd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            }
            if (outputfd < 0) {
                perror("Open file to be copied to failed\n");
                exit(5);
            }

            dup2(outputfd, STDOUT_FILENO);
            close(outputfd);
        }

        // logic from filecopy program (last assignment)
        if (inRedirect) {
            int inputfd;
            inputfd = open(inFile, O_RDONLY);
            dup2(inputfd, STDIN_FILENO);
            close(inputfd);
            if (inputfd < 0) {
                perror("File not found");
                exit(2);
            }
        }

//        printf("%s\n", currArgs[0]);

        // execute !
        execvp(currArgs[0], currArgs);

        // if user presses enter at prompt, exit execute so that
        // prompt displayed again in main
        if (strcmp(currArgs[0], "\n") == 0) {
            exit(127);
        }
        // only reached if command not pwd, cd, exit, or
        // any command recognized by execvp
        printf("%s: command not found\n", currArgs[0]);
        exit(127);
    }
}

// called when pwd is typed into prompt
// implements built-in pwd for myshell using getcwd()
void
getpwd() {
    char tempwd[BUFFERSIZE];
    getcwd(tempwd, sizeof(tempwd));
    printf("%s\n", tempwd);
}

// called when cd is typed into prompt
// implements built-in cd for myshell using chdir()
void
changeDir(char **passedArgs) {
    // if statement if cd is only argument
    // will change to home directory
    if (passedArgs[1] == NULL) {
        const char *homeName = getenv("HOME");
        chdir(homeName);
    }
    else if (chdir(passedArgs[1]) < 0) {
        printf("No such directory\n");
    }
    else;
}

// was not familiar with & operator prior to this project
// implemented it based on description from
// https://bashitout.com/2013/05/18/Ampersands-on-the-command-line.html
// pargv passed in should be parsed already, separating command before &
int
subshell(char **pargv) {
    char buf[BUFFERSIZE];
    pid_t pid = fork();
    // in parent
    if (pid > 0) {
        pargv = getInput(pargv, buf);
        executeArgs(pargv);
        wait(0);
    }
        // in child
    else if (pid == 0) {
        if (strcmp(pargv[0], "pwd") == 0) {
            getpwd();
        }
        else if (strcmp(pargv[0], "cd") == 0) {
            changeDir(pargv);
            // wanted to put in separate execute function. haven't figured out yet
        }
        else {
            executeArgs(pargv);
        }
        exit(127);
    }
    else {
        perror("Fork failed");
        return -1;
    }

    return 0;
}

// ran out of time to fully implement piping
// i got so far as to separate the left side of the pipe from the right
// since I don't have anything implemented, I at least displayed the
// separation to the user
void
pipeFunction(char **passedArgs, int pipeAt) {
    char **leftSide = malloc(BUFFERSIZE * sizeof(char *));
    char **rightSide = malloc(BUFFERSIZE * sizeof(char *));
    int i = 0;


    while (i < pipeAt) {
        leftSide[i]= passedArgs[i];
        printf("left side: %s ",leftSide[i]);
        i++;
    }
    putchar('\n');
    leftSide[pipeAt] = NULL;
    i++;

    int j = 0;
    while (passedArgs[i] != NULL){
        rightSide[j] = passedArgs[i];
        printf("right side: %s ",rightSide[j]);
        i++;
        j++;
    }
    putchar('\n');
    rightSide[i] = NULL;

//    passedArgs[pipeAt] = NULL;
}


int
main(int argc, char **argv) {
    char buffer[BUFFERSIZE];
    char **myargv = malloc(BUFFERSIZE * sizeof(char *));
    // processCount used for & operator
    int processCount = 0;

//    char **myargv;
    // int flag default is false. changed to nonzero in for loop
    int myargc;

    while (1) {
        // location flag for ampersand
        int ampLoc = -1;
        // location flag for pipe
        int pipeLoc = -1;

        myargv = getInput(myargv, buffer);

        // for loop to look for pipe and ampersand if present
        for (int i = 0; myargv[i] != NULL; i++) {
            if (strcmp(myargv[i], "&") == 0) {
                ampLoc = i;
            }

            if (strcmp(myargv[i], "|") == 0) {
                pipeLoc = i;
                printf("Location of pipe: %d\n",i);
            }

        }

        // if exit typed into prompt, break to exit myshell
        if (strcmp(myargv[0], "exit") == 0) {
            break;
        }

        // initialize myargc; don't use anywhere though
        myargc = getArgCount(myargv);

        // first time encountering & operator, so I tried to mimic what
        // I saw in the linux terminal.
        // if statement entered if & found in previous for loop
        if (ampLoc >= 0) {
            for (int i = ampLoc; myargv[i] != NULL; i++) {
                myargv[i] = NULL;
            }

            if (subshell(myargv) == 0) {
                processCount++;
                printf("\n[%d]+\tDone\t%s\n", processCount, myargv[0]);
            }
            else
                perror("& Operator Failure");
        }

        // did not fully implement piping
        else if (pipeLoc >= 0) {
            pipeFunction(myargv, pipeLoc);
        }

        // pwd built-in
        // if pwd typed into prompt, call getpwd()
        else if (strcmp(myargv[0], "pwd") == 0) {
            getpwd();
        }
            // cd built-in
            // if cd typed into prompt, call changeDir()
        else if (strcmp(myargv[0], "cd") == 0) {
            changeDir(myargv);
        }
            // for all else commands, execute as normal by calling executeArgs()
        else {
            executeArgs(myargv);
        }
    }
    return 0;
}