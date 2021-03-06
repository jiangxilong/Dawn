#ifndef _STD_LIB_DEF_H_
#define _STD_LIB_DEF_H_

#define malloc(mem) kmalloc(mem)
#define free(mem) kfree(mem)

void* kmalloc(unsigned long);
void kfree(void*);

#endif
