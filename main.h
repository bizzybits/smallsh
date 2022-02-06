#define _GNU_SOURCE
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <fcntl.h>

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

// Greeting shell during startup
void init_shell()
{
	clear();
	printf("\n\n\n\n******************"
		"************************");
	printf("\n\n\n\t****MY SHELL****");
	printf("\n\n\t-USE AT YOUR OWN RISK-");
	printf("\n\n\n\n*******************"
		"***********************");
	char* username = getenv("USER");
	printf("\n\n\nUSER is: @%s", username);
	printf("\n");
	sleep(1);
	clear();
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
			printf("\nCould not execute command..\n");
		}
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

void runcmd(int fd, char ** parsedpipe){

	int saved_stdout;
	saved_stdout = dup(STDOUT_FILENO);
	int status;

	switch (fork()){
		case 0: //child
			dup2(fd, 1); //fd becomes stdout
			execvp(parsedpipe[0], parsedpipe);
			perror(parsedpipe[0]);
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
	printf("The file descriptor for targetFD is %d\n", targetFD);

	// Use dup2 to point FD 1, i.e., standard output to targetFD
	saved_stdout = dup(STDOUT_FILENO);
	int result = dup2(targetFD, 1);
	if (result == -1) {
		perror("dup2"); 
		exit(2); 
	}
	// Now whatever we write to standard out will be written to targetFD
	printf("All of this is being written to the file using printf\n"); 
	
	
	runcmd(targetFD, parsed);
	close(targetFD);
	fflush(stdout);
	
	
	 dup2(saved_stdout, STDOUT_FILENO);
	 close(saved_stdout);
	return;
	
}



void g4g(char ** parsedpipe, char ** parsed){
	// 0 is read end, 1 is write end
	int pipefd[2];
	pid_t p1, p2;


	p1 = fork();

	if (p1 < 0){
		printf("could not fork p1");
	}

	if (p1 == 0){
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		if (execvp(parsed[0], parsed) < 0) {
			printf("\nCould not execute command 1..");
			exit(0);
		}
	} else {
		// Parent executing
		p2 = fork();
		if (p2 < 0) {
			printf("\nCould not fork");
			return;
		}
		// Child 2 executing..
		// It only needs to read at the read end
		if (p2 == 0) {
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
			if (execvp(parsed[0], parsedpipe) < 0) {
				printf("\nCould not execute command 2..");
				exit(0);
			}
		} else {
			// parent executing, waiting for two children
			wait(NULL);
			wait(NULL);
		}
	}
 }
	
// Help command builtin
void openHelp()
{
	puts("\n***WELCOME TO MY SHELL HELP***"
		"\nCopyright @ Suprotik Dey"
		"\n-Use the shell at your own risk..."
		"\nList of Commands supported:"
		"\n>cd"
		"\n>ls"
		"\n>exit"
		"\n>all other general commands available in UNIX shell"
		"\n>pipe handling"
		"\n>improper space handling");

	return;
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

	i = strstr(str, "&");
	if (i != NULL)
	{
		printf("found &");
	}
	else 
		printf("no &");
	
	
	
}
// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
	int i;
	for (i = 0; i < 2; i++) {
		strpiped[i] = strsep(&str, "<");
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
int parseSpace(char* str, char** parsed, int childStatus)
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

int processString(char* str, char** parsed, char** parsedpipe, int childStatus)
{

	char* strpiped[2];
	int piped = 0;
	int background = 0;

	piped = parsePipe(str, strpiped);
	background = findBackground(str, strpiped);

	if (background)
	{
		printf("background char found\n");
		parseSpace(strpiped[0], parsed, childStatus);
	}

	if (piped) 
	{
		parseSpace(strpiped[0], parsed, childStatus);
		parseSpace(strpiped[1], parsedpipe, childStatus);

	}else {
		parseSpace(str, parsed, childStatus);
	}

	if (ownCmdHandler(parsed, childStatus))
		return 0;


	else
		return 1 + piped;
}

// struct child *createChild(char* str, char** parsed, char** parsedpipe)
// {

// 	struct child *currChild = malloc(sizeof(struct child));

// 	currChild->command =  calloc(strlen(token) + 1, sizeof(char));
// 	strcpy(currChild->command, parsed[0]);

// 	currChild->pid = calloc(strlen(token) + 1, sizeof(int));
// 	//get pid

// 	currChild->status = calloc(strlen(token) + 1, sizeof(int));
// 	//get exit exstatus

// 	currChild->next = NULL;

// 	currChild->prev = currChild;

// }



// struct child {
//   char *command;
//   int pid;
//   int status;
//   struct child *next;
//   struct child *prev;
// };


//https://stackoverflow.com/questions/13636252/c-minishell-adding-pipelines
typedef void (*SigHandler)(int signum);

static void sigchld_status(void)
{
    const char *handling = "Handler";
    SigHandler sigchld = signal(SIGCHLD, SIG_IGN);
    signal(SIGCHLD, sigchld);
    if (sigchld == SIG_IGN)
        handling = "Ignored";
    else if (sigchld == SIG_DFL)
        handling = "Default";
    printf("SIGCHLD set to %s\n", handling);
}

