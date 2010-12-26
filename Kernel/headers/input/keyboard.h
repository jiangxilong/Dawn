#ifndef _KEYBOARD_DEF_H_
#define _KEYBOARD_DEF_H_
#include <common.h>
#include <fs/vfs.h>

/* Advanced keyboard related input functions and definitions */
#define FLAG_SHIFT_STATE 1
#define FLAG_ESCAPED 2
#define FLAG_CAPS_LOCK 4

//Lookup a asci character from the keyboard lookup table (If it cannot be done return 0)
char lookupAsciCharacterFromScancode(uint8 scancode,uint32 flags);

//Load a keytable from a asci text file
//Return 0 if fail, 1 if succeed
unsigned char loadAsciKeytable(fs_node_t* node);

#endif //_KEYBOARD_DEF_H_