#include <screen/screen.h>
#include <terminal/terminal.h>
#include <stdlib.h>

//The VGA Frame buffer starts at 0xB8000
static uint16 * video_memory_location = (uint16 *)0xB8000;
static unsigned int cursor_x = 0;
static unsigned int cursor_y = 0;
static terminal_t* current_terminal = 0;

void text_mode_set_x(uint8 x) 
{
	cursor_x = x;
}

void text_mode_set_y(uint8 y) 
{
	cursor_y = y;
}

static uint8 active_foreground_col = 15;
static uint8 active_background_col = 0;

static void text_mode_move_cursor() 
{
    // The screen is 80 characters wide...
    uint16 cursorLocation = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
    outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    outb(0x3D5, cursorLocation);      // Send the low cursor byte.
}

// Scrolls the text on the screen up by one line.
static void text_mode_scroll()
{

    // Get a space character with the default colour attributes.
    uint8 attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    uint16 blank = 0x20 /* space */ | (attributeByte << 8);

    // Row 25 is the end, this means we need to scroll up
    if(cursor_y >= 25)
    {
        // Move the current text chunk that makes up the screen
        // back in the buffer by a line
        int i;
        for (i = 0*80; i < 24*80; i++)
        {
            video_memory_location[i] = video_memory_location[i+80];
        }

        // The last line should now be blank. Do this by writing
        // 80 spaces to it.
        for (i = 24*80; i < 25*80; i++)
        {
            video_memory_location[i] = blank;
        }
        // The cursor should now be on the last line.
        cursor_y = 24;
    }
}

void text_mode_clearscreen() 
{
    // Make an attribute byte for the default colours
    uint8 attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    uint16 blank = 0x20 /* space */ | (attributeByte << 8);

    int i;
    for (i = 0; i < 80*25; i++)
    {
        video_memory_location[i] = blank;
    }

    // Move the hardware cursor back to the start.
    cursor_x = 0;
    cursor_y = 0;
    text_mode_move_cursor();
}

void text_mode_set_fg_color(uint8 col) 
{
	active_foreground_col = col;
}

void text_mode_set_bg_color(uint8 col) 
{
	active_background_col = col;
}

void text_mode_putc(char c) 
{
    // The attribute byte is made up of two nibbles - the lower being the 
    // foreground colour, and the upper the background colour.
    uint8  attributeByte = (active_background_col << 4) | (active_foreground_col & 0x0F);
    // The attribute byte is the top 8 bits of the word we have to send to the
    // VGA board.
    uint16 attribute = attributeByte << 8;
    uint16 *location;

    // Handle a backspace, by moving the cursor back one space
    if (c == 0x08 && cursor_x)
    {
        cursor_x--;
    }

    // Handle a tab by increasing the cursor's X, but only to a point
    // where it is divisible by 8.
    else if (c == 0x09)
    {
        cursor_x = (cursor_x+8) & ~(8-1);
    }

    // Handle carriage return
    else if (c == '\r')
    {
        cursor_x = 0;
    }

    // Handle newline by moving cursor back to left and increasing the row
    else if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    // Handle any other printable character.
    else if(c >= ' ')
    {
        location = video_memory_location + (cursor_y*80 + cursor_x);
        *location = c | attribute;
        cursor_x++;
    }

    // Check if we need to insert a new line because we have reached the end
    // of the screen.
    if (cursor_x >= 80)
    {
        cursor_x = 0;
        cursor_y ++;
    }

    // Scroll the screen if needed.
    text_mode_scroll();
    // Move the hardware cursor.
    text_mode_move_cursor();

}

void text_mode_hardwrite(const char * Array) 
{
	unsigned int i = 0;
	while (Array[i]) {
		text_mode_putc(Array[i++]);	
	}
}

void backup_terminal(terminal_t* term)
{
	term->m_backupData = (void*) (malloc(sizeof(uint16) * term->m_width * term->m_height));
	memcpy(term->m_backupData, video_memory_location, sizeof(uint16) * term->m_width * term->m_height);

	term->m_used = 0;
	term->f_putchar = 0;
	term->f_setForeground = 0;
	term->f_setBackground = 0;
	term->f_updateCursor = 0;
}

void text_mode_tputc(terminal_t* term, char c)
{
	text_mode_putc(c);
	term->m_cursorX = cursor_x;
	term->m_cursorY = cursor_y;
}

void text_mode_tsetfg(terminal_t* term, uint8 col)
{
	text_mode_set_fg_color(col);
}

void text_mode_tsetbg(terminal_t* term, uint8 col)
{
	text_mode_set_bg_color(col);
}

void text_mode_tclear(terminal_t* term)
{
	text_mode_clearscreen();
}

void text_mode_tup(terminal_t* term)
{
	cursor_x = term->m_cursorX;
	cursor_y = term->m_cursorY;
}

void text_mode_sett(terminal_t* term)
{
	if (current_terminal != 0)
		backup_terminal(current_terminal);

	current_terminal = 0;
	term->m_used = 1;

	if (term->m_height != 25 || term->m_width != 80)
	{
		//Incompatable with this terminal
		current_terminal = 0;
		text_mode_clearscreen();
		return; //Crash
	}

	text_mode_clearscreen();

	cursor_x = term->m_cursorX;
	cursor_y = term->m_cursorY;
	text_mode_move_cursor();
	
	if (term->m_backupData != 0)
	{
		//Restore from backed up data
		memcpy(video_memory_location, term->m_backupData, sizeof(uint16) * term->m_width * term->m_height);
		free(term->m_backupData);
		term->m_backupData = 0; //Null it after
	}

	term->f_putchar = text_mode_tputc;
	term->f_clear = text_mode_tclear;
	term->f_updateCursor = text_mode_tup;

	term->f_setForeground = text_mode_tsetfg;

	term->f_setBackground = text_mode_tsetbg;

	current_terminal = term;

}
