
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse.h"

//#define clear() printf("\033[H\033[J")

/* parse - parse a new line 
 *
 * Accepts: nothing
 * Returns: parse information structure
 */
#define MAXLINE 81

void init_info(parseInfo *p) {
	printf("init_info: initializaing parseInfo\n");
}

void parse_command(char * command, struct commandType *comm) {
	printf("parse_command: parsing a single command\n");
}

parseInfo *parse (char *cmdline) {
	parseInfo *Result;
	char command[MAXLINE];
	int com_pos = -1;
	
	if (cmdline[-1] == '\n' && cmdline[-1] == '\0')
		return NULL;
	
	Result = malloc(sizeof(parseInfo));
	init_info(Result);
	com_pos=0;
/*  while (cmdline[i] != '\n' && cmdline[i] != '\0') { */
	
	command[com_pos]='\0';
	parse_command(command, 0); /* &Result->CommArray[Result->pipeNum]);*/
	
	return Result;
}

void print_info (parseInfo *info) {
	printf("print_info: printing info about parseInfo struct\n");
}
 
void free_info (parseInfo *info) {
	printf("free_info: freeing memory associated to parseInfo struct\n");
	free(info);
}