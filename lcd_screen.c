#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "lcd_screen.h"
#include "game.h"

#define GPIO_BASE_PATH	"/sys/class/gpio"
#define GPIO_EXPORT		"/export"
#define GPIO_PIN		"/gpio"		// Add GPIO number after
#define GPIO_PIN_DIR	"/direction"
#define GPIO_PIN_VALUE	"/value"

#define PATH_BUFF_SIZE 512
#define  INT_BUFF_SIZE 8

// Index of gpioPins
enum GpioPins {
	LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD_RS, LCD_EN
};

#define NUM_GPIO_PINS 10
static int gpioPins[NUM_GPIO_PINS] = {48, 5, 3, 49, 117, 115, 111, 110, 30, 31};
static int bbgPins[NUM_GPIO_PINS] = {15, 17, 21, 23, 25, 27, 29, 31, 11, 13};

// Wait times during LCD execution
#define LCD_EN_PULSE_DELAY_US 	1
#define LCD_WRITE_DELAY_US		50
#define LCD_RTN_HOME_DELAY_US	2000

// RS (Register Select) values
#define LCD_RS_COMD 0
#define LCD_RS_DATA 1

// List of common commands
// Refer to HD44780U documentation to add more
enum LcdScreen_Commands {
	LCD_RTN_HOME	= 0x02, // Resets shift & cursor pos (also used to init in 4-bit mode first)
	LCD_INIT_4BIT	= 0x28, // 2 lines, 5x8 dots, 4-bit mode
	LCD_INIT_8BIT	= 0x38, // 2 lines, 5x8 dots, 8-bit mode
	LCD_DISP_CLEAR	= 0x01, // Clear display, reset cursor
	LCD_DISP_C_ON	= 0b1111, // Bits: Opcode, Display on/off, Cursor on/off, Blinking on/off
	LCD_DISP_C_OFF	= 0b1100,
	LCD_AUTO_R		= 0x06, // Auto increment address right

	LCD_SHIFT_L		= 0x18, // Shifts display left (does not shift data registers)
	LCD_SHIFT_R		= 0x1C, // Shifts display right

	LCD_CSTM_CHAR	= 0x40, // +=8 for next char (0-7)
	LCD_ROW0		= 0x80, // +=1 for next position (0-15)
	LCD_ROW1		= 0xC0, // +=1 for next position (0-15)
	LCD_ROW2		= 0x94, // +=1 for next position (0-15)
	LCD_ROW3		= 0xD4  // +=1 for next position (0-15)
};

static bool isNibbleMode = false; // true = 4-bit, false = 8-bit mode

// Modified from sleepForMs by Brian Fraser
static void sleepForUs(long long delayInUs)
{
	const long long NS_PER_US = 1000;
	const long long NS_PER_SECOND = 1000000000;
	long long delayNs = delayInUs * NS_PER_US;
	int seconds = delayNs / NS_PER_SECOND;
	int nanoseconds = delayNs % NS_PER_SECOND;
	struct timespec reqDelay = {seconds, nanoseconds};
	nanosleep(&reqDelay, (struct timespec *) NULL);
}

// Function provided by Brian Fraser
static void runCommand(char* command)
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
	}
}

// Function provided by Brian Fraser
static void writeToFile(char* fileAddress, char* content)
{
	FILE *pFile = fopen(fileAddress, "w");
	if (pFile == NULL) {
		printf("ERROR OPENING %s.", fileAddress);
		exit(1);
	}

	int charWritten = fprintf(pFile, content);
	if (charWritten < 0) {
		printf("ERROR WRITING DATA");
		exit(1);
	}

	fclose(pFile);
}

static void gpioSetup(void)
{
	char* cfgComdStart = "config-pin p9.";
	char* cfgComdEnd = " gpio";

	// Config & export GPIO pins
	char fileExport[PATH_BUFF_SIZE];
	snprintf(fileExport, PATH_BUFF_SIZE, "%s%s", GPIO_BASE_PATH, GPIO_EXPORT);
	
	for (int i = 0; i < NUM_GPIO_PINS; i++) {
		char command[PATH_BUFF_SIZE];
		snprintf(command, PATH_BUFF_SIZE, "%s%d%s", cfgComdStart, bbgPins[i], cfgComdEnd);
		runCommand(command);

		char gpioPinNum[INT_BUFF_SIZE];
		snprintf(gpioPinNum, INT_BUFF_SIZE, "%d", gpioPins[i]);
		writeToFile(fileExport, gpioPinNum);
	}
	
	sleepForUs(300000); // Wait for export to complete

	// Set GPIO pins to out direction
	char filePinDir[PATH_BUFF_SIZE];
	for (int i = 0; i < NUM_GPIO_PINS; i++) {
		snprintf(filePinDir, PATH_BUFF_SIZE, "%s%s%d%s",
				GPIO_BASE_PATH, GPIO_PIN, gpioPins[i], GPIO_PIN_DIR);
		
		writeToFile(filePinDir, "out");
	}
}

static void pulseEnAndSleepUs(long long delayInUs)
{	// Sends enable pulse to write to register (latches on falling edge)
	char fileEnValue[PATH_BUFF_SIZE];
	snprintf(fileEnValue, PATH_BUFF_SIZE, "%s%s%d%s",
			GPIO_BASE_PATH, GPIO_PIN, gpioPins[LCD_EN], GPIO_PIN_VALUE);

	writeToFile(fileEnValue, "1");
	sleepForUs(LCD_EN_PULSE_DELAY_US);

	writeToFile(fileEnValue, "0");
	sleepForUs(delayInUs);
}

