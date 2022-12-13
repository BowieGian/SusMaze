// Module to move the cursor and write characters to an LCD screen
// Created by: Bowie Gian
// Modified on: 2022-12-07
#ifndef LCD_SCREEN_H
#define LCD_SCREEN_H

#define LCD_ROW 4
#define LCD_COL 20
#define LCD_CHAR_HEIGHT 8
#define CSTM_CHAR_MEM 8

typedef struct {
	unsigned char bitPattern[LCD_CHAR_HEIGHT];
} customChar_t;



// Exports gpio pins and configures them to out direction,
// then initializes the LCD screen in (true=4bit, false=8bit) mode
void LcdScreen_setup(bool isNibbleModeIn);

// Clears Screen
void LcdScreen_clear(void);

// Writes the char at the cursor location
void LcdScreen_sendData(unsigned char data);

// Move the cursor starting at 0, 0
void LcdScreen_moveCursor(int row, int col);
void LcdScreen_hideCursor();
void LcdScreen_showCursor();

// Writes a string starting at the cursor location
// message must be filled with ' ' for blanks until the end of the
// row because 0 is the customChar location, not the '\0'
void LcdScreen_writeString(char* message);

// Places a char at location (row, col)
void LcdScreen_placeChar(int row, int col, unsigned char data);

// Loads a custom character at mem location 0-7
void LcdScreen_loadCstmChar(int location, customChar_t customChar);
#endif
