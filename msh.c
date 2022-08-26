/*
	Name: Mahmoud Natsheh
	ID: 1001860023
*/

// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports ten arguments

int main()
{

	char * command_string = (char*) malloc( MAX_COMMAND_SIZE );

	// Array history_list will save all the commands that the user inputs
	// up until the 15th user entry, it will serve as our history array
	char* history_list[15];
	// counts/keeps track of how many things are in the history_list array
	int history_count = 0;
	// keeps track of where in the array we are adding new entries
	int history_monitor = 0;

	// Array pid_list will save all the pids of child processes
	// up until the 15th entry, it will serve as our pid array
	int pid_list[15];
	// counts/keeps track of how many things are in the pid_list array
	int pid_count = 0;
	// keeps track of where in the array we are adding new entries
	int pid_monitor = 0;

	// make room in the array history_list in order to put entries into
	// the array
	int i = 0;
	for(i = 0; i < 15; i++)
	{
		history_list[i] = (char*) malloc( MAX_COMMAND_SIZE );
	}

	//This will allow the program to continuosly loop until
	//the command exit or quit has been enetered
	while( 1 )
	{
		// Print out the msh prompt
		printf ("msh> ");

		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input
		while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );
		/* Parse input */
		char *token[MAX_NUM_ARGUMENTS];

		int token_count = 0;

		// Pointer to point to the token
		// parsed by strsep
		char *argument_ptr;

		char *working_string  = strdup( command_string );

		// we are going to move the working_string pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *head_ptr = working_string;

		// Tokenize the input strings with whitespace used as the delimiter
		while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
	              (token_count<MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
			if( strlen( token[token_count] ) == 0 )
			{
				token[token_count] = NULL;
			}
			token_count++;
		}

		//If the user inputs a blank line (clicks enter without putting in a command)
		//then the program will do nothing and loop back to the beginning of the while
		//inorder to allow the user to continue using the msh
		if(token[0] == NULL)
		{

		}
		else
		{
			// checking if the user inputed an "!", this will signal to us that we need 
			// to pull up previous user commands in the history if the command was inputted
			char* check = strstr(token[0], "!");
			if(check)
			{
				//we tokenize the user input in order to get the number the user 
				//inputed before the "!" command in order to figure out which command from 
				//history the user wants us to execute
				char* tok = strtok(token[0], "!");
				//in order to avoid errors we will check if the user put a number infront of
				//the "!" command, if the user did not enter a number before it then we
				// can't execute the command so we can send an error message to user
				if(tok == NULL)
				{
					printf("%s: Command not found.\n", token[0]);
				}
				else
				{
					// this will store the index of the history array that we need to pull up
					int num = atoi(tok);

					// this will input the history command that the user requested from the history 
					// into our string that we are dealing with in order to later execute the command
					// only if the number we recived from the user is a valid index of our history array
					if(num <= history_count && num < 15 && num >= 0)
					{
						strcpy(token[0], history_list[num]);
					}
					else
					{
						printf("Command not in history.\n");
						strcpy(token[0], "!");
					}
				}
			}
			// if the user only inputed "!" with no number following it we will not add it to the history array
			if(strstr(token[0], "!"))
			{

			}
			// This is adding the command the user inputed to the history array, since we are only allowed 15 
			// entries in the array we will check if our spot in the array is less than 15, if it is then we
			// will add the command to our current spot in the array, if it is not, then we will use the next
			// else if statment and loop to the start of the array and start adding entries from the start
			else if(history_monitor < 15)
			{
				strcpy(history_list[history_monitor], token[0]);
				history_count++;
				history_monitor++;
			}
			else if(history_monitor >= 15)
			{
				history_monitor = 0;
				strcpy(history_list[history_monitor], token[0]);
			}

			// if the user only inputed "!" with no number following it we will not execute the command and
			// instead we will end the current process and let it loop back to the start of the while loop
			// where we will reoutput to the user the msh> prompt 
			if(strstr(token[0], "!"))
			{

			}
			// if the user inputs the command quit or exit we will end the process by freeing all memory 
			else if(strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
			{
				free( head_ptr );
				return 0;
			}
			// if the user inputs the command "cd" we will change directories using the direcotory the user
			// inputs, but if the user writes cd with no directory name following the cd command then we 
			// will remain in our current directory
			else if(strcmp(token[0], "cd") == 0)
			{
				if(token[1] == NULL)
				{

				}
				else
				{
					chdir(token[1]);
				}
			}
			// if the user inputs the command "showpids" we will output up until the 15 most recent pids
			// the user used, this is done by printing everything in the pid array
			else if(strcmp(token[0], "showpids") == 0)
			{
				for(i = 0; i < pid_count && i < 15; i++)
				{
					printf("PID %d: %d\n", i, pid_list[i]);
				}
			}
			// if the user inputs the command "history" we will output up until the 15 most recent history
			// commands the user used, this is done by printing everything in the history array
			else if(strcmp(token[0], "history") == 0)
			{
				for(i = 0; i < history_count && i < 15; i++)
				{
					printf("%d: %s\n", i, history_list[i]);
				}
			}
			// if the command that the user inputed was none of the previous commands checked for then we
			// execute the command in the shell, this is done by forking a new process to execute the user
			// command 
			else
			{
				pid_t pid = fork();
				// this child process will ececute the user command
				if(pid == 0)
				{
					int ret = execvp(token[0], &token[0]);
					// if the command does not exist the execution will fail and we will inform the user
					// their command is invalid
					if(ret == -1)
					{
						printf("%s: Command not found.\n", token[0]);
						return 0;
					}
				}
				// if the child process fails to fork then we will end the process and return to the 
				// parent process
				else if(pid == -1)
				{
					return 0;
				}
				// the parent process will wait until it hears back from the child process and save the 
				// child process pid into the pid array for future use
				else
				{
					int status;
					wait(&status);
					// This is adding the pid of the child process into the pid array, since we are only allowed 15 
					// entries in the array we will check if our spot in the array is less than 15, if it is then we
					// will add the pid to our current spot in the array, if it is not, then we will use the next
					// else if statment and loop to the start of the array and start adding pids from the start
					if(pid_monitor < 15)
					{
						pid_list[pid_monitor] = pid;
						pid_count++;
						pid_monitor++;
					}
					else if(pid_monitor >= 15)
					{
						pid_monitor = 0;
						pid_list[pid_monitor] = pid;
					}
				}
			}
		}
	}
	return 0;
	// e2520ca2-76f3-11ec-90d6-0242ac120003
}
