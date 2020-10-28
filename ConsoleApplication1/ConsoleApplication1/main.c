/********************************************************************
* Program Name: Assignment 3: smallsh
* Author: Patricia Booth
* OSU Email Address: boothpat@oregonstate.edu
* Course Number / Section: CS 344-400
* Due Date: 11/03/2020
* Last Modified: 10/27/2020
* Description: This program is its own miniature version of a shell
* that mimics functionality of shells like bash. It has built in
* commands like cd (change directory), status, and exit. It initiates
* other commands via creating a child processes in either the
* foreground or background. It also supports file I/O and has a 
* variable expansion feature.
* Citations:
* Code adapted from exploration modules and sample code from:
* Operating Systems I (CS 344-400 Fall 2020)
* Instructor: Nauman Chaudhry
* and additional outside sources such as StackOverflow. Specific
* links to code and references included on a function basis.
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// For Opening / Closing Directories
#include <dirent.h>
// For current working directory
#include <unistd.h>
// For Processes
#include <sys/types.h>
// For Boolean Types
#include <stdbool.h>
// For process functions
#include <sys/wait.h> 
// For file manipulation
#include <fcntl.h>
// For signal manipulation
#include <signal.h>

#define MAX_CHAR 2048
#define MAX_ARG 512

// Indicates whether the program should run in foreground mode or not
bool foregroundOnly = false;

/********************************************************************
* Command Structure
* The command structure stores the content of a string entered by
* the user. The command is composed of the command name (required), 
* and optional add-ons including the arguments array, an input file,
* output file, and whether the command is a background process or not
* It is based on the following input:
* command [arg1 arg2 ...] [< input_file] [> output_file] [&]
*********************************************************************/  
struct command {
	char * name;
	char * argumentString;
	char * arguments[MAX_ARG];
	char * inputFile;
	char * outputFile;
	char * jobType;
};

/********************************************************************
* backgroundProcessList Structure
* In order to keep track of child processes in the background, a linked
* list structure of background processes is created.
*********************************************************************/
struct backgroundProcessList {
	int processID;
	struct backgroundProcessList * next;
};

/********************************************************************
* Function: printColon
* Receives: None
* Returns: None
* Description: As stated in program functionality requirements, the
* command shell prints a colon : symbol for each command.
*********************************************************************/
void printColon() {
	printf(": ");
	fflush(stdout);
}

/********************************************************************
* Function: expandVariable
* Receives: [Required] A pointer to char representing string
*           that has "$$" within the string
* Returns: Pointer to char with expanded version of "$$" represnting
*          the smallsh program's ID. Ex. $$ becomes 1234
* Description: This function first converts the process ID of smallsh
*              from a string to integer. Then it determines how many
*              of "$$" appear in the string. Then it finds and replaces
*              the string to the final desired value.
* Note: This function assumes there is "$$" in the string already.
*       This function does not check for this. 
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

	/* Find and replace and build final string
	* = insertPtr points to next expandable variable
	* = targetString points to the portion of string 
	*   following the variable
	* = tempPtr points to end of final string */
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

	/* Remove trailing newline
	 * Based on https://stackoverflow.com/questions/2693776 */
	if (strlen(stringVal) > 1) {
		stringVal[strcspn(stringVal, "\n")] = 0;
	}

	// Remove trailing space
	if (stringVal[strlen(stringVal)-1] == ' ') {
		printf("There is a trailing space\n");
		stringVal[strlen(stringVal)-1] = '\0';
	}

	// Blank input check
	if (stringVal[0] == '\n' && stringVal[1] == '\0') {
		stringVal[0] = '\0';
	}

	return stringVal;
}

