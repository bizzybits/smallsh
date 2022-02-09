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

#define MAXCHARS 2048 // max number of letters to be supported
#define MAXARGS 512 // max number of commands to be supported
#define MAX_VAR_NUM 10
#define PIPE_MAX_NUM 10
#define FILE_MAX_SIZE 40
#define MAXFILE 81

void handle_sigint(int signo)
{
    char* message = "terminated by signal 2\n";
	write(STDOUT_FILENO, message, 25);
	sleep(3);
}

// To implement: 
// SIGTSTP flag

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

// Function to print shell prompt.
void printPrompt()
{
	printf(": ");
}

// Function to print last process's exit status.
void printStatus(int childStatus)
{
	printf("exit valud %d\n", childStatus);
}

// Function where the system command is executed
int execArgs(char** parsed)
{
	// Forking a child
	int childStatus;
	pid_t pid = fork();

	if (pid == -1) {
		printf("\nFailed forking child..\n");
		return 1;
	} else if (pid == 0) {
		if (execvp(parsed[0], parsed) < 0) {
			perror(parsed[0]);
			childStatus = 1;
		}
		return childStatus;
		exit(0);
	} else {
		// waiting for child to terminate
		pid = waitpid(pid, &childStatus, 0);
		if (WIFEXITED(childStatus))
		{
			if (childStatus == 0) //Child exited normally with status 0
			{
				return childStatus;
			}
			else 
				childStatus = 1; //Child exited normally with status other than 0
				return childStatus;
		}
		else 
		{
			return childStatus; //Child exited abnormally due to signal
		}
		wait(NULL);
	}
}

void err_syserr(const char *fmt, char * parsedpipe)
{
    int errnum = errno;
    if (errnum != 0)
        fprintf(stderr, "%d: %s\n", errnum, strerror(errnum));
    putc('\n', stderr);
    exit(EXIT_FAILURE);
}

void runcmd(int fd, char ** parsedpipe)
{
	int saved_stdout;
	saved_stdout = dup(STDOUT_FILENO);
	int status;

	switch (fork())
	{
		case 0: //child
			dup2(fd, 1); //fd becomes stdout
			execvp(parsedpipe[0], parsedpipe);
			err_syserr("cannot open %s for input", parsedpipe[0]);
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
void execArgsPiped(char** parsed, char** parsedpipe)
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
	// Use dup2 to point FD 1, i.e., standard output to targetFD
	saved_stdout = dup(STDOUT_FILENO);
	int result = dup2(targetFD, 1);
	if (result == -1) {
		perror("dup2"); 
		exit(2); 
	}
	// Now whatever we write to standard out will be written to targetFD
	
	runcmd(targetFD, parsed);
	close(targetFD);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);
	return;
	
}

// This function will execute custom functions
int ownCmdHandler(char** parsed, int childStatus)
{
	int NoOfOwnCmds = 3, i, switchOwnArg = 0;
	char* ListOfOwnCmds[NoOfOwnCmds];
	char* homeDir;
	char* newDir;

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

// This function is for finding background ("&") argument -- to be implemented.
int findBackground(char* str, char** strpiped)
{
	char *i;

	i = strstr(str, "&");
	if (i != NULL)
	{
		return 1; // & found
	}
	else 
		return 0; // no & found
}

// This function finds a redirect symbol.
int parsePipe(char* str, char** strpiped)
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

// This function parses command arguments from the input string and adds them
// to an array called parsed.
int parseSpace(char* str, char** parsed, int childStatus)
{
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

}

int processString(char* str, char** parsed, char** parsedpipe, int childStatus)
{

	char* strpiped[2];
	int piped = 0;
	int background = 0;

	piped = parsePipe(str, strpiped);
	background = findBackground(str, strpiped);

	if (background)
	{
		parseSpace(strpiped[0], parsed, childStatus);
	}

	if (piped) 
	{
		parseSpace(strpiped[0], parsed, childStatus);
		parseSpace(strpiped[1], parsedpipe, childStatus);

	}else {
		parseSpace(str, parsed, childStatus);
	}
//# TODO look into why ownCmdHandler called in processString and parseSpace
	if (ownCmdHandler(parsed, childStatus))
		return 0;
	else
		return 1 + piped;
}

