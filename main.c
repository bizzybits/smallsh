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

/*
Citations for the following program:
Date: 2/7/2022

Several sections of main.c and main.h written for this program are based on 
and/or adapted in part from the example programs and concepts in our class Explorations 
for Modules 4 and 5 as well as additional resources provided at:

Source URL: 
https://developerweb.net/viewtopic.php?id=3934// helped me in finding the largest file
https://www.geeksforgeeks.org/c-program-count-number-lines-file // helped me to find out the count lines in a file.
https://stackoverflow.com/questions/22949500/how-to-create-file-inside-a-directory-using-c // helped with getting the files in the right location
https://www.geeksforgeeks.org/making-linux-shell-c/ // getting started with printing args to stdout and example of built in command functions 
https://www.geeksforgeeks.org/how-to-build-your-own-commands-in-linux/?ref=rp //maybe useful for creating unique commands
https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf // not C, but helpful to look at
https://www.cs.cornell.edu/courses/cs414/2004su/homework/shell/shell.html // if i were to implement with a struct format
https://danishpraka.sh/2018/01/15/write-a-shell.html // 
https://www.youtube.com/watch?v=1R9h-H2UnLs&list=PL0VYt36OaaJll8G0-0xrqaJW60I-5RXdW&index=17 // Prof. Brewster lectures on YouTube
https://stackoverflow.com/questions/13636252/c-minishell-adding-pipelines 
https://canvas.oregonstate.edu/courses/1884946/pages/exploration-process-api-monitoring-child-processes?module_item_id=21835973 // status
https://canvas.oregonstate.edu/courses/1884946/pages/exploration-process-api-executing-a-new-program?module_item_id=21835974 // new process
https://stackoverflow.com/questions/40077022/c-replace-no-such-file-or-directory-message // custom error message to replace perror
https://www.geeksforgeeks.org/signals-c-language/ user defined signal handlers
https://stackoverflow.com/questions/13636252/c-minishell-adding-pipelines // i don't think i used this but it was in my browser history list
https://www.geeksforgeeks.org/c-program-not-suspend-ctrlz-pressed/ signal handlers
*/

int main()
{
	char inputString[MAXCHARS], *parsedArgs[MAXARGS];
	//char* parsedArgsPiped[MAXARGS];
	int execFlag = 0;
  	int ret;
  	int i;
  	int childStatus;
	int bgFlag;
	int bgToggle = 0;
	int sigintFlag = 0;
	int sigtstpFlag = 0;

	while (1) {
	
		// print shell line
		printPrompt();
		// take input
		takeInput(inputString);
    	// check if inputString is NULL (if so continue)
		if (strlen(inputString) == 0)
		{
			continue;
		}

		int pid = getpid();
		printf("pid of main loop is %d\n", pid);
		fflush(stdout);

    	size_t s = strlen(inputString);
	 	execFlag = processString(inputString, parsedArgs, childStatus);


		int num;
		num = strcmp(inputString, "status");
		
		if (num == 0)
		{
			printf("exit status %d\n", childStatus);
			continue;
		}

		if (execFlag == 1)
		{
		childStatus = execArgs(parsedArgs, bgFlag, sigintFlag, sigtstpFlag);
		}
		else 
			printf("bye");
	}
	return 0;


}