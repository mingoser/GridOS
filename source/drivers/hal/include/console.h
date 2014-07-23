/*
	Copyright (C) 2012-2014, GridOS Developing Team
	All Rights Reserved.

	Developers: 
	-> LuYan
	-> Mighten. Dai.( mingchen97@163.com)
	-> Wuj (921954642@qq.com)	
	
	Moudkle File location: /source/drivers/hal/include/console.h
	Module        Version: 2.1
	Module    Description:
						HAL Console
						
						Print the debug message from printk.
						
						This file offer the necessary structure.
*/
#ifndef INCLUDE_HAL_CONSOLE_H___
#define INCLUDE_HAL_CONSOLE_H___


#include <ddk/compatible.h>
#include <screen.h>


#define          CONSOLE_BUFFER_SIZE           5 * 1024   // Buffer size.

#define DOTFNT_CHAR_SPACE_LEN		6		// pixel length for " "
#define DOTFNT_CHAR_LINE_HEIGHT		15		// pixel count of the HEIGHT of a line
#define DOTFNT_CHAR_WIDTH	    	16		// used to judge it is necessary for next-line drawing or not.

#define CONSOLE_SCREEN_DEFAULT_RESOLUTION_X		1024
#define CONSOLE_SCREEN_DEFAULT_RESOLUTION_Y		768

// Pixels.
#define WHITE_PIX 0xffffffff
#define BLACK_PIX 0

////////////////////////////////////////////////////////////////////
struct   console_descriptor  // @ {
{
	// Buffer
	char  buffer[CONSOLE_BUFFER_SIZE];
	
	// Variables #1
	// Drawing Color.
	unsigned int  m_drawing_color;
	
	// Variables #2
	// the screen buffer pointers.
	unsigned char *m_buffer_begin;
	unsigned char *m_buffer_current;
	unsigned char *m_buffer_end;

	// Variables #3
	//  Point to the area display on the screen.
	unsigned char *m_screen_begin;
	unsigned char *m_screen_current;        // replace '\0' with cursor
	unsigned char *m_screen_end;

	// Variables #4
	// the screen cursor's current position.
	unsigned int  m_cursor_position_x;      // this->m_screen_current
	unsigned int  m_cursor_position_y;      // this->m_screen_current
	unsigned int  m_cursor_position_max_x;  // the limited position x of cursor.
	unsigned int  m_cursor_position_max_y;  // the limited position y of cursor.

	// Variables #5
	// screen's resolution.
	unsigned int  m_screen_resolution_x;
	unsigned int  m_screen_resolution_y;

	// Variables #6
	// Provided for BAXI keyboard framework.
//	bool          m_BAXI_input_enable;
//	unsigned int  m_BAXI_input_screen_begin_x;
//	unsigned int  m_BAXI_input_screen_begin_y;
//	unsigned int  m_BAXI_input_screen_end_x;
//	unsigned int  m_BAXI_input_screen_end_y;
//	char         *m_BAXI_input_buffer_begin;
//	char         *m_BAXI_input_buffer_end;

	// Variables #7
	// Steps of screen rolling.
	unsigned char *m_current_screen_first_one;
	unsigned char *m_current_screen_second_line_head;
	unsigned char *m_current_screen_last_one;
	
	unsigned int  m_steps_can_upward;

};   // struct   console_descriptor @ }

////////////////////////////////////////////////////////////////////
struct hal_console_ops // @ {
{
	int (*read) (char *buffer, int size);
	int (*write)(char *buffer, int size);
}; // struct hal_console_ops // @ }

////////////////////////////////////////////////////////////////////
// the previous calling hierarchy of   i686_write_string [ directly invoked by pointers array ]
void     console_write(char *string, int size);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HAL Console Initiator,  
//     NOTE: This function is invoked before running GridOS !!!
void     hal_console_init( void );


#endif  // #ifndef INCLUDE_HAL_CONSOLE_H___