/********************************************************************
* Function: stpSignalHandler
* Receives: Integer
* Returns: None
* Description: The signal handler acts as a replacement action for
* the typical signal sent to command line. In this case, this is the
* signal handler for signal SIGTSTP as entered by CTRL + Z. The
* handler allows a toggle between Foreground Only mode and Foreground
* and Background mode that determines which types of processes are
* allowed to run.
* Citation:
* Exploration: Signal Handling API
* https://repl.it/@cs344/53singal2c
*********************************************************************/
void stpSignalHandler(int signo) {
	// Turn on Foreground Only mode
	if (foregroundOnly == false) {
		char* message = 
			"\nEntering foreground only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 50);
		fflush(stdout);
		foregroundOnly = true;
	}
	// Turn on Foreground AND Background Mode
	else if (foregroundOnly == true) {
		char* message = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 30);
		fflush(stdout);
		foregroundOnly = false;
	}
}

/********************************************************************
* Function: installSignalHandlers
* Receives: None
* Returns: None
* Description: Constructs the signal behavior performed in parent
* process of smallsh. CTRL + C is modified to have no effect in
* stopping program as opposed to its normal functionality. CTRL + Z
* is redesigned to have a toggle for foreground mode.
* Citation:
* Exploration: Signal Handling API
* https://repl.it/@cs344/53singal2c
*********************************************************************/
void installSignalHandlers() {

	// Initialize empty sigaction structures for signal modification
	struct sigaction sigintAction = { 0 }; // CTRL + C [SIGINT]
	struct sigaction sigstpAction = { 0 }; // CTRL + Z [SIGTSTP]

	// Register signal handlers
	sigintAction.sa_handler = SIG_IGN; // Ignore input
	sigstpAction.sa_handler = stpSignalHandler;

	// Block all catchable signals while handlers are running
	sigfillset(&sigintAction.sa_mask);
	sigfillset(&sigstpAction.sa_mask);

	// No flags set
	sigintAction.sa_flags = 0;
	sigstpAction.sa_flags = 0;

	// Install our signal handler
	sigaction(SIGINT, &sigintAction, NULL); // CTRL + C [SIGINT]
	sigaction(SIGTSTP, &sigstpAction, NULL); // CTRL + Z [SIGTSTP]
}

/********************************************************************
* Function: changeDirectory
* Receives: Pointer to char containing string of user command input
* Returns: None
* Description: Performs a directory change based on user input. If
* there are no arguments in CD, it reverts to the HOME directory
* Note: Command is required to start with "cd"
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

	// Confirmation print of changing directory
	getcwd(currentPath, sizeof(char) * MAX_CHAR);
	printf("Changed directory to %s\n", currentPath);
	fflush(stdout);

	// Close Directory
	closedir(currentDirectory);
	// Free memory
	free(currentPath);
}

/********************************************************************
* Function: printStatus
* Receives: Integer representing the current status
* Returns: None
* Description: Prints the text to console displaying information about
* the exit status of processes
*********************************************************************/
void printStatus(int statusValue) {
	printf("exit status %d\n", statusValue);
	fflush(stdout);
}

/********************************************************************
* Function: endContains
* Receives: Pointer to char representing string, pointer to char
*           representing suffix
* Returns: Integer indicating whether desired suffix is at end of
*          string
* Description: Compares the end of string and returns whether the
*              string has that suffix
* Citation: Stack Overflow question on comparing string suffix via
*           https://stackoverflow.com/questions/744766/
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
* Function: createList
* Receives: None
* Returns: Pointer to head of backgroundProcessList structure linked
*          list
* Description: Initializes a linked list of background processes. For
*              reference purposes, the parent ID is kept in first node.
*********************************************************************/
struct backgroundProcessList * createList() {
	// Allocate memory
	struct backgroundProcessList * newList =
		malloc(sizeof(struct backgroundProcessList));

	// Assign values
	// First value is parent ID
	newList->processID = getpid();
	newList->next = NULL;

	return newList;
}

/********************************************************************
* Function: removeList
* Receives: Pointer to backgroundProcessList structure representing
*           head of background processes linked list
* Returns: None
* Description: Frees memory allocated to the currently pressent linked
*              list
*********************************************************************/
void removeList(struct backgroundProcessList * myList) {

	struct backgroundProcessList * currentProcess = myList;
	struct backgroundProcessList * tempProcess;
	
	// Free memory of each node currently present
	while (currentProcess->next != NULL) {
		tempProcess = currentProcess;
		currentProcess = currentProcess->next;
		free(tempProcess);
	}
	free(currentProcess);
}

