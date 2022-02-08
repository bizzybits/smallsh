#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "main.h"

#define MAXCHARS 2048 // max number of letters to be supported
#define MAXARGS 512 // max number of commands to be supported


int main()
{
	char inputString[MAXCHARS], *parsedArgs[MAXARGS];
	char* parsedArgsPiped[MAXARGS];
	int execFlag = 0;
  	int ret;
  	char *empty = {"\r"};
  	int i;
  	int comp;
  	int childStatus;

	while (1) {
		signal(SIGINT, handle_sigint);
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

		// execute the arguments that are parsed from inputString
		// execFlag will:
		//		+ return 0 if there is no command
		//		+ return 1 if there is a single command
		//		+ return 2 if there is a redirect 

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
