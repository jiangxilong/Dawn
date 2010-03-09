#ifndef _VIRTUAL_MEMORY_MANAGER_DEF_H_
#define _VIRTUAL_MEMORY_MANAGER_DEF_H_

#define PAGE_MASK 0xFFFFF000
#define PAGE_PRESENT   0x1
#define PAGE_WRITE     0x2
#define PAGE_USER      0x4

#define PAGE_DIR_VIRTUAL_ADDR   0xFFBFF000
#define PAGE_TABLE_VIRTUAL_ADDR 0xFFC00000
#define PAGE_DIR_IDX(x) ((uint32) x/1024)
#define PAGE_TABLE_IDX(x) ((uint32) x%1024)

#define page_directory_t uint32
#include "headers/int_types.h"

void init_virt_mm();

void mark_paging_enabled();
void map (uint32 va, uint32 pa, uint32 flags);
void unmap (uint32 va);
char get_mapping (uint32 va, uint32 *pa);

#endif