/********************************************************************
* Function: addProcess
* Receives: Pointer to backgroundProcessList structure representing
*           head of linked list, and integer of child's process ID
* Returns: None
* Description: When a new background process is run, a new node is
*              added to the current list with the process ID data.
*********************************************************************/
void addProcess(struct backgroundProcessList * currList, int processID) {
	struct backgroundProcessList * newProcess =
		malloc(sizeof(struct backgroundProcessList));

	newProcess->processID = processID;
	newProcess->next = NULL;

	struct backgroundProcessList * currentProcess = currList;

	while (currentProcess->next != NULL) {
		currentProcess = currentProcess->next;
	}

	currentProcess->next = newProcess;
}

/********************************************************************
* Function: removeProcess
* Receives: Pointer to structure of backgroundProcessList representing
* head of linked list, Integer of process ID to be removed
* Returns: None
* Description: Searches for process ID that recently ended and removes
* it from the list of processes
* Note: Assumes the process ID is valid
*********************************************************************/
void removeProcess(struct backgroundProcessList * currList, int processID) {
	struct backgroundProcessList * currentProcess = currList;
	struct backgroundProcessList * processBefore;

	// Search for process ID
	while (currentProcess->next != NULL
		&& currentProcess->processID != processID) {
		processBefore = currentProcess;
		currentProcess = currentProcess->next;
	}

	// Delete process from list
	processBefore->next = currentProcess->next;
	free(currentProcess);
}

/********************************************************************
* Function: createCommand
* Receives: Pointer to char representing user input
* Returns: Pointer to command structure
* Description: Takes the user input string and allocates it to members
* of the command structure by utilizing strtok_r. Accounts for varying
* cases of how the input is entered ( ex. no arguments or no input file
* but has an output file.) Missing data is marked as NULL.
* Input is formated as the following:
* command [arg1 arg2 ...] [< input_file] [> output_file] [&]
* There are no brackets. There are spaces between each data type.
* Citation:
* CS 344 Student Processing Program
* https://repl.it/@cs344/studentsc
* Counting Tokens in a String
* https://stackoverflow.com/questions/13078926
*********************************************************************/
struct command * createCommand(char * userCommand) {

	bool hasInput = false;
	bool hasOutput = false;
	bool hasArg = true;

	// Command has input or output
	if (strstr(userCommand, " < ") != NULL) {
		hasInput = true;
	}
	if (strstr(userCommand, " > ") != NULL) {
		hasOutput = true;
	}
	
	// Allocate memory for command structure
	struct command *currentCommand = malloc(sizeof(struct command));

	// Assign Job type
	char background[] = "back";
	char foreground[] = "fore";
	currentCommand->jobType = malloc(strlen(background)+1);
	if (currentCommand->jobType != NULL) {
		free(currentCommand->jobType);
	}
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
	
	/* Based on token count, this indicates whether arguments
	 * are contained in the command string */
	if (currentCommand->jobType == "fore") {
		if (count == 0) {
			hasArg = false;
		}
		if (count == 2) {
			if (hasInput == true) {
				hasArg = false;
			}
			else if (hasOutput == true) {
				hasArg = false;
			}
		}
		if (count == 4) {
			if (hasInput == true && hasOutput == true) {
				hasArg = false;
			}
		}
	} // Background input [&]
	else if (currentCommand->jobType == "back") {
		if (count == 1) {
			hasArg = false;
		}
		if (count == 3) {
			if (hasInput == true) {
				hasArg = false;
			}
			else if (hasOutput == true) {
				hasArg = false;
			}
		}
		if (count == 5) {
			if (hasInput == true && hasOutput == true) {
				hasArg = false;
			}
		}
	}

	/* Assign data to command elements based on input entered*/
	char *savePtr;

