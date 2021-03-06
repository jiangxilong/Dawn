// Dawn Entry Point
// Main function is the entry point of the kernel.

#include <multiboot.h>
#include <version/kernel_version.h>
#include <common.h>
#include <stdlib.h>
#include <printf.h>

void initializeKernel(struct multiboot* mboot_ptr, uint32_t initial_esp);
void postInitialization();

/**
 @brief Main entry point of the Kernel. It is passed the multiboot header by GRUB when the bootloader begins the Kernel execution. (Multiboot header defined in multiboot.h)
 @callgraph
 */
int entry_point(struct multiboot* mboot,
		uint32_t stack_ptr /* Pointer to the stack pointer */) {
	//The kernel SHOULLDD now init fine, and think its in the magical land of the higher half

	//Needs the multiboot location to function, 1 signals that the init routines should be noisy and cover the screen in stuff
	initializeKernel(mboot, stack_ptr);

	printf("Dawn - Kernel initialized\n");

	printf("Kernel Codename: \"%s\" Version: %i.%i.%i\n", KVERSION_CODENAME,
			KVERSION_MAJOR, KVERSION_MINOR, KVERSION_BUILD);

	//Run whatever the kernel is designed to do after initialization
	postInitialization();

	for (;;) {
	}

	return 0xDEADBABA;
}
