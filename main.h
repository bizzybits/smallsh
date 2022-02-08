#define _GNU_SOURCE
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>


#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

#define MAX_VAR_NUM 10
#define PIPE_MAX_NUM 10
#define FILE_MAX_SIZE 40
#define MAXFILE 81

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")
static void sig_int(int);



void  handle_SIGTSTP(int signo) 
{
	int BACKGROUND_ALLOWED = 0;
	pid_t spawnpid = -5;
  	int during_process = 1;
  	char message[100];
  	char *background = "Exiting foreground-only mode!\n";
  	char *foreground = "Entering foreground-only mode (& is now ignored)\n"; 

  memset(message, '\0', sizeof(strlen(message)));

   /* keeps the signal handler waiting until child process dies,
     * creates a flag that signals that we need to fake a prompt */
    while (waitpid(spawnpid, NULL, WNOHANG) == 0) {
        during_process = 0;
    };

    if (BACKGROUND_ALLOWED && during_process) {
        strcpy(message, foreground);
        BACKGROUND_ALLOWED = 1;
    } else if (BACKGROUND_ALLOWED && !during_process) {

        /* if not during a process, we are adding a new line
         * so add the look of a prompt after */
        strcpy(message, foreground);
        strcat(message, ": ");
        BACKGROUND_ALLOWED = 1;
    } else if (!BACKGROUND_ALLOWED && during_process) {
        strcpy(message, background);
        BACKGROUND_ALLOWED = 0;
    } else {
        strcpy(message, background);
        strcat(message, ": ");
        BACKGROUND_ALLOWED = 0;
    }

    /* print our message to the screen */
    fflush(stdout);
    write(STDOUT_FILENO, message, strlen(message));

}

void handle_SIGINT(int signo){
	char* message = "Caught SIGINT, sleeping for 5 seconds\n";
	// We are using write rather than printf
	write(STDOUT_FILENO, message, 39);
	sleep(5);
}



