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
  int status_code = 0;
  int ret;
  char *comment = {"#"};
  int i;
  int comp;
  int exit_status = 0;
  // char *return = {"\r"};

	while (1) {
		// print shell line
		printDir();
		// take input
		// if (takeInput(inputString))
		// 	continue;
   
		// process
    if (takeInput(inputString))
			continue;
   
   

    ret = strcmp(inputString, comment);
    //checks to see if input is a comment 
    if (ret == 0)
    {
      continue;
    }

   


    size_t s = strlen(inputString);

    // for (i = 0; i < s; ++i)
    // {
    //   comp = strcmp(inputString, "<");
    //   if (comp == 0)
    //   {
    //     printf("call a < redirect\n");
    //   } 
    // }
    // printf("no < found\n");

    // for (i = 0; i < s; ++i)
    // {
    //   comp = strcmp(inputString, ">");
    //   if (comp == 0)
    //   {
    //     printf("call a > redirect\n");
    //   } 
    // }
    // printf("no > found\n");
    
    execFlag = processString(inputString,
		parsedArgs, parsedArgsPiped);
    
		// execFlag = parseSpace(inputString,
		// parsedArgs);
		// execflag returns zero if there is no command
		// or it is a builtin command,
		// 1 if it is a simple command
		// 2 if it is including a pipe.
    
		// execute
		if (execFlag == 1)
			execArgs(parsedArgs);
      
		if (execFlag == 2)
			execArgsPiped(parsedArgs, parsedArgsPiped);
    
    
	}
	return 0;
}
