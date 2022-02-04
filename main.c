#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "main.h"

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported


// Citations for the following program:
// Date: 1/28/2022
// Several sections of main.c and main.h written 
// for this program are based on and/or adapted in part from the example programs provided at:
// Source URL: 
//  https://developerweb.net/viewtopic.php?id=3934// helped me in finding the largest file
//  https://www.geeksforgeeks.org/c-program-count-number-lines-file // helped me to find out the count lines in a file.
//  Getting File and Directory Meta-Data from Exploration: Directories // helped me in finding the largest file
//  Explorations and also https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c // in processFile function
//  I referenced the example code listed in the assignment 1 page when working on the parsing the files into structs.
//  https://stackoverflow.com/questions/22949500/how-to-create-file-inside-a-directory-using-c // helped with getting the files in the right location

//  https://www.geeksforgeeks.org/making-linux-shell-c/ // getting started with printing args to stdout
// https://www.geeksforgeeks.org/how-to-build-your-own-commands-in-linux/?ref=rp //maybe useful for creating own commands?
// https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf 
// https://www.cs.cornell.edu/courses/cs414/2004su/homework/shell/shell.html 
// https://danishpraka.sh/2018/01/15/write-a-shell.html 
// Brewster lectures on YouTube: https://www.youtube.com/watch?v=1R9h-H2UnLs&list=PL0VYt36OaaJll8G0-0xrqaJW60I-5RXdW&index=17


// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")


int main()
{
	char inputString[MAXCOM], *parsedArgs[MAXLIST];
	char* parsedArgsPiped[MAXLIST];
	int execFlag = 0;
	init_shell();

	while (1) {
		// print shell line
		printDir();
		// take input
		if (takeInput(inputString))
			continue;
		// process
		execFlag = processString(inputString,
		parsedArgs, parsedArgsPiped);
		// execflag returns zero if there is no command
		// or it is a builtin command,
		// 1 if it is a simple command
		// 2 if it is including a pipe.

		// execute
		if (execFlag == 1)
			executeCommands(parsedArgs);

		if (execFlag == 2)
			execArgsPiped(parsedArgs, parsedArgsPiped);
	}
	return 0;
}
