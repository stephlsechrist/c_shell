# CSC 415 - Project 3 - My Shell

## Student Name: Stephanie Sechrist

## Build Instructions
This program was built on a Linux Virtual Machine in CLion 2018.3.4 in C language.
Clone the repo into your local directory. In Linux terminal in the repo, type
```
make
```
to compile the program. This will follow the instructions in Makefile to build program.

## Run Instructions
To run the program in the terminal, type 
```
./myshell
```
The shell is basic and assumes all commands will be separated by a space. It is case sensitive, just
like Linux terminal. cd and pwd are built-ins for the shell.

The & operator I have implemented only works if the & is at the end of the line and next command is 
on a new line. Example:
myshell >> test &
myshell >> test

Pipe implemented; collaborated with Thoams Sechrist and used Souza's example on iLearn as well.

## List Extra Credit comepleted (if not attempted leave blank)
