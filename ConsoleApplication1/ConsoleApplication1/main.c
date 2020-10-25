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
// Processes
#include <sys/types.h>

#define MAX_CHAR 2048


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
* Function: expandVariable
* Receives:
* Returns:
* Description:
* Citations: 
* Exploration: Process Concept & States
* https://repl.it/@cs344/42pidc
* StackOverflow question on converting integers to strings
* https://stackoverflow.com/questions/8257714/
* Find and replace substrings in C
* https://stackoverflow.com/questions/779875/
*********************************************************************/
char * expandVariable(char * targetString) {

	char * expandedString;
	char * tempPtr;
	char * insertPtr;
	int lengthDistance;
	int count;

	char * variableProcess = "$$";

	// Get process ID
	int currentProcess = getpid();

	// Convert process ID to string
	char * processString = malloc(sizeof(char) * MAX_CHAR);
	sprintf(processString, "%d", currentProcess);

	// Get length of variable and expansion
	int variableLength = strlen(variableProcess);
	int expandLength = strlen(processString);

	// Count how many replacements
	insertPtr = targetString;
	for (count = 0; tempPtr = strstr(insertPtr, variableProcess); ++count) {
		insertPtr = tempPtr + variableLength;
	}
	tempPtr
		= expandedString
		= malloc(strlen(targetString)
			+ (expandLength - variableLength) * count + 1);

	// Find and replace and build final string
	while (count--) {
		insertPtr = strstr(targetString, variableProcess);
		lengthDistance = insertPtr - targetString;
		tempPtr = strncpy(tempPtr, targetString, lengthDistance)
			+ lengthDistance;
		tempPtr = strcpy(tempPtr, processString) + expandLength;
		targetString += lengthDistance + variableLength;
	}
	strcpy(tempPtr, targetString);

	// Free memory
	free(processString);


	return expandedString;
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
* Stack Overflow question on finding substrings
*           https://stackoverflow.com/questions/12784766/ 
*********************************************************************/
char * getString() {
	char *stringVal;

	// Shell must support command lines up to 2048 characters
	stringVal = malloc(sizeof(char) * MAX_CHAR);

	//scanf("%s", stringVal);
	fgets(stringVal, MAX_CHAR, stdin);

	// See if string contains $$
	if (strstr(stringVal, "$$") != NULL) {
		// Variable expansion of $$ into process ID
		char * expandedVal;
		expandedVal = expandVariable(stringVal);
		free(stringVal);
		stringVal = expandedVal;
	}

	// Remove trailing newline
	// https://stackoverflow.com/questions/2693776
	if (strlen(stringVal) > 1) {
		stringVal[strcspn(stringVal, "\n")] = 0;
	}

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

	// Test print
	getcwd(currentPath, sizeof(char) * MAX_CHAR);
	printf("Changed directory to %s\n", currentPath);

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

		printf("userString: %s\n", userString);
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