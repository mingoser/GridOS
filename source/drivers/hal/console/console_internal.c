/*
	Copyright (C) 2012-2014, GridOS Developing Team
	All Rights Reserved.

	Developers: 
	-> LuYan
	-> Mighten. Dai.( mingchen97@163.com)
	-> Wuj (921954642@qq.com)	
	
	Moudkle File location: /source/drivers/hal/console/console_internal.c
	Module        Version: 2.1
	Module    Description:
						The implementation of GridOS.
*/
#include       "console_c_interface.h"
#include       "internal.h"
#include       <string.h>
#include       <console.h>
#include       <screen.h>

// Stored the address of method of operating the screen.
struct hal_console_ops        video_console_ops;

// Describe the status of "current" screen, if it is necessary to create more than one console, it is convenient to mange.
struct console_descriptor     console_main_screen;

// Define the pointer pointed to main screen descriptor.
struct console_descriptor     *pointer_main_screen;


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                     Re-fresh the screen using the screen-buffer pointers.
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void    m_refresh_screen(  struct console_descriptor     *console_pointer )  // @ {
{
	do // while (  console_pointer->m_screen_end <= console_pointer->m_screen_current ) @ {
	{
		// Height checking...
		if ( console_pointer->m_cursor_position_y + DOTFNT_CHAR_LINE_HEIGHT>= 
			console_pointer->m_cursor_position_max_y ) // @ {
		{
			// if need roll screen, return to previous calling hierarchy.
			return ;
		}// @ } if ( need to roll screen , right? )
	
		// Hide the cursor.
		console_cursor_toggle( false );
		
		//====================================================================================================
		//
		//                  Drawing all the data in buffer.
		//                        NOTE: after that, '\0' == *console_pointer->m_screen_end
		//====================================================================================================
		//-----------------------------------------------------------------------------------
		//
		//         1st step:     Check printing is overflow or not.
		//
		//-----------------------------------------------------------------------------------
		// Width checking...
		if ( console_pointer->m_cursor_position_x + DOTFNT_CHAR_WIDTH >= 
			console_pointer->m_cursor_position_max_x)  // @ {
		{
			console_pointer->m_cursor_position_x  = 0;
			console_pointer->m_cursor_position_y += DOTFNT_CHAR_LINE_HEIGHT;
			
			console_pointer->m_current_screen_last_one = console_pointer->m_screen_end++;
			
			m_get_second_line_head_in_buffer( console_pointer, false, console_pointer->m_screen_end );
		} // @ } if ( single-line printing is overflow )
	
		//-----------------------------------------------------------------------------------
		//
		//         2nd step:     Divide & Deal
		//
		//-----------------------------------------------------------------------------------
		if ( *console_pointer->m_screen_current > 0x7f ) // @ {
		{
			u16           GBK_code;
			
			// GBK Code dealing...
			char          first_byte     = *console_pointer->m_screen_current++;
			char          following_byte = *console_pointer->m_screen_current++;
			GBK_code       = ( first_byte << 8 ) | ( following_byte );
			
			console_pointer->m_cursor_position_x += draw_gbk_chinese( GBK_code, 
														  console_pointer->m_cursor_position_x, 
														  console_pointer->m_cursor_position_y, 
														  console_pointer->m_drawing_color);
			console_pointer->m_current_screen_last_one = console_pointer->m_screen_end++;
			console_pointer->m_screen_end++;
		}
		else
		{
			// <CR> & <LF> Dealing... @ {
			if ( '\n' == *console_pointer->m_screen_current || 0x0d == *console_pointer->m_screen_current )
			{
				console_pointer->m_cursor_position_x  =  0;
				console_pointer->m_cursor_position_y  += DOTFNT_CHAR_LINE_HEIGHT;
				console_pointer->m_current_screen_last_one = console_pointer->m_screen_end++;

				m_get_second_line_head_in_buffer( console_pointer, false, console_pointer->m_screen_end );

				goto move_and_enable_cursor;

			} //  @ } if ( 0x0a, 0x0d )

			// Blank space dealing... @ {
			if ( ' ' == *console_pointer->m_screen_current )
			{
				console_pointer->m_cursor_position_x += DOTFNT_CHAR_SPACE_LEN;
				console_pointer->m_current_screen_last_one = console_pointer->m_screen_end++;

				goto move_and_enable_cursor;

			} // @ } if ( it is blank space )

			// Tab  dealing...  @ {
			if ( 0x09 == *console_pointer->m_screen_current )
			{
				console_pointer->m_cursor_position_x |= 31;
				console_pointer->m_cursor_position_x++;
				console_pointer->m_current_screen_last_one = console_pointer->m_screen_end++;

				goto move_and_enable_cursor;

			} // @ } if ( it is tab )
			
			// NUL dealing.... @ {
			if ( '\0' == *console_pointer->m_screen_current )
			{
				console_cursor_toggle( true );
				console_pointer->m_current_screen_last_one = console_pointer->m_screen_end++;
				return ; 
			} // @ } if ( <NUL> )

			// ASCII Code dealing...
			console_pointer->m_cursor_position_x += draw_eng_char(   *console_pointer->m_screen_current,
														  console_pointer->m_cursor_position_x, 
														  console_pointer->m_cursor_position_y, 
														  console_pointer->m_drawing_color);
			console_pointer->m_current_screen_last_one = console_pointer->m_screen_end++;
		} // @ } if ( *console_pointer->m_screen_current > 0x7f )

		///////////////////////////////////////////////////////////////////////////////////
		move_and_enable_cursor:/*    Label:   */ 
		console_cursor_move( console_pointer->m_cursor_position_x, console_pointer->m_cursor_position_y );
		console_cursor_toggle( true );

	}while (  console_pointer->m_screen_end <= console_pointer->m_screen_current ); // @ } 	
		
	return ;
} // @ }  void m_refresh_screen(  struct console_descriptor     *console_pointer  )

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Draw on screen completely.                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////////////
 void    m_sync_buffer_to_screen(  struct console_descriptor  *console_pointer )  // @ {
{
	for ( ;  console_pointer->m_current_screen_last_one != console_pointer->m_screen_current ; )
	{
		m_refresh_screen( console_pointer );
		m_down_ward_screen( console_pointer, true, 1);
	}
	
} // @ }  void   m_sync_buffer_to_screen( struct console_descriptor     *console_pointer )


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                   Refresh the screen and make changes effective.                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void    m_down_ward_screen(  struct console_descriptor *console_pointer, bool is_single_line_rolling,  unsigned int steps ) // @ {
{
// Why comment the judge of multi-rolling? because it JUST A TEST !!!
// it just a test for single line rolling... not now to support multiply-line rolling :(
//	if ( is_single_line_rolling )
//	{
		console_pointer->m_current_screen_first_one = console_pointer->m_current_screen_second_line_head;
		
		m_get_second_line_head_in_buffer( console_pointer, true, /* omit */  console_pointer->m_screen_end  );
		m_refresh_screen( console_pointer );
//	}
}  // @ }  void    m_down_ward_screen(  struct console_descriptor *console_pointer, bool is_single_line_rolling,  unsigned int steps );


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//     Description: Get the resolution of the screen.
//              In: the address of variables stored resolutions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////
 void   m_get_screen_resolution( struct console_descriptor *console_pointer, unsigned int *x, unsigned int *y ) // @ {
{
	*x = console_pointer->m_screen_resolution_x;
	*y = console_pointer->m_screen_resolution_y;

} // @ }  void   m_get_screen_resolution( struct console_descriptor *console_pointer, unsigned int *x, unsigned int *y )

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//     Description: Set the resolution of the screen, and refresh the screen.( Make changes effective. )
//              In: the address of variables stored resolutions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////
 void   m_set_screen_resolution(  struct console_descriptor *console_pointer, unsigned int x,  unsigned int y ) // @ {
{
// if the resolution passed in have zero, just ignore.
	if ( x && y )
	{
		console_pointer->m_screen_resolution_x = x;
		console_pointer->m_screen_resolution_y = y;
		
		m_refresh_screen( console_pointer );
	}
	
} // @ }  void   m_set_screen_resolution(  struct console_descriptor *console_pointer, unsigned int x,  unsigned int y )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Console Descriptor Initiator  
 void          m_console_descriptor_initialize( struct console_descriptor    *console_pointer ) // @ {
{
	// use '\0' fill the buffer.
	m_zero_fill_buffer( console_pointer, CONSOLE_BUFFER_SIZE );
	
	// Setting the buffer pointer.
	console_pointer->m_buffer_begin   = &console_pointer->buffer[0];
	console_pointer->m_buffer_current = &console_pointer->buffer[0];
	console_pointer->m_buffer_end     = &console_pointer->buffer[ CONSOLE_BUFFER_SIZE - 1 ];
	
	// Setting the screen pointer.
	console_pointer->m_screen_begin   = console_pointer->m_buffer_begin;
	console_pointer->m_screen_current = console_pointer->m_screen_begin;
	console_pointer->m_screen_end     = console_pointer->m_screen_begin;
	
	console_pointer->m_current_screen_first_one        = console_pointer->m_screen_begin;
	console_pointer->m_current_screen_second_line_head = console_pointer->m_screen_begin;
	console_pointer->m_current_screen_last_one         = console_pointer->m_screen_begin;
	
	// Setting the drawing color.
	console_pointer->m_drawing_color  = WHITE_PIX;
	
	// Initializing the step(s) can upward.
	console_pointer->m_steps_can_upward  = 0;
	
	// Setting the default screen resolution, and make it effective.
	console_pointer->m_screen_resolution_x = CONSOLE_SCREEN_DEFAULT_RESOLUTION_X;
	console_pointer->m_screen_resolution_y = CONSOLE_SCREEN_DEFAULT_RESOLUTION_Y;
	m_refresh_screen( console_pointer );  // make changes effective.

	// Cursor setting.
	console_pointer->m_cursor_position_x = 0;
	console_pointer->m_cursor_position_y = 0;
	console_pointer->m_cursor_position_max_x = console_pointer->m_screen_resolution_x;
	console_pointer->m_cursor_position_max_x = console_pointer->m_screen_resolution_y;

	// Cursor initializing.
	console_cursor_setup();
	console_cursor_set_height( DOTFNT_CHAR_LINE_HEIGHT );

} // @ }   void m_console_descriptor_initialize( ...... )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Console Descriptor's Buffer fill with NULL  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void          m_zero_fill_buffer( struct console_descriptor    *console_pointer, unsigned int buffer_space_count )
{
	unsigned int space_number;

	// Filling order : end to head.
	for ( space_number = buffer_space_count ; space_number > 0; --space_number )
	{
		console_pointer->m_buffer_begin[space_number] = '\0';
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Record the second line once, was reset by rolling screen.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
 void      m_get_second_line_head_in_buffer(  struct console_descriptor *console_pointer, bool you_can_roll_screen,  unsigned char *new_second_line_head__you_think ) // @ {
{
	static bool second_address_is_locked = false;
	
	if ( false == you_can_roll_screen )
	{
	    // Protect the second line's head address.
		second_address_is_locked = true;	

		console_pointer->m_current_screen_second_line_head = (unsigned char *) new_second_line_head__you_think;
	}
	else
	{
		second_address_is_locked = false;
	}
} // @ }  void      m_get_second_line_head_in_buffer(  struct console_descriptor *console_pointer, bool you_can_roll_screen,  unsigned char *new_second_line_head__you_think )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   Module Entry Point
//
//  Calling hierarchy: printk => console_ops_array->write => i686_write_string
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int     i686_write_string(char *string, int size) // @ {
{
	int     count = size - 2; // '\0' is counted, and the subscript of array is begin with 0, so subtract 2
	char         *p_string       = (char *) string;
	char         *p_string_end   = (char *) (unsigned int)( string + count);
	
	//----------------------------------------------------------------------------------------------
	// Prepare: Check the data is overflow or not.   
	//                                               
	//           NOTE: Need Overflow Measures. -------Discard the current string.
	//----------------------------------------------------------------------------------------------
	if (  pointer_main_screen->m_buffer_current + (unsigned int)size > 
	      pointer_main_screen->m_buffer_end )
	{
		// clear the screen,
		// and draw a sentence -->  BUFFER IS OVERFLOW !!!!
		return ;
	} // if the buffer is overflow.
	
	//----------------------------------------------------------------------------------------------
	//
	//   Step #1:  Copy the useful data to buffer   :)
	//           NOTE:  *p_string  always point to next character after copied one character.
	//----------------------------------------------------------------------------------------------
	do     // @ {  while( p_string != p_string_end );
	{
		// I hope it is not a GBK code.
		bool is_GBK = false;
		
		switch ( *p_string )// @ { 
		{
		case 0x00:  // <NUL>,  fall through.
		case 0x09:  // <Tab>,  fall through.
		case 0x0a:  // <LF>,   fall through.
		case 0x0d:  // <CR>,   fall through.
			break;  //    copy the above character to buffer.
		
		case 0x7f:  // <DEL>, must be cleared by default, or will not display normally due to GBK.( sizeof ASCII = 1, sizeof GBK = 2)
		default:
			// ignore other Control character.
			if ( *p_string < 0x20 )  /* then */ continue;
			
			// Special dealing for GBK coding China's characters..
			// if it is GBK codes, is_GBK set to 1.
			if ( *p_string > 0x7f )  /* then */ is_GBK = true;

		}// @ } switch ( *p_string )

		// Must divide and deal, or  ASCII & GBK will have a great FIGHT !!!!!!
		if ( is_GBK ) // @ {
		{   // [ double bytes ] GBK  codes writing...
			*pointer_main_screen->m_buffer_current++ = *p_string++;
			*pointer_main_screen->m_buffer_current++ = *p_string++;
		}
		else 
		{   // [ single byte ] ASCII codes writing...
			*pointer_main_screen->m_buffer_current++ = *p_string++;
		} // @ } if ( is_GBK )
	}
	while( p_string != p_string_end + 1 ); // @ }
	//----------------------------------------------------------------------------------------------
	//
	//   Step #2:  synchronize the buffer to pointer.
	//
	//----------------------------------------------------------------------------------------------
	m_sync_buffer_to_screen( pointer_main_screen );

	return 0;

} //  @ } int     i686_write_string(char *buffer, int size)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//     >>>>>>>>>>>>>>> Interface.
//                     Re-fresh the main screen.
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void          refresh_main_console_screen( void )
{
	m_refresh_screen( pointer_main_screen );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//     >>>>>>>>>>>>>>> Interface.
//     Description: Get the resolution of the screen.
//              In: the address of variables stored resolutions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void     get_main_screen_resolution( unsigned int *x, unsigned int *y ) // @ {
{
// Default Console "Object" :  Main Console screen.
	m_get_screen_resolution(  pointer_main_screen, x, y );

} // @ } void   get_main_screen_resolution( unsigned int *x, unsigned int *y )

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//     >>>>>>>>>>>>>>> Interface.
//     Description: Set the resolution of the screen, and refresh the screen.( Make changes effective. )
//              In: the address of variables stored resolutions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void     set_main_screen_resolution( unsigned int x, unsigned int y )
{
	m_set_screen_resolution( pointer_main_screen, x, y );

} // @ } set_main_screen_resolution( ....... )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HAL Console Initiator,  
//     NOTE: This function is invoked before running GridOS !!!
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void     hal_console_init( void )
{
	// the method of operating the "current" screen.
	video_console_ops.write = i686_write_string;
	
	// Bind
	pointer_main_screen = &console_main_screen;
	
	m_console_descriptor_initialize( pointer_main_screen );
}