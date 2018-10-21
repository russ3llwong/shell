# CSC 415 - Project 3 - My Shell

## Student Name: Russell Wong

## Student ID: 916507662

## Build Instructions
After cloning the project, use Terminal or anything else of the same sort to change directory to the project's directory. The one that contains the files of this project. Then, type in "gcc myshell.c" to compile the program. For example:

cd ~/Desktop/csc415-p3-russ3llwong
gcc myshell.c

if built successfully, an executable file named "a.out" will be generated in the same folder

## Run Instructions
to run the program, type in the following and hit enter:

./a.out

After that, you will enter the simple shell program. 
"myShell >> " will appear on a new line. Type in the desired commands and hit enter.

Enter "exit" to exit/stop the program.

## Project Overview
The project runs a simple bash Shell/Command Line Interpreter program. It allows the user to perform simple commands like on Terminal. The shell takes in user input and read it into a 256-byte buffer, then parse and execute the commands entered.

This simple shell program supports the following functions:
    1. Executing a single command with up to four command line arguments, like:
        Myshell> ls –a
        Myshell> ls –m /usr/src/linux
    2. Execute a command in background, like:
        Myshell> ls -­­l &
    3. Redirect the standard output of a command to a file, like: 
        Myshell> ls -­­l > outfile
        Myshell> ls -­­l >> outfile
    4. Redirect the standard input of a command to come from a file, like:
        Myshell> wc -w < test.txt
    5. Execute multiple commands connected by a single shell pipe, like:
        Myshell> ls | sort
    6. Execute the cd and pwd commands, like:
        Myshell> cd some_path
        Myshell> pwd

## List Extra Credits comepleted (if non attempted leave blank)