	// Token 1. Command Name [Required]
	char *token = strtok_r(userCommand, " ", &savePtr);
	currentCommand->name = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currentCommand->name, token);

	// Case: ONLY command entered (no arguments)
	if (hasArg == false && hasInput == false) {
		if (hasOutput == false) {
			// Set other attributes to NULL
			currentCommand->argumentString = NULL;
			currentCommand->arguments[0] = currentCommand->name;
			currentCommand->arguments[1] = NULL;
			currentCommand->inputFile = NULL;
			currentCommand->outputFile = NULL;

			return currentCommand;
		}
	}
	// Case: NO arguments, I/O present
	if (hasArg == false) {
		// Case: Input or Output present

		// Case: Input, no output
		if (hasInput == true && hasOutput == false) {
			// Foreground process
			if (currentCommand->jobType == "fore") {
				token = strtok_r(NULL, "\0", &savePtr);
				currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->inputFile, token+2, strlen(token)-1);
			}
			// Background process
			else if (currentCommand->jobType == "back") {
				token = strtok_r(NULL, "&", &savePtr);
				currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->inputFile, token+2, strlen(token)-3);
			}	
			currentCommand->argumentString = NULL;
			currentCommand->arguments[0] = currentCommand->name;
			currentCommand->arguments[1] = NULL;
			currentCommand->outputFile = NULL;

			return currentCommand;
		}
		// Case: Output, no input
		else if (hasInput == false && hasOutput == true) {
			// Foreground process
			if (currentCommand->jobType == "fore") {
				token = strtok_r(NULL, "\0", &savePtr);
				currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->outputFile, token + 2, strlen(token) - 1);
			}
			// Background process
			else if (currentCommand->jobType == "back") {
				token = strtok_r(NULL, "&", &savePtr);
				currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->outputFile, token + 2, strlen(token) - 3);
			}
			currentCommand->argumentString = NULL;
			currentCommand->arguments[0] = currentCommand->name;
			currentCommand->arguments[1] = NULL;
			currentCommand->inputFile = NULL;

			return currentCommand;
		}
		// Case: Input with Output afterwards
		token = strtok_r(NULL, ">", &savePtr);
		currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->inputFile, token + 2, strlen(token) - 3);

		// Token 4: Output [Optional]
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, " ", &savePtr);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token, strlen(token));
		}
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token + 1, strlen(token));
		}
		currentCommand->argumentString = NULL;
		currentCommand->arguments[0] = currentCommand->name;
		currentCommand->arguments[1] = NULL;

		return currentCommand;
	}
	// Token 2. Arguments [Optional]

	// Case: No Input or Output
	if (hasInput == false && hasOutput == false) {
		// Background process
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, "&", &savePtr);
			currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->argumentString, token, strlen(token)-1);
		} 
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
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
		currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->argumentString, token, strlen(token)-1);
	}
	else {
		token = strtok_r(NULL, ">", &savePtr);
		currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->argumentString, token, strlen(token)-1);
	}
	// Token 3. Input or Output [Optional]

	// Case: Input, no Output
	if (hasOutput == false) {
		// Background process
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, " ", &savePtr);
			currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->inputFile, token, strlen(token));
		}
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
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
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token, strlen(token));
		}
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token + 1, strlen(token) - 1);
		}
		currentCommand->inputFile = NULL;

		return currentCommand;
	}

	// Case: Input with Output afterwards
	token = strtok_r(NULL, ">", &savePtr);
	currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
	strncpy(currentCommand->inputFile, token + 1, strlen(token) - 2);

	// Token 4: Output [Optional]
	// Background process
	if (currentCommand->jobType == "back") {
		token = strtok_r(NULL, " ", &savePtr);
		currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->outputFile, token, strlen(token));
	}
	// Foreground process
	else {
		token = strtok_r(NULL, "\0", &savePtr);
		currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->outputFile, token + 1, strlen(token));
	}
	return currentCommand;
}

