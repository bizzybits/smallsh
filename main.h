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
void printDir()
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\nDir: %s\n", cwd);
}

// Function where the system command is executed
void execArgs(char** parsed)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1) {
		printf("\nFailed forking child..");
		return;
	} else if (pid == 0) {
		if (execvp(parsed[0], parsed) < 0) {
			printf("\nCould not execute command..");
		}
		exit(0);
	} else {
		// waiting for child to terminate
		wait(NULL);
		return;
	}
}

// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{


//   printf("*parsed = %s\n", *parsed);
//   printf("parsedpipe[0] = %s\n)", parsedpipe[0]);
  	int args = strlen(*parsed);
 // printf("strlen(*parsed = %d\n", args);

	if (args != 2){
		printf("Usage: ./main <filename to redirect stdout to>\n");
		exit(1);
	}

	int targetFD = open(parsedpipe[0], O_WRONLY | O_CREATE , 0640);
	
	if (targetFD == -1) {
		perror("open()");
		exit(1);
	}
	fcntl(targetFD, F_SETFD, FD_CLOEXEC);
	// Currently printf writes to the terminal
	printf("The file descriptor for targetFD is %d\n", targetFD);

	// Use dup2 to point FD 1, i.e., standard output to targetFD
	int result = dup2(targetFD, 1);
	if (result == -1) {
		perror("dup2"); 
		exit(2); 
	}
	// Now whatever we write to standard out will be written to targetFD
	printf("All of this is being written to the file using printf\n"); 
	
	
	runcmd(targetFD, parsed);
	fflush(targetFD);
	
	exit(0);
	
}

void runcmd(int fd, char ** parsed){

	
	int status;

	switch (fork()){
		case 0: //child
			dup2(fd, 1); //fd becomes stdout
			execvp(parsed[0], parsed);
			perror(parsed[0]);
			exit(1);

		default: //parent
			while (wait(&status) != -1); //picks up dead children
			break;

		case -1:
			perror("fork");
	}

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
	// // Open source file
	// int sourceFD = open(parsedpipe[0], O_RDONLY | O_WRONLY | O_TRUNC);
	// if (sourceFD == -1) { 
	// 	perror("source open()"); 
	// 	exit(1); 
	// }
	// // Written to terminal
	// printf("sourceFD == %d\n", sourceFD); 

	// // Redirect stdin to source file
	// int result = dup2(sourceFD, 0);
	// if (result == -1) { 
	// 	perror("source dup2()"); 
	// 	exit(2); 
	// }

	// // Open target file
	// int targetFD = open(parsedpipe[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	// if (targetFD == -1) { 
	// 	perror("target open()"); 
	// 	exit(1); 
	// }

	// printf("targetFD == %d\n", targetFD); // Written to terminal
  
	// // Redirect stdout to target file
	// result = dup2(targetFD, 1);
	// if (result == -1) { 
	// 	perror("target dup2()"); 
	// 	exit(2); 
	// }
	// // Run the sort program using execlp.
	// // The stdin and stdout are pointing to files
	// execlp(parsed[0], parsed, NULL);
	
	
	// return;

			
 //}

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
int ownCmdHandler(char** parsed)
{
	int NoOfOwnCmds = 4, i, switchOwnArg = 0;
	char* ListOfOwnCmds[NoOfOwnCmds];
	char* username;

	ListOfOwnCmds[0] = "exit";
	ListOfOwnCmds[1] = "cd";
	ListOfOwnCmds[2] = "status";
	ListOfOwnCmds[3] = "hello";

  //compares the first element of the parsed string (if parsed is "ls -la" then parsed[0] is only ls)
	for (i = 0; i < NoOfOwnCmds; i++) {
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
			switchOwnArg = i + 1; //if it matches it will be 1, 2, 3, or 4
			break;
		}
	}

	switch (switchOwnArg) {
	case 1:
		printf("\nGoodbye\n");
		exit(0);
	case 2: //need to check if there is no parse[1], 
          // then need to set as if parse[1] = "~" or $HOME
		chdir(parsed[1]); //parsed[0] is cd, parsed[1] is destination directory 
		return 1;
	case 3:
		printf("status will print now");
		return 1;
	case 4:
		// username = getenv("USER");
		// printf("\nHello %s.\nMind that this is "
		// 	"not a place to play around."
		// 	"\nUse help to know more..\n",
		// 	username);
		return 1;
	default:
		break;
	}

	return 0;
}

// function for finding pipe
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

// function for parsing command words
int parseSpace(char* str, char** parsed)
{
	int i;

	for (i = 0; i < MAXLIST; i++) {
		parsed[i] = strsep(&str, " ");

		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
  if (ownCmdHandler(parsed))
		return 0;
	else
		return 1;
}

int processString(char* str, char** parsed, char** parsedpipe)
{

	char* strpiped[2];
	int piped = 0;

	piped = parsePipe(str, strpiped);

	if (piped) {
		parseSpace(strpiped[0], parsed);
		parseSpace(strpiped[1], parsedpipe);

	} else {

		parseSpace(str, parsed);
	}

	if (ownCmdHandler(parsed))
		return 0;
	else
		return 1 + piped;
}

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

int status(int pid)
{
	 int status;
     
    waitpid(pid, &status, 0);
 
    if ( WIFEXITED(status) )
    {
        int exit_status = WEXITSTATUS(status);       
        printf("Exit status of the child was %d\n",
                                     exit_status);
    }

	printf("no status found\n");
	return 0;
}