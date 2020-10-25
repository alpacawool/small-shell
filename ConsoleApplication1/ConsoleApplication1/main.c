/********************************************************************
* Program Name: Assignment 3: smallsh
* Author: Patricia Booth
* OSU Email Address: boothpat@oregonstate.edu
* Course Number / Section: CS 344-400
* Due Date: 11/03/2020
* Last Modified: 10/25/2020
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
// Boolean Type
#include <stdbool.h>

#define MAX_CHAR 2048
#define MAX_ARG 512

/* Command Data Structure */
//  command [arg1 arg2 ...][< input_file][> output_file][&]
struct command {
	char * name;
	char * argumentString;
	char ** arguments[MAX_ARG];
	char * inputFile;
	char * outputFile;
	char * jobType;
};

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

	// Open current directory
	DIR* currentDirectory = opendir(currentPath);

	// Change directory without any argument
	if (strncmp("cd", userCommand, strlen(userCommand)) == 0) {
		// Go to HOME directory
		chdir(getenv("HOME"));

	}
	// Change directory with argument
	else {
char * fileDirectory = malloc(sizeof(char) * MAX_CHAR);

// Get directory name
strncpy(fileDirectory, userCommand + 3, sizeof(char) * MAX_CHAR);

// Go to specified directory
chdir(fileDirectory);

free(fileDirectory);

	}

	// Test print
	getcwd(currentPath, sizeof(char) * MAX_CHAR);
	printf("Changed directory to %s\n", currentPath);
	// **************

	// Close Directory
	closedir(currentDirectory);
	// Free memory
	free(currentPath);
}

/********************************************************************
* Function: printStatus
* Receives:
* Returns:
* Description:
*********************************************************************/
/* INSTRUCTION
* * status
* The status command prints out either the exit status or the
* terminating signal of the last foreground process ran by your shell.
* If this command is run before any foreground command is run, then it
* should simply return the exit status 0.
* The three built-in shell commands do not count as foreground processes
* for the purposes of this built-in command - i.e., status should
* ignore built-in commands.*/
void printStatus() {

}

/********************************************************************
* Function: endContains
* Receives:
* Returns:
* Description:
*********************************************************************/
int endContains(char *userString, char *stringEnding) {

	size_t stringLength = strlen(userString);
	size_t endingLength = strlen(stringEnding);

	if (endingLength > stringLength) {
		return 0;
	}

	return strncmp(userString + stringLength - endingLength,
		stringEnding, endingLength) == 0;
}

/********************************************************************
* Function: createCommand
* Receives:
* Returns:
* Description:
* Citation:
* https://stackoverflow.com/questions/13078926
*********************************************************************/
struct command * createCommand(char * userCommand) {
	//  command [arg1 arg2 ...][< input_file][> output_file][&]

	bool hasInput = false;
	bool hasOutput = false;
	bool hasArg = true;

	if (strstr(userCommand, " < ") != NULL) {
		printf("Input detected\n");
		hasInput = true;
	}
	if (strstr(userCommand, " > ") != NULL) {
		printf("Output detected\n");
		hasOutput = true;
	}
	

	// Allocate memory
	struct command *currentCommand = malloc(sizeof(struct command));

	// Assign Job type
	currentCommand->jobType = calloc(strlen("xxxx") + 1, sizeof(char));
	if (endContains(userCommand, "&") == 1) {
		currentCommand->jobType = "back";
	}
	else {
		currentCommand->jobType = "fore";
	}

	// Count amount of tokens
	int count = 0;
	char * charPtr = userCommand;
	while ((charPtr = strchr(charPtr, ' ')) != NULL) {
		count++;
		charPtr++;
	}

	printf("Counted tokens: %d\n", count);

	if (currentCommand->jobType == "fore") {
		if (count == 0) {
			printf("There are no arguments.\n");
			hasArg = false;
		}
		if (count == 2) {
			if (hasInput == true) {
				printf("There are no arguments.\n");
				hasArg = false;
			}
			else if (hasOutput == true) {
				printf("There are no arguments.\n");
				hasArg = false;
			}
		}
		if (count == 4) {
			if (hasInput == true && hasOutput == true) {
				printf("There are no arguments.\n");
				hasArg = false;
			}
		}
	}
	else if (currentCommand->jobType == "back") {
		if (count == 1) {
			printf("There are no arguments.\n");
			hasArg = false;
		}
		if (count == 3) {
			if (hasInput == true) {
				printf("There are no arguments.\n");
				hasArg = false;
			}
			else if (hasOutput == true) {
				printf("There are no arguments.\n");
				hasArg = false;
			}
		}
		if (count == 5) {
			if (hasInput == true && hasOutput == true) {
				printf("There are no arguments.\n");
				hasArg = false;
			}
		}
	}

	char *savePtr;

	//  command [arg1 arg2 ...][< input_file][> output_file][&]