/********************************************************************
* Function: createArgArray
* Receives: Pointer to command structure
* Returns: None
* Description: Takes the argument string member in command and 
* creates an array based on the string. 
* Citations:
* https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/
*********************************************************************/
void createArgArray(struct command * currentCommand) {
	char *savePtr = currentCommand->argumentString;;
	char *token;
	currentCommand->arguments[0] = currentCommand->name;
	int i = 1;
	/* Assign each space separated value to invidual value
	 * in arguments array */
	while ((token = strtok_r(savePtr, " ", &savePtr))) {
		currentCommand->arguments[i] = 
			malloc((strlen(token) + 1) * sizeof(char));
		strcpy(currentCommand->arguments[i], token);
		i++;
	}
	currentCommand->arguments[i] = 0;
}

/********************************************************************
* Function: checkBackgroundProcesses
* Receives: Pointer to backgroundProcessList struct representing head
* of linked list
* Returns: None
* Description: Background processes allow the user to continue to
* work on command line while they are running. This function does a
* check of marked as running functions and states if any finished
* or were terminated.
*********************************************************************/
void checkBackgroundProcesses(struct backgroundProcessList * myList) {
	if (myList->next == NULL) {
		/* ! Do not execute rest of code if it only contains first node
		 * which is the parent process ID */
	}
	else {
		// Point to head of list
		struct backgroundProcessList * currentProcess = myList;
		int childStatus;
		// Start at first background process
		currentProcess = myList->next;
		// Check if process is finished for each process in list
		while (currentProcess != NULL) {
			if (waitpid(currentProcess->processID, &childStatus, WNOHANG) > 0) {
				// Normal exit procedure
				if (WIFEXITED(childStatus)) {
					//  Print completion message
					printf("background pid %d is done: ", currentProcess->processID);
					fflush(stdout);
					printf("exit status %d\n", WEXITSTATUS(childStatus));
					fflush(stdout);
					// Remove process from list
					removeProcess(myList, currentProcess->processID);
				}
				// Terminated process
				else if (WIFSIGNALED(childStatus)) {
					//  Print completion message
					printf("background pid %d is done: ", currentProcess->processID);
					fflush(stdout);
					printf("terminated by signal %d\n", WTERMSIG(childStatus));
					fflush(stdout);
					// Remove process from list
					removeProcess(myList, currentProcess->processID);
				}
			}
			currentProcess = currentProcess->next;
		}
	}
}

