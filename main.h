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

int bgToggle;
int sigtstpFlag;
int sigintFlag;


void handle_SIGTSTP(int signo){
	if (bgToggle == 0){
		char* message = "\nEntering foreground-only mode (& is now ignored)\n";
		bgToggle = 1;
		write(STDOUT_FILENO, message, 50);
	}
	else{
		char* message = "\nExiting foreground-only mode\n";
		bgToggle = 0;
		write(STDOUT_FILENO, message, 30);
	}
	sigtstpFlag = 1;
	fflush(stdout);
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

// Function to print shell prompt.
void printPrompt()
{
	printf(": ");
}

// Function to print last process's exit status.
void printStatus(int childStatus)
{
	printf("exit value %d\n", childStatus);
}

// Function where the system command is executed
int execArgs(char** parsed, int bgFlag, int sigintFlag, int sigtstpFlag)
{
	int redirect;
	int fd;
	int i;
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};
	pid_t pid;
	int saved_stdout;
	saved_stdout = dup(STDOUT_FILENO);
	int status;
	int childStatus;

	//ignore sigint signals
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
	sigaction(SIGINT, &SIGINT_action, NULL);

	//custom SIGTSTP handler
	SIGTSTP_action.sa_handler = handle_SIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
  	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	pid = getpid();


	// Forking a child

	pid = fork();
	switch(pid){

	case -1:
		printf("\nFailed forking child..\n");
		return 1;
	case 0:
		if (bgFlag == 0){
			SIGINT_action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &SIGINT_action, NULL);
		}
		for (i = 1; i < strlen(*parsed); i++){
			if (bgFlag == 1) {	
				redirect = 1;
				fd = open("/dev/null", O_RDONLY);
				int result = dup2(fd, STDIN_FILENO);
				if (result == -1) { 
					perror("source open()"); 
					exit(1); 
				}
			}
			// input file direction
			if ((strcmp(parsed[i], "<") == 0)) {
				redirect = 1;
				
				fd = open(parsed[i + 1], O_RDONLY);
				int result = dup2(fd, STDIN_FILENO);
				if (result == -1) { 
					perror("source open()"); 
					exit(1); 
				}
			}
			// output file direction
			if ((strcmp(parsed[i], ">") == 0))  {
				redirect = 1;
				fd = open(parsed[i + 1], O_CREAT | O_RDWR | O_TRUNC, 0640);
				int result = dup2(fd, STDOUT_FILENO); //should have made a function for all this repeated code 
				if (result == -1){
					perror("source open()"); 
					exit(1); 
				}
			}
		}
		//truncate arguments in preperation for exec and close FD
		if (redirect == 1){
			close(fd);
			for(i = 1; i < strlen(*parsed); i++)
				*parsed--;
		}

		//pass trunceted arguments to execvp
		if (execvp(parsed[0], parsed) && sigtstpFlag != 1 && sigintFlag != 1){
			fprintf(stderr, "Cannot execute %s\n", parsed[0]);
			fflush(stdout);
			exit(1);
		}
		break;

	default:
		if (bgFlag == 1){
			printf("background pid is %d\n", pid);
			fflush(stdout);
		}
		else { 
			//wait for child to terminate
			waitpid(pid, &status, 0);
			if(sigtstpFlag != 1){
				//check that child is dead
				if (WIFSIGNALED(status) == 1 && WTERMSIG(status) != 0){
					printf("terminated by signal %d\n", WTERMSIG(status));
					fflush(stdout);
				}
				while (pid != -1){
					//re-fetch pid
					pid = waitpid(-1, &status, WNOHANG);	
					//print after results killed
					if (WIFEXITED(status) != 0 && pid > 0){
						printf("background pid %d is done: exit value %d\n", pid, WEXITSTATUS(status));
						fflush(stdout);
					}
					else if (WIFSIGNALED(status) != 0 && pid > 0 ){
						printf("background pid %d is done: terminated by signal %d\n", pid, WTERMSIG(status));
						fflush(stdout);
					}
				}
			}
		}
		break;
		
	}
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



// This function parses command arguments from the input string and adds them
// to an array called parsed.
int parseSpace(char* str, char** parsed, int childStatus)
{
	int bgFlag = 0;
	int bgToggle;
	int i;

	for (i = 0; i < MAXCHARS; i++) {
		parsed[i] = strsep(&str, " ");

		
		if (parsed[i] == NULL)
			break;
		if (strstr(parsed[i],"$$") != NULL)
		{
			
			char * temp = strdup(parsed[i]);
			strcpy(temp, "%d");
			sprintf(parsed[i], temp, getpid());
			free(temp);
		}

	
		if (strcmp(parsed[i], "&") == 0) {
			i--;
			parsed[i] = NULL;
			bgFlag = 1;
			if (bgToggle == 0){
				bgFlag = 1;
				(*parsed)++;
			}
			return bgFlag;
		}	

		if (strlen(parsed[i]) == 0)
		i--;		
	}
  	if (ownCmdHandler(parsed, childStatus))
	  {
		  return 1;
	  }
	else
		return 0;
	

}

int processString(char* str, char** parsed, int childStatus)
{

	char* strpiped[2];

	parseSpace(str, parsed, childStatus);

	if (ownCmdHandler(parsed, childStatus))
		return 0;
	else
		return 1;
}

