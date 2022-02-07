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

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

#define MAX_VAR_NUM 10
#define PIPE_MAX_NUM 10
#define FILE_MAX_SIZE 40
#define MAXFILE 81

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

struct child {
  char *command;
  int pid;
  int status;
  struct child *next;
  struct child *prev;
};


//to find the $$?
void slice(const char *str, char *result, size_t start, size_t end)
{
    strncpy(result, str + start, end - start);
}

// struct sigaction {
// 	void (*sa_handler)(int);
// 	sigset_t sa_mask;
// 	int sa_flags;
// 	void (*sa_sigaction)(int, siginfo_t*, void*);
// };

//typedef void (*SigHandler)(int signum);

// void handle_sigint(int sig)
// {
//     printf("terminated by signal %d\n", sig);
// 	fflush(stdout);
// }

// void sighandler(int sig_num)
// {
//     // Reset handler to catch SIGTSTP next time
//     signal(SIGTSTP, sighandler);
//     printf("Entering foreground-only mode\n");
// 	fflush(stdout);
// }


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
	printf("exit valud %d\n", childStatus);
}

// Function where the system command is executed
int execArgs(char** parsedArgs)
{
	// Forking a child
	int childStatus;
	pid_t pid = fork();

	if (pid == -1) {
		printf("\nFailed forking child..\n");
		return 1;
	} else if (pid == 0) {
		if (execvp(parsedArgs[0], parsedArgs) < 0) {
			perror(parsedArgs[0]);
			childStatus = 1;
		}
		return childStatus;
		exit(0);
	} else {
		// waiting for child to terminate
	//	printf("Child's pid = %d\n", pid);
		pid = waitpid(pid, &childStatus, 0);
	//	printf("waitpid returned value %d\n", pid);
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
}

void err_syserr(const char *fmt, char * parsedRedirectArgs)
{
    int errnum = errno;
    if (errnum != 0)
        fprintf(stderr, "%d: %s\n", errnum, strerror(errnum));
    putc('\n', stderr);
    exit(EXIT_FAILURE);
}

void runcmd(int fd, char ** parsedRedirectArgs){

	int saved_stdout;
	saved_stdout = dup(STDOUT_FILENO);
	int status;

	switch (fork()){
		case 0: //child
			dup2(fd, 1); //fd becomes stdout
			execvp(parsedRedirectArgs[0], parsedRedirectArgs);
			err_syserr("cannot open %s for input", parsedRedirectArgs[0]);
			fflush(stdout);
			break;

		default: //parent
			while (wait(&status) != -1); //picks up dead children
			break;

		case -1:
			perror("fork");
	}

	return;

	
}
// Function where the piped system commands is executed
void execArgsRedirect(char** parsedArgs, char** parsedRedirectArgs)
{

  	int args = strlen(*parsedArgs);

	int saved_stdout;

	if (args != 2){
		printf("Usage: ./main <filename to redirect stdout to>\n");
		exit(1);
	}

	int targetFD = open(parsedRedirectArgs[0], O_WRONLY | O_CREAT , 0640);
	
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
	
	
	runcmd(targetFD, parsedArgs);
	close(targetFD);
	fflush(stdout);
	
	
	 dup2(saved_stdout, STDOUT_FILENO);
	 close(saved_stdout);
	return;
	
}


// Function to execute builtin commands
int ownCmdHandler(char** parsedArgs, int childStatus)
{
	int NoOfOwnCmds = 3, i, switchOwnArg = 0;
	char* ListOfOwnCmds[NoOfOwnCmds];
	char* homeDir;
	char* newDir;
	// int childStatus = 0;

	ListOfOwnCmds[0] = "exit";
	ListOfOwnCmds[1] = "cd";
	ListOfOwnCmds[2] = "#";


  //compares the first element of the parsed string (if parsedArgs is "ls -la" then parsedArgs[0] is only ls)
	for (i = 0; i < NoOfOwnCmds; i++) {
		if (strcmp(parsedArgs[0], ListOfOwnCmds[i]) == 0) {
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
		
		  if (parsedArgs[1] == NULL)
		  {
			chdir(homeDir);
			return 1;
		  }
		 else 
		  	newDir = parsedArgs[1];
		  	chdir(newDir);
			return 1;
	case 3:
		printf("\r");
		fflush(stdout);
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

	i = strstr(str, "&");
	if (i != NULL)
	{
	 //	printf("found &");
		return 1;
	}
	else 
	//	printf("no &");
		return 0;
	
	
}
// function for finding pipe
int parseRedirect(char* str, char** strpiped)
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
int parseSpace(char* str, char** parsedArgs, int childStatus)
{
	int i;


	for (i = 0; i < MAXLIST; i++) {
		parsedArgs[i] = strsep(&str, " ");

		
		if (parsedArgs[i] == NULL)
			break;
		if (strstr(parsedArgs[i],"$$") != NULL)
		{
			parsedArgs[i] = strsep(&parsedArgs[i], "$$");
			sprintf(parsedArgs[i], "%d", getpid());
		}
		if (strlen(parsedArgs[i]) == 0)
			i--;
	}
  if (ownCmdHandler(parsedArgs, childStatus))
		return 0;
	else
		return 1;
}

int processString(char* str, char** parsedArgs, char** parsedRedirectArgs, int childStatus)
{

	char* strpiped[2];
	int redirect = 0;
	int background = 0;

	redirect = parseRedirect(str, strpiped);
	background = findBackground(str, strpiped);

	if (background)
	{
		printf("background char found\n");
		fflush(stdout);
		parseSpace(strpiped[0], parsedArgs, childStatus);
	}

	if (redirect) 
	{
		parseSpace(strpiped[0], parsedArgs, childStatus);
		parseSpace(strpiped[1], parsedRedirectArgs, childStatus);

	}else {
		parseSpace(str, parsedArgs, childStatus);
	}
//# TODO look into why ownCmdHandler called in processString and parseSpace
	if (ownCmdHandler(parsedArgs, childStatus))
		return 0;


	else
		return 1 + piped;
}