/********************************************************************
* Function: executeCommand [FOREGROUND]
* Receives: Pointer to command structure
* Returns: Integer of exit status
* Description: This function executes child commands in the foreground.
* If there is file input or output, it handles this too. In addition,
* It has specific signal handlers in the child not applicable to the 
* parent.
* Citations:
* Exploration: Process API - Executing a New Program
* https://repl.it/@cs344/42execvforklsc
* Exploration: Exploration: Processes and I/O
* https://repl.it/@cs344/54redirectc
* Exploration: Signal Handling API
* https://repl.it/@cs344/53siguserc
*********************************************************************/
int executeCommand(struct command * myCommand) {
	// File redirection must be done before execution
	int inFile;
	int outFile;
	// Input file command exists
	if (myCommand->inputFile != NULL) {
		// Open the file as READ-ONLY
		inFile  = 
			open(myCommand->inputFile, O_RDONLY, 0644);
	}
	// Output file command exists
	if (myCommand->outputFile != NULL) {
		// Open the file as WRITE ONLY, TRUNCATE IT, or CREATE IT
		outFile =
			open(myCommand->outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	}

	int childStatus;
	// Fork a new process;
	pid_t spawnPid = fork();

	// Signal Handler
	struct sigaction sigintAction = { 0 },
		sigstpAction = { 0 };

	switch (spawnPid) {
	case -1: // Error forking
		perror("fork()\n");
		exit(1);
		break;
	case 0:
		// In the child process

		// Install custom handlers for child process
		sigintAction.sa_handler = SIG_DFL;
		sigstpAction.sa_handler = SIG_IGN;
		// Block all catchable signals
		sigfillset(&sigintAction.sa_mask);
		// No flags set
		sigintAction.sa_flags = 0;
		// Install our signal handlers
		sigaction(SIGINT, &sigintAction, NULL); // CTRL + C
		sigaction(SIGTSTP, &sigstpAction, NULL); // CTRL + Z

		// I/O redirection
		// Input
		if (myCommand->inputFile != NULL) {
			int inResult = dup2(inFile, 0);
			if (inResult == -1) {
				// If there is error in opening
				perror("dup2");
				// Set status to 1
				exit(1);
			}
		}
		// Output
		if (myCommand->outputFile != NULL) {
			int outResult = dup2(outFile, 1);
			if (outResult == -1) {
				perror("dup2");
				// Set status to 1
				exit(1);
			}
		}
		// Execute command
		execvp(myCommand->name, myCommand->arguments);
		// Exec only returns if there is error
		perror("execvp");
		exit(1);
		break;
	default:
		// In the parent process
		// Wait for child's termination
		spawnPid = waitpid(spawnPid, &childStatus, 0);
		break;
	}
	// Print information about exit or termination
	// Exit Status
	if (WIFEXITED(childStatus)) {
		return WEXITSTATUS(childStatus);
	}
	// Termination signal
	else if (WIFSIGNALED(childStatus)) {
		printf("terminated by signal %d\n", WTERMSIG(childStatus));
	}
	return 0;
}

/********************************************************************
* Function: executeBackCommand
* Receives: Pointer to command structure, pointer to background-
*           ProcessList structure representing head of linked list
* Returns: None
* Description: Similar to executeCommand, this starts a child process
* that executes a command. The difference is that executeBackCommand
* executes the process in the background so the user may continue
* do functions while the process is runnning instead of waiting.
* Citation:
* Exploration: Process API - Executing a New Program
* https://repl.it/@cs344/42execvforklsc
* Exploration: Processes and I/O
* https://repl.it/@cs344/54redirectc
* Process API - Monitoring Child Processes
* https://repl.it/@cs344/42waitpidnohangc
*********************************************************************/
int executeBackCommand(struct command * myCommand, 
	struct backgroundProcessList * myList) {

	int inBackResult;
	int outBackResult;
	// File redirection must be done before execution
	int inFile;
	int outFile;
	// Input file command exists
	if (myCommand->inputFile != NULL) {
		// Open the file as READ ONLY
		inFile =
			open(myCommand->inputFile, O_RDONLY, 0644);

	}
	// Input file does not exit, assign to directory as stated in instructions
	else {
		inFile = open("/dev/null", O_RDONLY, 0644);
	}
	// Output file command exists
	if (myCommand->outputFile != NULL) {
		outFile =
			open(myCommand->outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0644);

	}
	// Output file does not exist, assign to directory as stated in instructions
	else {
		outFile = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0644);
	}

	int childStatus;

	// Fork a new process;
	pid_t spawnPid = fork();

	// Signal Handler
	struct sigaction sigstpAction = { 0 };

	switch (spawnPid) {
	case -1: // Unsuccessful Fork
		perror("fork()\n");
		exit(1);
		break;
	case 0: // In the child process

		// Install Child Signal Handler
		sigstpAction.sa_handler = SIG_IGN;
		sigfillset(&sigstpAction.sa_mask);
		sigstpAction.sa_flags = 0;
		sigaction(SIGTSTP, &sigstpAction, NULL);

		// I/O redirection
		// Input
		inBackResult = dup2(inFile, 0);
		if (inBackResult == -1) {
			perror("dup2");
			// Set status to 1
			exit(1);
		}

		// Output
		outBackResult = dup2(outFile, 1);
		if (outBackResult == -1) {
				perror("dup2");
				// Set status to 1
				exit(1);
		}
		// Execute command
		execvp(myCommand->name, myCommand->arguments);
		// Exec only returns if there is error
		perror("execvp");
		exit(1);
		break;
	default:
		// In the parent process
		// Print background process ID
		printf("background pid is %d\n", spawnPid);
		fflush(stdout);
		// Add background process to list of processes
		addProcess(myList, spawnPid);
		// Return to parent without waiting
		spawnPid = waitpid(spawnPid, &childStatus, WNOHANG);
		break;
	}
	return 0;
}

