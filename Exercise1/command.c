#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "command.h"

#define MAX_CMD_COUNT 50
#define MAX_CMD_LEN 25


	//TODO FUNCTION COMMENT
/*
 * PURPOSE: Interprets input and creates a command
 * INPUTS:
 *      User input to analyze, input
 *      Command to populate, cmd
 * RETURN:
 *      If there is an error parsing input, return false.
 *      Else, return true.
 **/
bool parse_user_input (const char* input, Commands_t** cmd) {
	
	//TODO ERROR CHECK INCOMING PARAMETERS
        if(!input || !cmd){
                perror("parse_user_input: bad input\n");
                return false;
        }

	char *string = strdup(input);
	
	*cmd = calloc (1,sizeof(Commands_t));
	(*cmd)->cmds = calloc(MAX_CMD_COUNT,sizeof(char*));

	unsigned int i = 0;
	char *token;
	token = strtok(string, " \n");
	for (; token != NULL && i < MAX_CMD_COUNT; ++i) {
		(*cmd)->cmds[i] = calloc(MAX_CMD_LEN,sizeof(char));
		if (!(*cmd)->cmds[i]) {
			perror("Allocation Error\n");
			return false;
		}	
		strncpy((*cmd)->cmds[i],token, strlen(token) + 1);
		(*cmd)->num_cmds++;
		token = strtok(NULL, " \n");
	}
	free(string);
	return true;
}// end parse_user_input

	//TODO FUNCTION COMMENT
/*
 * PURPOSE:
 * INPUTS:
 * RETURN:
 **/
void destroy_commands(Commands_t** cmd) {

	//TODO ERROR CHECK INCOMING PARAMETERS
        if(!cmd){
                perror("destroy_commands: bad input");
                return;
        }

	for (int i = 0; i < (*cmd)->num_cmds; ++i) {
                if((*cmd)->cmds[i]){
                        free((*cmd)->cmds[i]);
                }
	}
	free((*cmd)->cmds);
	free((*cmd));
	*cmd = NULL;
}// end destroy_commands