static void LcdScreen_writeNibble(char* nibble)
{	// Split bits to write to the 4 pins
	for (int i = 0; i < 4; i++) {
		char output[2] = {((*nibble >> i) & 0x01) + '0', '\0'};
		char fileDataValue[PATH_BUFF_SIZE];
		snprintf(fileDataValue, PATH_BUFF_SIZE, "%s%s%d%s",
				GPIO_BASE_PATH, GPIO_PIN, gpioPins[i + 4], GPIO_PIN_VALUE);
	
		writeToFile(fileDataValue, output);
	}
}

// Sends a byte to the LCD screen's rsInt register
static void LcdScreen_writeByte(unsigned char byte, int rsInt)
{
	char rsChar[INT_BUFF_SIZE];
	snprintf(rsChar, INT_BUFF_SIZE, "%d", rsInt);
	
	char fileRsValue[PATH_BUFF_SIZE];
	snprintf(fileRsValue, PATH_BUFF_SIZE, "%s%s%d%s",
			GPIO_BASE_PATH, GPIO_PIN, gpioPins[LCD_RS], GPIO_PIN_VALUE);

	writeToFile(fileRsValue, rsChar);

	if (isNibbleMode) {
		char nibble = (byte & 0xF0) >> 4;
		LcdScreen_writeNibble(&nibble);
		pulseEnAndSleepUs(LCD_EN_PULSE_DELAY_US);

		nibble = (byte & 0x0F);
		LcdScreen_writeNibble(&nibble);
	} else {
		for (int i = 0; i < 8; i++) {
			char output[2] = {((byte >> i) & 0x01) + '0', '\0'};
			char fileDataValue[PATH_BUFF_SIZE];
			snprintf(fileDataValue, PATH_BUFF_SIZE, "%s%s%d%s",
					GPIO_BASE_PATH, GPIO_PIN, gpioPins[i], GPIO_PIN_VALUE);

			writeToFile(fileDataValue, output);
		}
	}

	// 0x02 command needs more time to execute
	if ((byte == LCD_DISP_CLEAR || byte == LCD_RTN_HOME) && rsInt == LCD_RS_COMD) {
		pulseEnAndSleepUs(LCD_RTN_HOME_DELAY_US);
	} else {
		pulseEnAndSleepUs(LCD_WRITE_DELAY_US);
	}
}

static void LcdScreen_command(enum LcdScreen_Commands command)
{
	LcdScreen_writeByte(command, LCD_RS_COMD);
}

void LcdScreen_sendData(unsigned char data)
{
	LcdScreen_writeByte(data, LCD_RS_DATA);
}

void LcdScreen_moveCursor(int row, int col)
{
	if (col < 0 || col >= LCD_COL) {
		printf("Error: LcdScreen_moveCursor col must be in range [0,%d]\n", LCD_COL);
		return;
	}

	if (row == 0) {
		LcdScreen_command(LCD_ROW0 + col);
	} else if (row == 1) {
		LcdScreen_command(LCD_ROW1 + col);
	} else if (row == 2) {
		LcdScreen_command(LCD_ROW2 + col);
	} else if (row == 3) {
		LcdScreen_command(LCD_ROW3 + col);
	} else {
		printf("Error: LcdScreen_moveCursor row must be in range [0,3]\n");
	}
}

void LcdScreen_hideCursor()
{
	LcdScreen_command(LCD_DISP_C_OFF);
}

void LcdScreen_showCursor()
{
	LcdScreen_command(LCD_DISP_C_ON);
}

void LcdScreen_writeString(char* message)
{
	for(int i = 0; i < LCD_COL; i++) {
		LcdScreen_writeByte(*(message + i), LCD_RS_DATA);
	}
}

void LcdScreen_placeChar(int row, int col, unsigned char data)
{
	LcdScreen_moveCursor(row, col);
	LcdScreen_sendData(data);
}

void LcdScreen_loadCstmChar(int location, customChar_t customChar)
{
	if (location < 0 || location >= CSTM_CHAR_MEM) {
		printf("Error: LcdScreen_loadCstmChar location must be in range [0,%d]\n", CSTM_CHAR_MEM);
		return;
	}
	LcdScreen_writeByte(LCD_CSTM_CHAR + 8 * location, LCD_RS_COMD);
	for (int i = 0; i < LCD_CHAR_HEIGHT; i++) {
		LcdScreen_writeByte(customChar.bitPattern[i], LCD_RS_DATA);
	}
}

void LcdScreen_setup(bool isNibbleModeIn)
{
	isNibbleMode = isNibbleModeIn;
	gpioSetup();
	
	if (isNibbleMode) {
		LcdScreen_command(LCD_RTN_HOME); // 4-bit mode init
		LcdScreen_command(LCD_INIT_4BIT); // 2 lines, 5x8 dots, 4-bit mode
	} else {
		LcdScreen_command(LCD_INIT_8BIT); // 2 lines, 5x8 dots, 8-bit mode
	}
	LcdScreen_command(LCD_DISP_CLEAR); // Clear display, reset cursor
	LcdScreen_command(LCD_DISP_C_ON); // Display settings
	LcdScreen_command(LCD_AUTO_R); // auto increment address right
}

void LcdScreen_clear(void) 
{
	LcdScreen_command(LCD_DISP_CLEAR);
}
