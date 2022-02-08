#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include "main.h"

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

int BACKGROUND_ALLOWED = 0;
pid_t spawnpid = -5;

void  handle_SIGTSTP(int signo) 
{
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
  

int main()
{
	char inputString[MAXCOM], *parsedArgs[MAXLIST];
	char* parsedArgsPiped[MAXLIST];
	int execFlag = 0;
	
	int ret;
	char *empty = {"\r"};
	int i;
	int comp;
	int childStatus;
	int childPid;

	
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};

	SIGINT_action.sa_handler = handle_SIGINT;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
	sigaction(SIGINT, &SIGINT_action, NULL);

	SIGTSTP_action.sa_handler = handle_SIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	// ignore_action.sa_handler = SIG_IGN;

	// sigaction(SIGTERM, &ignore_action.sa_mask);
	// sigaction(SIGHUP, &ignore_action.sa_mask);
	// sigaction(SIGQUIT, &ignore_action.sa_mask);

	pid_t parentPid = getpid();
	
	printf("this process pid is %d\n", parentPid);
	// fflush(stdout);
	// pause();

	while (1) {
		// signal(SIGINT, handle_sigint);
		// signal(SIGTSTP, sighandler);
		// print shell line
		printPrompt();
		// take input
		takeInput(inputString);
    // check if inputString is NULL (if so continue)
	if (strlen(inputString) == 0){
		continue;
	}
   

    size_t s = strlen(inputString);

    
    execFlag = processString(inputString,
		parsedArgs, parsedArgsPiped, childStatus);
    
	
		// execflag returns zero if there is no command
		// or it is a builtin command,
		// 1 if it is a simple command
		// 2 if it is including a pipe.
    
		// execute
		int num;
		num = strcmp(inputString, "status");
		if (num == 0)
		{
			printf("exit status %d\n", childStatus);
			continue;
		}
	
		if (execFlag == 1){
			
			childStatus = execArgs(parsedArgs);
		
		}
		if (execFlag == 2)
			execArgsPiped(parsedArgs, parsedArgsPiped);
    
    
	}
	return 0;
}
