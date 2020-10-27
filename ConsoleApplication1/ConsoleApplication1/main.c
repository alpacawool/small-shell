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
// for waitpid
#include <sys/wait.h> 
// For file manipulation
#include <fcntl.h>

#define MAX_CHAR 2048
#define MAX_ARG 512

/* Command Data Structure */
//  command [arg1 arg2 ...][< input_file][> output_file][&]
struct command {
	char * name;
	char * argumentString;
	char * arguments[MAX_ARG];
	char * inputFile;
	char * outputFile;
	char * jobType;
};

// Background process linked list 
struct backgroundProcessList {
	int processID;
	struct backgroundProcessList * next;
};

/********************************************************************
* Function: printColon
* Receives:
* Returns: 
* Description: 
*********************************************************************/
void printColon() {
	printf(": ");
	fflush(stdout);
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

	//printf("Entered string of length %d: [%s]", strlen(stringVal), stringVal);

	// Remove trailing space
	if (stringVal[strlen(stringVal)-1] == ' ') {
		printf("There is a trailing space\n");
		stringVal[strlen(stringVal)-1] = '\0';
	}

	//printf("Entered string of length %d: [%s]", strlen(stringVal), stringVal);

	// Blank input
	if (stringVal[0] == '\n' && stringVal[1] == '\0') {
		stringVal[0] = '\0';
	}

	//printf("String length in getString: %d\n", strlen(stringVal));

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
	fflush(stdout);
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
void printStatus(int statusValue) {
	printf("exit status %d\n", statusValue);
	fflush(stdout);
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
* Function: createList
* Receives:
* Returns:
* Description:
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
* Receives:
* Returns:
* Description:
*********************************************************************/
void removeList(struct backgroundProcessList * myList) {

	struct backgroundProcessList * currentProcess = myList;
	struct backgroundProcessList * tempProcess;

	while (currentProcess->next != NULL) {
		tempProcess = currentProcess;
		//printf("Freeing memory of process [%d]\n", currentProcess->processID);
		currentProcess = currentProcess->next;
		free(tempProcess);
	}
	free(currentProcess);
}

/********************************************************************
* Function: addProcess
* Receives:
* Returns:
* Description:
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
* Function: removedProcess
* Receives:
* Returns:
* Description:
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

	//printf("Removing process %d\n", processID);
	// Delete process from list
	processBefore->next = currentProcess->next;
	free(currentProcess);
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
		//printf("Input detected\n");
		hasInput = true;
	}
	if (strstr(userCommand, " > ") != NULL) {
		//printf("Output detected\n");
		hasOutput = true;
	}
	

	// Allocate memory
	struct command *currentCommand = malloc(sizeof(struct command));

	char background[] = "back";
	char foreground[] = "fore";
	// Assign Job type
	currentCommand->jobType = malloc(strlen(background)+1);
	if (currentCommand->jobType != NULL) {
		free(currentCommand->jobType);
	}
	if (endContains(userCommand, "&") == 1) {
		//strcpy(currentCommand->jobType, background);
		currentCommand->jobType = "back";
	}
	else {
		//strcpy(currentCommand->jobType, foreground);
		currentCommand->jobType = "fore";
	}

	// Count amount of tokens
	int count = 0;
	char * charPtr = userCommand;
	while ((charPtr = strchr(charPtr, ' ')) != NULL) {
		count++;
		charPtr++;
	}

	//printf("Counted tokens: %d\n", count);

	if (currentCommand->jobType == "fore") {
		if (count == 0) {
			//printf("There are no arguments.\n");
			hasArg = false;
		}
		if (count == 2) {
			if (hasInput == true) {
				//printf("There are no arguments.\n");
				hasArg = false;
			}
			else if (hasOutput == true) {
				//printf("There are no arguments.\n");
				hasArg = false;
			}
		}
		if (count == 4) {
			if (hasInput == true && hasOutput == true) {
				//printf("There are no arguments.\n");
				hasArg = false;
			}
		}
	}
	else if (currentCommand->jobType == "back") {
		if (count == 1) {
			//printf("There are no arguments.\n");
			hasArg = false;
		}
		if (count == 3) {
			if (hasInput == true) {
				//printf("There are no arguments.\n");
				hasArg = false;
			}
			else if (hasOutput == true) {
				//printf("There are no arguments.\n");
				hasArg = false;
			}
		}
		if (count == 5) {
			if (hasInput == true && hasOutput == true) {
				//printf("There are no arguments.\n");
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
			//printf("One command entered\n");
			currentCommand->argumentString = NULL;
			currentCommand->arguments[0] = currentCommand->name;
			currentCommand->arguments[1] = NULL;
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
				//printf("Second Token (I/O): %s\n", token);
				currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->inputFile, token+2, strlen(token)-1);
			}
			// Background process
			else if (currentCommand->jobType == "back") {
				token = strtok_r(NULL, "&", &savePtr);
				//printf("Second Token (I/O): %s\n", token);
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
				//printf("Second Token (I/O): %s\n", token);
				currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
				strncpy(currentCommand->outputFile, token + 2, strlen(token) - 1);
			}
			// Background process
			else if (currentCommand->jobType == "back") {
				token = strtok_r(NULL, "&", &savePtr);
				//printf("Second Token (I/O): %s\n", token);
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
		//printf("Second Token (I/O): %s\n", token);
		currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->inputFile, token + 2, strlen(token) - 3);

		// Token 4: Output [Optional]
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, " ", &savePtr);
			//printf("Third Token (I/O): %s\n", token);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token, strlen(token));
		}
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			//printf("Third Token (I/O): %s\n", token);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token + 1, strlen(token));
		}

		currentCommand->argumentString = NULL;
		currentCommand->arguments[0] = currentCommand->name;
		currentCommand->arguments[1] = NULL;
		return currentCommand;

	}

	// Token 2. Arguments [Optional]

	//  command [arg1 arg2 ...][< input_file][> output_file][&]

	// Case: No Input or Output
	if (hasInput == false && hasOutput == false) {

		// Background process
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, "&", &savePtr);
			//printf("Second Token (Arguments): %s\n", token);
			currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->argumentString, token, strlen(token)-1);
		} 
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			//printf("Second Token (Arguments): %s\n", token);
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
		//printf("Second Token (Arguments): %s\n", token);
		currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->argumentString, token, strlen(token)-1);
	}
	else {
		token = strtok_r(NULL, ">", &savePtr);
		//printf("Second Token (Arguments): %s\n", token);
		currentCommand->argumentString = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->argumentString, token, strlen(token)-1);
	}

	// Token 3. Input or Output [Optional]

	// Case: Input, no Output
	if (hasOutput == false) {
		// Background process
		if (currentCommand->jobType == "back") {
			token = strtok_r(NULL, " ", &savePtr);
			//printf("Third Token (I/O): %s\n", token);
			currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->inputFile, token, strlen(token));
		}
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			//printf("Third Token (I/O): %s\n", token);
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
			//printf("Third Token (I/O): %s\n", token);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token, strlen(token));
		}
		// Foreground process
		else {
			token = strtok_r(NULL, "\0", &savePtr);
			//printf("Third Token (I/O): %s\n", token);
			currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strncpy(currentCommand->outputFile, token + 1, strlen(token) - 1);
		}


		currentCommand->inputFile = NULL;
		return currentCommand;
	}

	//  command [arg1 arg2 ...][< input_file][> output_file][&]

	// Case: Input with Output afterwards
	token = strtok_r(NULL, ">", &savePtr);
	//printf("Third Token (I/O): %s\n", token);
	currentCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
	strncpy(currentCommand->inputFile, token + 1, strlen(token) - 2);

	// Token 4: Output [Optional]

	if (currentCommand->jobType == "back") {
		token = strtok_r(NULL, " ", &savePtr);
		//printf("Fourth Token (I/O): %s\n", token);
		currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->outputFile, token, strlen(token));
	}
	else {
		token = strtok_r(NULL, "\0", &savePtr);
		//printf("Fourth Token (I/O): %s\n", token);
		currentCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
		strncpy(currentCommand->outputFile, token + 1, strlen(token));
	}


	return currentCommand;
}



/********************************************************************
* Function: createArgArray
* Receives:
* Returns:
* Description:
* Citations:
* https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/
*********************************************************************/
void createArgArray(struct command * currentCommand) {
	char *savePtr = currentCommand->argumentString;;
	char *token;
	currentCommand->arguments[0] = currentCommand->name;
	int i = 1;

	//printf("=======ARRAY======\n");
	while ((token = strtok_r(savePtr, " ", &savePtr))) {
		//printf("%s\n", token);
		currentCommand->arguments[i] = 
			malloc((strlen(token) + 1) * sizeof(char));
		strcpy(currentCommand->arguments[i], token);
		i++;
	}
	currentCommand->arguments[i] = 0;

	//printf("====================\n");
}

/********************************************************************
* TEST PRINT COMMAND OBJECT
*********************************************************************/
void printCommand(struct command * currentCommand) {

	printf("--------------------------------------\n");
	printf("Name: [%s]\n", currentCommand->name);

	if (currentCommand->argumentString != NULL) {
		printf("Arguments: [%s]\n", currentCommand->argumentString);
		int i = 0;
		printf("Arguments Array: ");
		while (currentCommand->arguments[i] != NULL) {
			printf("[%s] ", currentCommand->arguments[i]);
			i++;
		}
		printf("\n");
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
* Function: freeCommand
* Receives:
* Returns:
* Description:
*********************************************************************/
void freeCommand(struct command * currentCommand) {

if (currentCommand->name != NULL) {
	free(currentCommand->name);
}

/*if (currentCommand->jobType != NULL) {
	free(currentCommand->jobType);
}
*/
if (currentCommand->argumentString != NULL) {
	//printf("Freeing argumentString\n");
	free(currentCommand->argumentString);
	int i = 0;
	while (currentCommand->arguments[i] != NULL) {
		free(currentCommand->arguments[i]);
		i++;
	}
}
if (currentCommand->inputFile != NULL) {
	//printf("Freeing inputFile\n");
	free(currentCommand->inputFile);
}
if (currentCommand->outputFile != NULL) {
	//printf("Freeing outputFile\n");
	free(currentCommand->outputFile);
}

if (currentCommand != NULL) {
	free(currentCommand);
}
}

/********************************************************************
* Function: checkBackgroundProcesses
* Receives:
* Returns:
* Description:
*********************************************************************/

void checkBackgroundProcesses(struct backgroundProcessList * myList) {

	if (myList->next == NULL) {
		// Do not execute rest of code
	}
	else {
		
	
		struct backgroundProcessList * currentProcess = myList;
		/* Test print of current processes
		printf("Running processes: ");
		while (currentProcess != NULL) {
			printf("[%d] ", currentProcess->processID);
			currentProcess = currentProcess->next;
		}
		printf("\n");
		//**********************************************/

		int childStatus;
		// Start at first background process
		currentProcess = myList->next;
		// Check if process is finished for each in list
		while (currentProcess != NULL) {
			if (waitpid(currentProcess->processID, &childStatus, WNOHANG) > 0) {

				if (WIFEXITED(childStatus)) {
					//  Print completion message
					printf("background pid %d is done: ", currentProcess->processID);
					fflush(stdout);
					printf("exit status %d\n", WEXITSTATUS(childStatus));
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
* Function: executeCommand
* Receives:
* Returns:
* Description:
* Citation:
* Exploration: Process API - Executing a New Program
* https://repl.it/@cs344/42execvforklsc
* Exploration: Exploration: Processes and I/O
* https://repl.it/@cs344/54redirectc
*********************************************************************/
int executeCommand(struct command * myCommand) {

	// File redirection must be done before execution

	int inFile;
	int outFile;

	// Input file command exists
	if (myCommand->inputFile != NULL) {
		// Open the file
		inFile  = 
			open(myCommand->inputFile, O_RDONLY, 0644);

	}
	
	// Output file command exists
	if (myCommand->outputFile != NULL) {
		outFile =
			open(myCommand->outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0644);

	}

	int childStatus;

	// Fork a new process;
	pid_t spawnPid = fork();

	switch (spawnPid) {
	case -1:
		perror("fork()\n");
		exit(1);
		break;
	case 0:

		// In the child process
		// I/O redirection
		// Input
		if (myCommand->inputFile != NULL) {
			int inResult = dup2(inFile, 0);
			if (inResult == -1) {
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

		// Execute
		execvp(myCommand->name, myCommand->arguments);
		// Exec only returns if there is error
		perror("execvp");
		exit(1);
		break;
	default:
		// In the parent process
		// Wait for child's termination
		spawnPid = waitpid(spawnPid, &childStatus, 0);
		//printf("Parent(%d): child(%d) terminated. Exiting\n",
			//getpid(), spawnPid);
		break;
	}

	if (WIFEXITED(childStatus)) {
		return WEXITSTATUS(childStatus);
	}
	return 0;
}



/********************************************************************
* Function: executeBackCommand
* Receives:
* Returns:
* Description:
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
		// Open the file
		inFile =
			open(myCommand->inputFile, O_RDONLY, 0644);

	}
	// Input file does not exit
	else {
		inFile = open("/dev/null", O_RDONLY, 0644);
	}

	// Output file command exists
	if (myCommand->outputFile != NULL) {
		outFile =
			open(myCommand->outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0644);

	}
	// Output file does not exist
	else {
		outFile = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0644);
	}

	int childStatus;

	// Fork a new process;
	pid_t spawnPid = fork();

	switch (spawnPid) {
	case -1:
		perror("fork()\n");
		exit(1);
		break;
	case 0:

		// In the child process
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

		// Execute
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
* Receives:
* Returns:
* Description:
*********************************************************************/
int processCommand(char * userCommand, 
		int statusValue, struct backgroundProcessList * userList) {
	// Identify prefix of command

	// cd (change directory)
	if (strncmp("cd", userCommand, strlen("cd")) == 0) {
		//printf("Entered the cd command.\n");
		changeDirectory(userCommand);
	} // Status
	else if (strncmp("status", userCommand, strlen("status")) == 0) {
		//printf("Entered the status command.\n");
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
		// Execute command
		// Foreground command
		if (currentCommand->jobType == "fore") {
			statusValue = executeCommand(currentCommand);
		}
		// Background command
		else {
			statusValue = executeBackCommand(currentCommand, userList);
		}
		// Test print
		//printCommand(currentCommand);
		// TO DO: Free memory
		//freeCommand(currentCommand);
	}
	// Check for completed processes
	checkBackgroundProcesses(userList);

	return statusValue;

}

/********************************************************************
* Function: requestInputLoop
* Receives:
* Returns:
* Description:
*********************************************************************/
void requestInputLoop() {

	int statusValue = 0;

	// Request user input into shell
	printColon();
	fflush(stdout);
	char *userString = getString();
	printf(userString);

	// Create Background process list
	struct backgroundProcessList * childList = createList();

	// While user has not entered exit command
	while (strncmp("exit", userString, strlen("exit")) != 0) {

		//printf("Entered command: %s with length %d.\n", 
			//userString, strlen(userString));
		// Process command
		statusValue = processCommand(userString, statusValue, childList);

		// Request input again
		free(userString);
		fflush(stdout);
		printColon();
		userString = getString();
	}

	// Exit out of shell
	removeList(childList);
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