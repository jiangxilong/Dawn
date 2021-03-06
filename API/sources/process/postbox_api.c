#include <process/postbox_api.h>
#include <types/stdint.h>

//Read top reads a message from the top of the postbox
DEFN_SYSCALL1(postbox_read_top, 1, process_message*);

//Removes the top message from the postbox
DEFN_SYSCALL0(postbox_pop_top, 2);

//Returns 1 if there is data to read 0 if otherwise
DEFN_SYSCALL0(postbox_has_next, 3);

//Sets the flags on this process's postbox
DEFN_SYSCALL1(postbox_set_flags, 6, uint32_t);

unsigned char postboxHasNext() {
	return syscall_postbox_has_next();
}

process_message postboxGetNext() {
	process_message msg;
	syscall_postbox_read_top(&msg);
	syscall_postbox_pop_top();
	return msg;
}

void postboxSetFlags(uint32_t flags) {
	syscall_postbox_set_flags(flags);
}