// Function to take input
int takeInput(char* str)
{
	char* buf;
    size_t bufsize = 32;
    size_t characters;

    buf = (char *)malloc(bufsize * sizeof(char));
    if( buf == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

	characters = getline(&buf,&bufsize,stdin);
	if (characters != 0) {
        buf[strcspn(buf, "\n")] = 0;
		strcpy(str, buf);
		return 0;
	} else {
		return 1;
	}
}

// Function to print Current Directory.
void printPrompt()
{
	printf(": ");
}

//Function to print last call status
void printStatus(int childStatus)
{
	printf("exit value %d\n", childStatus);
}

// // Function where the system command is executed
// int execArgs(char** parsed)
// {
// 	// Forking a child
// 	int childStatus;
// 	pid_t childPid = fork();

// 	if (childPid == -1) {
// 		printf("\nFailed forking child..\n");
// 		return 1;
// 	} else if (childPid == 0) {
// 		if (execvp(parsed[0], parsed) < 0) {
// 			perror(parsed[0]);
// 			childStatus = 1;
// 		}
// 		return childStatus;
// 		exit(0);
// 	} else {
// 		// waiting for child to terminate
// 		int is_background = 0; //true
// 		printf("Child's pid = %d\n", childPid);
// 		childPid = waitpid(childPid, &childStatus, 0);
// 	//	printf("waitpid returned value %d\n", pid);
// 		if (WIFEXITED(childStatus))
// 		{
// 			//printf("Child %d exited normally with status %d\n", pid, WEXITSTATUS(childStatus));
// 			if (childStatus == 0)
// 			{
// 				return childStatus;
// 			}
// 			else 
// 				childStatus = 1;
// 			//	printf("exited notmrally wtih status other than 0 %d\n", childStatus);
// 				return childStatus;
// 		}
// 		else 
// 		{
// 		//	printf("Child %d exited abnormally due to signal %d\n", pid, WTERMSIG(childStatus));
// 			return childStatus;
// 		}
// 		wait(NULL);
// 	}
// }

// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe, int bgFlag)
{

  	int args = strlen(*parsed);

	int saved_stdout;

	if (args != 2){
		printf("Usage: ./main <filename to redirect stdout to>\n");
		exit(1);
	}

	int targetFD = open(parsedpipe[0], O_WRONLY | O_CREAT , 0640);
	
	if (targetFD == -1) {
		perror("open()");
		exit(1);
	}
	
	// Currently printf writes to the terminal
	//printf("The file descriptor for targetFD is %d\n", targetFD);

	// Use dup2 to point FD 1, i.e., standard output to targetFD
	saved_stdout = dup(STDOUT_FILENO);
	int result = dup2(targetFD, 1);
	if (result == -1) {
		perror("dup2"); 
		exit(2); 
	}
	// Now whatever we write to standard out will be written to targetFD
	//printf("All of this is being written to the file using printf\n"); 
	
	
	execArgs(targetFD, parsed, parsedpipe, bgFlag);
	close(targetFD);
	fflush(stdout);
	
	
	 dup2(saved_stdout, STDOUT_FILENO);
	 close(saved_stdout);
	return;
	
}

void err_syserr(const char *fmt, char * parsedpipe)
{
    int errnum = errno;
    if (errnum != 0)
        fprintf(stderr, "%d: %s\n", errnum, strerror(errnum));
    putc('\n', stderr);
    exit(EXIT_FAILURE);
}

int execArgs(int fd, char ** parsedpipe, char ** parsed, int bgFlag)
{	
	// Forking a child
	int childStatus;
	int saved_stdout;
	saved_stdout = dup(STDOUT_FILENO);
	int status;
	pid_t childPid = getpid();
	

	switch (fork()){
		case -1:
			perror("fork() failed\n");
			exit(1);
			break;

		case 0: //this is child zone
			//setting up handler for foreground 
			if (bgFlag == 1){
				struct sigaction SIGINT_action = {0};
				SIGINT_action.sa_handler = SIG_DFL;
				sigaction(SIGINT, &SIGINT_action, NULL);
			}

			if (bgFlag == 0){
				execArgsPiped(parsed, parsedpipe, bgFlag);
			}
			dup2(fd, 1); //fd becomes stdout
			execvp(parsedpipe[0], parsedpipe);
			err_syserr("cannot open %s for input", parsedpipe[0]);
			fflush(stdout);
			break;

		default: //parent
			if (bgFlag == 1)
			{
				printf("background pid is %d\n", childPid);
			}
			else 
			{
				waitpid(childPid, &status, 0); //picks up dead children
				if (WIFEXITED(childStatus))
				{
			//printf("Child %d exited normally with status %d\n", pid, WEXITSTATUS(childStatus));
					if (childStatus == 0)
					{
						return childStatus;
					}
					else 
						childStatus = 1;
			//	printf("exited notmrally wtih status other than 0 %d\n", childStatus);
						return childStatus;
				}
				else 
				{
		//	printf("Child %d exited abnormally due to signal %d\n", pid, WTERMSIG(childStatus));
				return childStatus;
				}
				wait(NULL);
			}
		break;
	}
}


// Function to execute builtin commands
int ownCmdHandler(char** parsed, int childStatus)
{
	int NoOfOwnCmds = 3, i, switchOwnArg = 0;
	char* ListOfOwnCmds[NoOfOwnCmds];
	char* homeDir;
	char* newDir;
	// int childStatus = 0;

	ListOfOwnCmds[0] = "exit";
	ListOfOwnCmds[1] = "cd";
	ListOfOwnCmds[2] = "#";


  //compares the first element of the parsed string (if parsed is "ls -la" then parsed[0] is only ls)
	for (i = 0; i < NoOfOwnCmds; i++) {
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
			switchOwnArg = i + 1; //if it matches it will be 1, 2, or 3
			break;
		}
	}

	switch (switchOwnArg) {
	case 1:
		printf("\nGoodbye\n");
		fflush(stdout);
		exit(0);
	case 2: //need to check if there is no parse[1], 
          // then need to set as if parse[1] = "~" or $HOME
		  homeDir = getenv("HOME");
		
		  if (parsed[1] == NULL)
		  {
			chdir(homeDir);
			return 1;
		  }
		 else 
		  	newDir = parsed[1];
		  	chdir(newDir);
			return 1;
	case 3:
		printf("\r");
		return 1;
	default:
		break;
	}

	return 0;
}

// function for finding background command
int findBackground(char* str, char** strpiped)
{
	char *i;
	int bgFlag;

	i = strstr(str, "&");
	if (i != NULL)
	{
	 	bgFlag = 0;
		return 1;
	}
	else 
		bgFlag = 1;
		return 0;
	
	
}
// function for finding pipe
int parsePipe(char* str, char** strpiped, int bgFlag)
{
	int i;
	for (i = 0; i < 2; i++) {
		strpiped[i] = strsep(&str, ">");
		if (strpiped[i] == NULL)
			break;
	}

	if (strpiped[1] == NULL)
		return 0; // returns zero if no pipe is found.
	else {
		return 1;
	}
}
// function for parsing command words
int parseSpace(char* str, char** parsed, int childStatus, int bgFlag)
{
	int i;


	for (i = 0; i < MAXLIST; i++) {
		parsed[i] = strsep(&str, " ");

		
		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
  if (ownCmdHandler(parsed, childStatus))
		return 0;
	else
		return 1;
}

int processString(char* str, char** parsed, char** parsedpipe, int childStatus, int bgFlag)
{

	char* strpiped[2];
	int piped = 0;
	int background = 0;

	piped = parsePipe(str, strpiped, bgFlag);
	background = findBackground(str, strpiped);

	if (background)
	{
		printf("background char found\n");
		parseSpace(strpiped[0], parsed, childStatus, bgFlag);
	}

	if (piped) 
	{
		parseSpace(strpiped[0], parsed, childStatus, bgFlag);
		parseSpace(strpiped[1], parsedpipe, childStatus, bgFlag);

	}else {
		parseSpace(str, parsed, childStatus, bgFlag);
	}
//# TODO look into why ownCmdHandler called in processString and parseSpace
	if (ownCmdHandler(parsed, childStatus))
		return 0;


	else
		return 1 + piped;
}