	// Token 1. Command Name [Required]
	char *token = strtok_r(userCommand, " ", &savePtr);
	currentCommand->name = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currentCommand->name, token);

	// Case: ONLY command entered (no arguments)
	if (hasArg == false && hasInput == false) {
		if (hasOutput == false) {
			// Set other attributes to NULL
			printf("One command entered\n");
			currentCommand->argumentString = NULL;
			currentCommand->inputFile = NULL;
			currentCommand->outputFile = NULL;
			return currentCommand;
		}
	}
	// * Case: NO arguments, I/O present
	if (hasArg == false) {
		// Case: Input or Output present

		// Case: Input, no output
		if (hasInput == true && hasOutput == false) {
			// Foreground process
			if (currentCommand->jobType == "fore") {
				token = strtok_r(NULL, "\0", &savePtr);
				printf("Second Token (I/O): %s\n", token);
				currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->inputFile, token+2, strlen(token)-1);
			}
			// Background process
			else if (currentCommand->jobType == "back") {
				token = strtok_r(NULL, "&", &savePtr);
				printf("Second Token (I/O): %s\n", token);
				currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->inputFile, token+2, strlen(token)-3);
			}
			
			currentCommand->argumentString = NULL;
			currentCommand->outputFile = NULL;
			return currentCommand;
		}
		// Case: Output, no input
		else if (hasInput == false && hasOutput == true) {
			// Foreground process
			if (currentCommand->jobType == "fore") {
				token = strtok_r(NULL, "\0", &savePtr);
				printf("Second Token (I/O): %s\n", token);
				currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->outputFile, token + 2, strlen(token) - 1);
			}
			// Background process
			else if (currentCommand->jobType == "back") {
				token = strtok_r(NULL, "&", &savePtr);
				printf("Second Token (I/O): %s\n", token);
				currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->outputFile, token + 2, strlen(token) - 3);
			}

			currentCommand->argumentString = NULL;
			currentCommand->inputFile = NULL;
			return currentCommand;
		}
		// Case: Input with Output afterwards
		token = strtok_r(NULL, ">", &savePtr);
		printf("Second Token (I/O): %s\n", token);
		currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->inputFile, token + 2, strlen(token) - 3);

		// Token 4: Output [Optional]
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, " ", &savePtr);
			printf("Third Token (I/O): %s\n", token);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token, strlen(token));
		}
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			printf("Third Token (I/O): %s\n", token);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token + 1, strlen(token));
		}

		currentCommand->argumentString = NULL;
		return currentCommand;

	}

	// Token 2. Arguments [Optional]

	//  command [arg1 arg2 ...][< input_file][> output_file][&]

	// Case: No Input or Output
	if (hasInput == false && hasOutput == false) {

		// Background process
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, "&", &savePtr);
			printf("Second Token (Arguments): %s\n", token);
			currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->argumentString, token, strlen(token)-1);
		} 
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			printf("Second Token (Arguments): %s\n", token);
			currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currentCommand->argumentString, token);
		}

		

		currentCommand->inputFile = NULL;
		currentCommand->outputFile = NULL;
		return currentCommand;
	}

	// Case: Input or Output after Arguments
	if (hasInput == true) {
		token = strtok_r(NULL, "<", &savePtr);
		printf("Second Token (Arguments): %s\n", token);
		currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->argumentString, token, strlen(token)-1);
	}
	else {
		token = strtok_r(NULL, ">", &savePtr);
		printf("Second Token (Arguments): %s\n", token);
		currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->argumentString, token, strlen(token)-1);
	}

	// Token 3. Input or Output [Optional]

	// Case: Input, no Output
	if (hasOutput == false) {
		// Background process
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, " ", &savePtr);
			printf("Third Token (I/O): %s\n", token);
			currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->inputFile, token, strlen(token));
		}
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			printf("Third Token (I/O): %s\n", token);
			currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->inputFile, token+1, strlen(token) - 1);
		}

		currentCommand->outputFile = NULL;
		return currentCommand;
	}

	// Case: Output, no Input
	if (hasInput == false) {
		// Background process
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, " ", &savePtr);
			printf("Third Token (I/O): %s\n", token);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token, strlen(token));
		}
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			printf("Third Token (I/O): %s\n", token);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token + 1, strlen(token) - 1);
		}


		currentCommand->inputFile = NULL;
		return currentCommand;
	}

	//  command [arg1 arg2 ...][< input_file][> output_file][&]

	// Case: Input with Output afterwards
	token = strtok_r(NULL, ">", &savePtr);
	printf("Third Token (I/O): %s\n", token);
	currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
	strncpy(currentCommand->inputFile, token + 1, strlen(token) - 2);

	// Token 4: Output [Optional]

	if (currentCommand->jobType == "back") {
		token = strtok_r(NULL, " ", &savePtr);
		printf("Fourth Token (I/O): %s\n", token);
		currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->outputFile, token, strlen(token));
	}
	else {
		token = strtok_r(NULL, "\0", &savePtr);
		printf("Fourth Token (I/O): %s\n", token);
		currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->outputFile, token + 1, strlen(token));
	}


	return currentCommand;
}

/********************************************************************
* TEST PRINT COMMAND OBJECT
*********************************************************************/
void printCommand(struct command * currentCommand) {

	printf("--------------------------------------\n");
	printf("Name: [%s]\n", currentCommand->name);
	
	if (currentCommand->argumentString != NULL) {
		printf("Arguments: [%s]\n", currentCommand->argumentString);
	}
	if (currentCommand->inputFile != NULL) {
		printf("Input File: [%s]\n", currentCommand->inputFile);
	}
	if (currentCommand->outputFile != NULL) {
		printf("Output File: [%s]\n", currentCommand->outputFile);
	}
	printf("Job Type: [%s]\n", currentCommand->jobType);

	printf("--------------------------------------\n");
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
	} // Status
	else if (strncmp("status", userCommand, strlen("status")) == 0) {
		printf("Entered the status command.\n");
		printStatus();
	}
	else {
		struct command * currentCommand = createCommand(userCommand);
		printCommand(currentCommand);
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
	while (strncmp("exit", userString, strlen("exit")) != 0) {

		printf("Entered command: %s\n", userString);
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