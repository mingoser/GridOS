/*
	Copyright (C) 2012-2014, GridOS Developing Team
	All Rights Reserved.

	Developers: 
	-> LuYan
	-> Mighten. Dai.( mingchen97@163.com)
	-> Wuj (921954642@qq.com)	
	
	Moudkle File location: /source/drivers/hal/console/console_c_interface.h
	Module        Version: 2.1
	Module    Description:
                        Describe the interface for other modules.
													For example: BAXI input module.
*/
#ifndef      INCLUDE_CONSOLE_C_INTERFACE_H_
#define      INCLUDE_CONSOLE_C_INTERFACE_H_


// Module Entry.
//  Calling hierarchy: printk => console_ops_array->write => i686_write_string
//  NOTE: its return value is of nonsense.
int      i686_write_string(char *buffer, int size);

// Get the main screen's resolution.
void     get_main_screen_resolution( unsigned int *x, unsigned int *y );

// Set the main screen's resolution.
void     set_main_screen_resolution( unsigned int x, unsigned int y );

// Refresh the main screen by using the context of Console descriptor.
void     refresh_main_console_screen( void );


#endif  // #ifndef      INCLUDE_CONSOLE_C_INTERFACE_H_