/********************************************************************
* Function: processCommand
* Receives: Pointer to character representing user input into command
* line, Integer of status value, Pointer to backgroundProcessList
* structure representing head of process linked list
* Returns: Integer of Status Value
* Description: Depending on command entered, executes both built-in
* and exec commands. After the command is executed, the function
* performs a check for ended/terminated background processes.
*********************************************************************/
int processCommand(char * userCommand, 
		int statusValue, struct backgroundProcessList * userList) {

	/* Identify prefix of command
	 * BUILT-IN COMMANDS */

	// cd (change directory) Command
	if (strncmp("cd", userCommand, strlen("cd")) == 0) {
		changeDirectory(userCommand);
	} // status Command
	else if (strncmp("status", userCommand, strlen("status")) == 0) {
		printStatus(statusValue);
	}
	else if (strncmp("#", userCommand, strlen("#")) == 0) {
		// # Commands do nothing
	}
	else if (userCommand[0] == '\0') {
		// Blank commands do nothing
	}
	// Commands that are not built-in
	else {
		// Create command structure
		struct command * currentCommand = createCommand(userCommand);
		// Create argument array if arguments exist
		if (currentCommand->argumentString != NULL) {
			createArgArray(currentCommand);
		}
		// SPECIAL MODE: Foreground Only
		if (foregroundOnly == true) {
			currentCommand->jobType = "fore";
		}
		// Execute command
		// Foreground command
		if (currentCommand->jobType == "fore") {
			statusValue = executeCommand(currentCommand);
		}
		// Background command
		else {
			statusValue = executeBackCommand(currentCommand, userList);
		}
	}
	// Check for completed processes
	checkBackgroundProcesses(userList);

	return statusValue;
}

/********************************************************************
* Function: killRunningChildren
* Receives: Pointer to backgroundProcessList structure representing
* head of process linked list
* Returns: None
* Description: Kills any running background process present to prevent
* processes from running after the smallsh program has ended.
*********************************************************************/
void killRunningChildren(struct backgroundProcessList * myList) {
	// First value in linked list is parent, do not touch
	if (myList->next != NULL) {
		// Build kill command string needed for ending the process
		struct backgroundProcessList * currentProcess = myList->next;
		int statusValue = 0;
		char * commandName = malloc(1024);
		char * commandNumber = malloc(1024);
		sprintf(commandNumber, "%d", currentProcess->processID);
		strcpy(commandName, "kill -9 ");
		strcat(commandName, commandNumber);
		// Loop through each existing process and kill them
		while (currentProcess  != NULL) {
			statusValue = processCommand(commandName, statusValue, myList);
			currentProcess = currentProcess->next;
			if (currentProcess != NULL) {
				// Build another kill command string for next child killing
				sprintf(commandNumber, "%d", currentProcess->processID);
				strcpy(commandName, "kill -9 ");
				strcat(commandName, commandNumber);
			}
		}
		free(commandName);
		free(commandNumber);
	}
}

/********************************************************************
* Function: requestInputLoop
* Receives: None
* Returns: None
* Description: This function loops through the command line after
* initializing the necessary requirements eg. process list, initial
* input, and signal handlers. It loops until exit is entered and then
* performs cleanup of processes.
*********************************************************************/
void requestInputLoop() {

	int statusValue = 0;
	// Install signal handlers
	installSignalHandlers();
	// Request user input into shell
	printColon();
	fflush(stdout);
	char *userString = getString();
	// Create Background process list
	struct backgroundProcessList * childList = createList();

	// While user has not entered exit command
	while (strncmp("exit", userString, strlen("exit")) != 0) {
		// Process command
		statusValue = processCommand(userString, statusValue, childList);
		// Request input again
		free(userString);
		fflush(stdout);
		printColon();
		userString = getString();
	}

	// Exit out of shell
	// Kill running children if any exist
	killRunningChildren(childList);
	// Free memory
	removeList(childList);
	free(userString);
}

/********************************************************************
* Function: runShell
* Receives: None
* Returns: None
* Description: Calls requestInputLoop to start shell running
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