/********************************************************************
* Program Name: Assignment 3: smallsh
* Author: Patricia Booth
* OSU Email Address: boothpat@oregonstate.edu
* Course Number / Section: CS 344-400
* Due Date: 11/03/2020
* Last Modified: 10/24/2020
* Description: 
* Citations:
*
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Opening / Closing Directories
#include <dirent.h>
// getcwd() - Current working directory
#include <unistd.h>

#define MAX_CHAR 2048

/********************************************************************
* INSTRUCTIONS
*
* [1] THE COMMAND PROMPT ------------------------------------
*
* Use the colon : symbol as a prompt for each command line. 
* The general syntax of a command line is:
* command [arg1 arg2 ...] [< input_file] [> output_file] [&]
* …where items in square brackets are optional.
*
* * You can assume that a command is made up of words
*   separated by spaces.
* * The special symbols , and & are recognized, but they must be
* * surrounded by spaces like other words.
*
* * If the command is to be executed in the background, the last word 
* * must be &. If the & character appears anywhere else, just 
* * treat it as normal text.
*
* * If standard input or output is to be redirected, the > or < words
* * followed by a filename word must appear after all the arguments.
* * Input redirection can appear before or after output redirection.
*
* * Your shell does not need to support any quoting; so arguments with 
* * spaces inside them are not possible. We are also not implementing
* * the pipe "|" operator.
*
* * Your shell must support command lines with a maximum length of 2048 
* * characters, and a maximum of 512 arguments.
*
* * You do not need to do any error checking on the syntax
* * of the command line.
*
*
* [2] COMMENTS & BLANK LINES ------------------------------
*
* Your shell should allow blank lines and comments.
*
* Any line that begins with the # character is a comment line
* and should be ignored. Mid-line comments, such as the C-style //,
* will not be supported.
*
* A blank line (one without any commands) should also do nothing.
*
* Your shell should just re-prompt for another command when it
* receives either a blank line or a comment line.
*
*
* [3] EXPANSION OF VARIABLE $$ ------------------------------
*
* Your program must expand any instance of "$$" in a command into 
* the process ID of the smallsh itself. Your shell does not otherwise 
* perform variable expansion. 
*
*
* [4] BUILT-IN COMMANDS --------------------------------------
*
* Your shell will support three built-in commands: exit, cd, 
* and status. These three built-in commands are the only ones that 
* your shell will handle itself - all others are simply passed on 
* to a member of the exec() family of functions.
*
* You do not have to support input/output redirection for these
* built in commands
*
* These commands do not have to set any exit status.
*
* If the user tries to run one of these built-in commands in the 
* background with the & option, ignore that option and run the command 
* in the foreground anyway (i.e. don't display an error, just run the 
* command in the foreground).
*
* exit
* The exit command exits your shell. It takes no arguments. 
* When this command is run, your shell must kill any other processes 
* or jobs that your shell has started before it terminates itself.
*
* cd
* The cd command changes the working directory of smallsh.
* By itself - with no arguments - it changes to the directory 
* specified in the HOME environment variable
* This is typically not the location where smallsh was executed from,
* unless your shell executable is located in the HOME directory, 
* in which case these are the same.
* This command can also take one argument: the path of a directory to
* change to. Your cd command should support both absolute and relative
* paths.
*
* status
* The status command prints out either the exit status or the 
* terminating signal of the last foreground process ran by your shell.
* If this command is run before any foreground command is run, then it 
* should simply return the exit status 0.
* The three built-in shell commands do not count as foreground processes
* for the purposes of this built-in command - i.e., status should
* ignore built-in commands.
*********************************************************************/

/********************************************************************
* Function: printColon
* Receives:
* Returns: 
* Description: 
*********************************************************************/
void printColon() {
	printf(": ");
}

/********************************************************************
* Function: getString
* Receives: none
* Returns: Pointer to char
* Description: A utility function that is designed to get the user
*              string via memory allocated for pointer to char.
*              Warning: Allocates memory which must be freed at a
*                       later point.
* Citation: Stack Overflow question on getting string pointers via
*           https://stackoverflow.com/questions/14707427/
*********************************************************************/
char * getString() {
	char *stringVal;

	// Shell must support command lines up to 2048 characters
	stringVal = malloc(sizeof(char) * MAX_CHAR);

	//scanf("%s", stringVal);
	fgets(stringVal, MAX_CHAR, stdin);

	// Remove trailing newline
	// https://stackoverflow.com/questions/2693776
	stringVal[strcspn(stringVal, "\n")] = 0;

	return stringVal;
}

/********************************************************************
* Function: changeDirectory
* Receives:
* Returns:
* Description:
* Citations:
* Exploration: Directories
* https://repl.it/@cs344/34directoryc
* Exploration: Environment
* https://repl.it/@cs344/45envvarsc
*********************************************************************/
void changeDirectory(char * userCommand) {

	// Get current working directory
	char * currentPath = malloc(sizeof(char) * MAX_CHAR);
	getcwd(currentPath, sizeof(char) * MAX_CHAR);
	//printf("The current working directory is %s\n", currentPath);

	// Open current directory
	DIR* currentDirectory = opendir(currentPath);

	//struct dirent *aDir;
	/* Iterate through entries
	while ((aDir = readdir(currentDirectory)) != NULL) {
		printf("%s  %lu\n", aDir->d_name, aDir->d_ino);
	}
	*/

	// Change directory without any argument
	if (strncmp("cd", userCommand, strlen(userCommand)) == 0) {
		//printf("Entered cd without arguments.\n");
		// Go to HOME directory
		//printf("Home Directory: %s\n", getenv("HOME"));
		chdir(getenv("HOME"));

	}
	// Change directory with argument
	else {
		char * fileDirectory = malloc(sizeof(char) * MAX_CHAR);
		
		// Get directory name
		strncpy(fileDirectory, userCommand + 3, sizeof(char) * MAX_CHAR);

		//printf("fileDirectory: %s\n", fileDirectory);
		// Go to specified directory
		chdir(fileDirectory);

		free(fileDirectory);

	}

	/* Test print
	getcwd(currentPath, sizeof(char) * MAX_CHAR);
	printf("Changed directory to %s\n", currentPath);
	*/

	// Close Directory
	closedir(currentDirectory);
	// Free memory
	free(currentPath);
}

/********************************************************************
* Function: processCommand
* Receives:
* Returns:
* Description:
*********************************************************************/
void processCommand(char * userCommand) {
	// Identify prefix of command

	// cd (change directory)
	if (strncmp("cd", userCommand, strlen("cd")) == 0) {
		//printf("Entered the cd command.\n");
		changeDirectory(userCommand);
	}
}

/********************************************************************
* Function: requestInputLoop
* Receives:
* Returns:
* Description:
*********************************************************************/
void requestInputLoop() {

	// Request user input into shell
	printColon();
	char *userString = getString();

	// While user has not entered exit command
	while (strstr("exit", userString) == NULL) {

		// Process command
		processCommand(userString);

		// Request input again
		free(userString);
		printColon();
		userString = getString();
	}

	// Exit out of shell
	free(userString);
	// printf("You have entered exit. Exiting...\n");

}

/********************************************************************
* Function: runShell
* Receives:
* Returns:
* Description:
*********************************************************************/
void runShell() {
	requestInputLoop();
}

/********************************************************************
* Main Function
*********************************************************************/

int main(int argc, char *argv[]) {
	
	runShell();

	return EXIT_SUCCESS;
}