#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "main.h"

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")



int main()
{
	char inputString[MAXCOM], *parsedArgs[MAXLIST];
	char* parsedArgsPiped[MAXLIST];
	int execFlag = 0;
	init_shell();
  int ret;
  char *empty = {"\r"};
  int i;
  int comp;


  
  // char *return = {"\r"};

	while (1) {
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
		parsedArgs, parsedArgsPiped);
    
	
		// execflag returns zero if there is no command
		// or it is a builtin command,
		// 1 if it is a simple command
		// 2 if it is including a pipe.
    
		// execute


	   
		if (execFlag == 1){
			int childStatus;
			childStatus = execArgs(parsedArgs);

			printf("status of arg is %d\n", childStatus);
		}
		if (execFlag == 2)
			execArgsPiped(parsedArgs, parsedArgsPiped);
    
    
	}
	return 0;
}
