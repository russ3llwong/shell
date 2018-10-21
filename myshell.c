/****************************************************************
* Name        :	Russell Wong                                   *
* Class       : CSC 415                                        *
* Date        : 10/15/18                                       *
* Description :  Writting a simple bash shell program          *
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
#include <fcntl.h>
#include <sys/wait.h>

#define BUFFERSIZE 256
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)

//***function headers
void execute(char*[], int, int, int); //function to execute commands without pipe
void execPipe(char*[], char*[]); // function to execute commands with pipe

//main function
int main()
{
	//infinite loop until exit or EOF
	while (1) {
		//prints prompt
		printf("%s", PROMPT); 

		//two sets of buffer
		char buffer[BUFFERSIZE], copy[BUFFERSIZE];
		//arg counters, temp, file descriptors and flags
		int myargc = 0, myargc2 = 0, temp = 0, background = 0, fd1 = 0, fd2 = 0, pipeFlag = 0;

		//get buffer and if not empty,copy a set of it
		if (fgets(buffer, BUFFERSIZE, stdin) != NULL) {
			strncpy(copy, buffer, BUFFERSIZE);
		}
		else {
			continue;
		}

		//tokenize buffer in order to count arguments and detect pipe
		char* token;
		token = strtok(buffer, " \n");
		while (token != NULL) {
			if (strcmp(token, "|") == 0) {
				//if pipe deteced, turn pipeFlag on, store myargc and reset it to zero to count the other arguments
				pipeFlag = 1;
				temp = myargc;
				myargc = 0;
			}
			else {
				//otherwise increase counter
				myargc++;
			}
			//reads next token
			token = strtok(NULL, " \n");
		}

		//proceed if there are arguments
		if (myargc > 0) {
			//if pipe command, assign myargc2's value and reset myargc to its original value
			if (pipeFlag) {
				myargc2 = myargc;
				myargc = temp;
			}

			//create char pointer-arrays for the arguments
			char *myargv[myargc + 1];
			char *myargv2[myargc2 + 1]; //for commands with pipe only

			//helper variables
			int i = 0, j = 0;
			//to loop through the buffer again, this time to initialize the pointer-arrays for the arguments
			myargv[i] = strtok(copy, " \n");

			//loop will end after encountering '|','<','<<','>' or '&'
			do {
				if (strcmp(myargv[i], "|") == 0) {
					//to initialize the right-side of arguments
					do {
						myargv2[j] = strtok(NULL, " \n");
					} while (myargv2[j++] != NULL);
					myargv[i] = myargv2[--j];
					break;
				}
				if (strcmp(myargv[i], ">") == 0) {
					//get the next token, which is the file name
					myargv[i] = strtok(NULL, " \n");
					//create file, otherwise rewrite file
					fd1 = open(myargv[i], O_RDWR | O_CREAT | O_TRUNC, 0644);
					if (fd1 == -1) {
						perror("Error creating file\n");
						exit(0);
					}
					myargv[i] = strtok(NULL, " \n");
					break;
				}
				if (strcmp(myargv[i], ">>") == 0) {
					//get the next token, which is the file name
					myargv[i] = strtok(NULL, " \n");
					//append file
					fd1 = open(myargv[i], O_RDWR | O_CREAT | O_APPEND, 0644);
					if (fd1 == -1) {
						perror("Error appending file\n");
						exit(0);
					}
					myargv[i] = strtok(NULL, " \n");
					break;
				}
				if (strcmp(myargv[i], "<") == 0) {
					//get the next token, which is the file name
					myargv[i] = strtok(NULL, " \n");
					//read file
					fd2 = open(myargv[i], O_RDONLY, NULL);
					if (fd2 == -1) {
						perror("Error reading file\n");
						exit(0);
					}
					myargv[i] = strtok(NULL, " \n");
					break;
				}
				if (strcmp(myargv[i], "&") == 0) {
					//turn background flag on
					background = 1;
					myargv[i] = strtok(NULL, " \n");
					break;
				}

				myargv[++i] = strtok(NULL, " \n");

			} while (myargv[i] != NULL);


			//check commands
			if (strcmp(myargv[0], "exit") == 0) {
				//if exit is entered, program will exit
				exit(0);
			}
			else if (strcmp(myargv[0], "cd") == 0) {
				//if no arguments after cd, it will change to the home directory
				if (myargv[1] == NULL) {
					chdir(getenv("HOME"));
				}
				//otherwise it will change to designated path
				else {
					chdir(myargv[1]);
				}
			}
			else if (strcmp(myargv[0], "pwd") == 0) {
				//create buffer to store current working directory
				char pwd[BUFFERSIZE];
				if (getcwd(pwd, BUFFERSIZE) != NULL) {
					//print if not null
					printf("%s\n", pwd);
				}
			}
			else {
				//for pipe commands
				if (pipeFlag) {
					execPipe(myargv, myargv2);
				}
				//for non-pipe commands
				else {
					execute(myargv, fd1, fd2, background);
				}
			}
		}
	}
	return 0;
}

//function to execute commands without pipe
void execute(char *myargv[], int fd1, int fd2, int background) {
	pid_t pid;

	//fork a new proces
	pid = fork();
	if (pid == -1) {
		perror("Failed to create new process");
		exit(-1);
	}
	else if (pid == 0) { //child process
		if (fd1) { //commands with ">" or ">>"
			close(1);
			dup(fd1);
			if (execvp(myargv[0], myargv) < 0) {
				perror("Error: invalid command\n");
				exit(1);
			}
			close(fd1); //close file descriptor
		}
		else if (fd2) { //commands with "<"
			close(0);
			dup(fd2);
			if (execvp(myargv[0], myargv) < 0) {
				perror("Error: invalid command\n");
				exit(1);
			}
			close(fd2); //close file descriptor
		}
		else { //commands without file I/O
			if (execvp(myargv[0], myargv) < 0) {
				perror("Error: invalid command\n");
				exit(1);
			}
		}
	}
	else { //parents process
		//if there's no '&',parent waits for child process
		if (!background) {
			waitpid(pid, NULL, 0);
		}
	}
}

//functions to execute commands with pipe
void execPipe(char *myargv[], char *myargv2[]) {
	pid_t pid, pid2; 
	int pipefd[2]; //0 is read end, 1 is write end

	if (pipe(pipefd) < 0) {
		perror("Failed to initialize pipe");
		exit(-1);
	}

	//fork a new process
	pid = fork();
	if (pid == -1) {
		perror("Failed to create new process");
		exit(-1);
	}
	if (pid == 0) { //child process
		//only needs to read at the read end
		close(pipefd[1]);
		close(0);
		dup(pipefd[0]);
		if (execvp(myargv2[0], myargv2) < 0) {
			perror("Error: invalid command\n");
			exit(1);
		}
		close(pipefd[0]);
	}

	//fork a new process
	pid2 = fork();
	if (pid2 == -1) {
		perror("Failed to create new process");
		exit(-1);
	}
	if (pid2 == 0) { //child process
		//only needs to write at the write end
		close(pipefd[0]);
		close(1);
		dup(pipefd[1]);
		if (execvp(myargv[0], myargv) < 0) {
			perror("Error: invalid command\n");
			exit(1);
		}
		close(pipefd[1]);
	}
	//close both ends of pipe
	close(pipefd[0]);
	close(pipefd[1]);

	//parent waits for child processes
	wait(NULL);
	wait(NULL);
}
