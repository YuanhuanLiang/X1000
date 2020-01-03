#ifndef __JZ_MEM_H__
#define __JZ_MEM_H__

void *JZMalloc(int align, int size);
void jz47_free_alloc_mem();
unsigned int get_phy_addr(unsigned int vaddr);
#endif    /* __JZ_MEM_H__ */
