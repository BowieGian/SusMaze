//  created by Chenting Mao
//	last edited: 2022/10/2	by:	Chenting Mao
//	last edited: 2022/10/31	by:	Chenting Mao

#include "gpio.h"

#define CMD_BUFF_SIZE 1024

//	Unique Error Code for GPIO value reading check
//	Must be string of numbers
//	#define UNIQUE_ERROR_MESSAGE "97392913"

//	function given in notes
//	excute "command" in Linux
//	return false if command is unable to excute
static bool runCommand(char* command);

//	function given in notes
//	return < cat "fileName" > in Linux
//	return UNIQUE_ERROR_MESSAGE if file opening failed
// 	malloc memory so need to free the memory
static char* readFromFileToScreen(const char *fileName)
{
	FILE *pFile = fopen(fileName, "r");
	if (pFile == NULL) {
		printf("ERROR: Unable to open file (%s) for read\n", fileName);
		return NULL;
	}

	// Read string (line)
	const int MAX_LENGTH = 1024;
	char buff[MAX_LENGTH];
	fgets(buff, MAX_LENGTH, pFile);
	char *message = malloc(sizeof(message)*MAX_LENGTH);
	snprintf(message, MAX_LENGTH, "%s", buff);

	// Close
	fclose(pFile);
	//printf("Read: %s \n", s);
	return message;
}



bool UserInit(void)
{
    return(runCommand("config-pin p8.43 gpio"));
}


int ReadUserValue(void)
{
	char* UserValue = readFromFileToScreen(USER_VALUE);
	int UserValueCopy = atoi(UserValue);
	free(UserValue);
	UserValue = NULL;
	return UserValueCopy;
}


// bool ReadUserValueErrorCheck(void)
// {
//     return(ReadUserValue() == atoi(UNIQUE_ERROR_MESSAGE));
// }


bool UserIsPressed(void)
{
	if(ReadUserValue()==0){
		return true;
	}
	else{
		return false;
	}
	
}


static bool runCommand(char* command)
{
 	// Execute the shell command (output into pipe)
 	FILE *pipe = popen(command, "r");

 	// Ignore output of the command; but consume it 
 	// so we don't get an error when closing the pipe.
 	char buffer[1024];
 	while (!feof(pipe) && !ferror(pipe)) {
 		if (fgets(buffer, sizeof(buffer), pipe) == NULL)
 			break;
 	// printf("--> %s", buffer); // Uncomment for debugging
 	}

 	// Get the exit code from the pipe; non-zero is an error:
 	int exitCode = WEXITSTATUS(pclose(pipe));
 	if (exitCode != 0) {
 		perror("Unable to execute command:");
 		printf(" command: %s\n", command);
 		printf(" exit code: %d\n", exitCode);
		return 0;
 	}
	return 1;
}




void GPIO_8_Init(int gpioNum)
{
	char command[CMD_BUFF_SIZE];
	snprintf(command, CMD_BUFF_SIZE, "config-pin p8.%d gpio", gpioNum);
	runCommand(command);

}


void GPIO_9_Init(int gpioNum)
{
	char command[CMD_BUFF_SIZE];
	snprintf(command, CMD_BUFF_SIZE, "config-pin p9.%d gpio", gpioNum);
	runCommand(command);
}




int ReadGPIOValue(int gpioNum)
{
	char GPIOValueFile[CMD_BUFF_SIZE];
	snprintf(GPIOValueFile, CMD_BUFF_SIZE, "/sys/class/gpio/gpio%d/value", gpioNum);

	char* GpioValue = readFromFileToScreen(GPIOValueFile);
	int GpioValueCopy = atoi(GpioValue);
	free(GpioValue);
	GpioValue = NULL;
	return GpioValueCopy;
}