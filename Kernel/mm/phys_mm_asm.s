[GLOBAL asm_copy_frame]
asm_copy_frame:
   push ebx              ; Preserve the value of ebx
   pushf                 ; Push EFLAGs so we can later restore the interrupt state
                         ;
   cli                   ; Disable interrupts
                         ; Load the destination & source addresses before paging is disabled as they won't exist at that location after its disabled (probably)

   mov ebx, [esp+12]     ; Source address
   mov ecx, [esp+16]     ; Destination address

			 ; Disable paging
   mov edx, cr0          ; get the cr0
   and edx, 0x7fffffff   ; and it with the paging mask
   mov cr0, edx          ; set cr0 to the new value

			 ;4 byte blocks. 1024 * 4 = 4096 = one frame
   mov edx, 1024

.loop:

   mov eax, [ebx]        ;
   mov [ecx], eax        ;

   add ebx, 4            ;
   add ecx, 4            ;
   dec edx               ;
   jnz .loop

   mov edx, cr0          ; Re enable paging
   or  edx, 0x80000000   ;
   mov cr0, edx          ;

   popf                  ; Pop EFLAGS back (Returning the state of interrupts)
   pop ebx               ; Return ebx to its original value
   ret