#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <jpa_api/jz_mem.h>

#define PAGE_SIZE (4096)
#define MEM_ALLOC_DEV_NUM 1

struct JZ47_MEM_DEV {
        unsigned int vaddr;
        unsigned int paddr;
        unsigned int totalsize;
        unsigned int usedsize;
} jz47_memdev[MEM_ALLOC_DEV_NUM];

struct IMEM_INFO
{
        char *name;
        unsigned int power;
        unsigned int vaddr;
}imem_info[4] = {{"/proc/jz/mem/imem", 12/*32M*/, 0x0/* 0x24000000 */},
                  {"/proc/jz/mem/imem1", 0/*16M*/, 0x0},
                  {"/proc/jz/mem/imem2", 0, 0x0},
                  {"/proc/jz/mem/imem3", 0, 0x0}};

static int memdev_count = 0, mem_init = 0;
int mmapfd = 0;

unsigned int get_phy_addr(unsigned int vaddr)
{
        int i;
        int high_4bits;
        for (i = 0; i < memdev_count; i++) {
                if (vaddr >= jz47_memdev[i].vaddr &&
                    vaddr < jz47_memdev[i].vaddr + jz47_memdev[i].totalsize)
                        return jz47_memdev[i].paddr + (vaddr - jz47_memdev[i].vaddr);
        }

        high_4bits = (vaddr >> 28) & 0xf;

        printf("high_4bits is 0x%x\n", high_4bits);
        if((high_4bits & 0xb))
                return vaddr & 0x1FFFFFFF;
        printf("vaddr 0x%x get phy addr failed\n", vaddr);
        return -1;
}

unsigned int get_vaddr(unsigned int paddr)
{
        int i;

        for (i = 0; i < memdev_count; i++) {
                if (paddr >= jz47_memdev[i].paddr &&
                    paddr < jz47_memdev[i].paddr + jz47_memdev[i].totalsize)
                        return jz47_memdev[i].vaddr + (paddr - jz47_memdev[i].paddr);
        }

        return 0;
}

static void jz47_exit_memalloc (void *p)
{
        int i;
        char cmdbuf[128];

        for (i = 0; i < memdev_count; i++) {
                if (jz47_memdev[i].vaddr) {
                        munmap((void *)jz47_memdev[i].vaddr, jz47_memdev[i].totalsize);
                        memset (&jz47_memdev[i], 0, sizeof(struct JZ47_MEM_DEV));
                }

                sprintf (cmdbuf, "echo FF > %s", imem_info[i].name);
                system (cmdbuf);
        }
}

void jz47_free_alloc_mem()
{
        int i;
        jz47_exit_memalloc(NULL);
        for (i = 0; i < memdev_count; i++)
                jz47_memdev[i].usedsize = 0;

        memdev_count = 0;
        mem_init = 0;

        if (mmapfd)
                close(mmapfd);
        mmapfd = 0;
        printf("------mmapfd = %d\n", mmapfd);
}

static void jz47_alloc_memdev()
{
        int power, pages=1, fd;
        char cmdbuf[128];
        unsigned int vaddr, paddr;

        /* Free all proc memory.  */
        if (mem_init == 0) {
                sprintf (cmdbuf, "echo FF > %s", imem_info[0].name);
                // echo FF > /proc/jz/imem
                system (cmdbuf);
                mem_init = 1;
        }
        printf("------mmapfd = %d\n", mmapfd);
        /* open /dev/mem for map the memory.  */
        if (mmapfd == 0)
                mmapfd = open ("/dev/mem", O_RDWR);
        if (mmapfd <= 0) {
                printf ("+++ Jz47 alloc frame failed: can not mmap the memory +++\n");
                return;
        }

        /* Get the memory device.  */
        power = imem_info[memdev_count].power; /* request max mem size of 2 ^ n pages  */
        paddr = 0;

        printf ("+++ Jz47:  power 0x%x memory, imem_info[memdev_count].name = %s.\n", power, imem_info[memdev_count].name);

        while (1) {
                // request mem
                sprintf (cmdbuf, "echo %x > %s", power, imem_info[memdev_count].name);
                system (cmdbuf);

                // get paddr
                fd = open (imem_info[memdev_count].name, O_RDWR);
                if (fd >= 0) {
                        read (fd, &paddr, 4);
                        close (fd);
                }
                if (paddr != 0 || power < 8)
                        break;
                power--;
        }

        pages = 1 << power; /* total page numbers.  */
        /* Set the memory device info.  */
        if (paddr == 0) {
                printf ("+++ Jz47: Can not get address of the reserved 4M memory.\n");
                return;
        } else {
                printf ("+++ Jz47:  paddr 0x%x memory, pages =%d.\n", paddr, pages);
                printf ("+++ Jz47:  imem_info[memdev_count].vaddr 0x%x.\n", imem_info[memdev_count].vaddr);
                /* mmap the memory and record vaddr and paddr.  */
                /* vaddr = (unsigned int)mmap ((void *)imem_info[memdev_count].vaddr, */
                /*                             pages * PAGE_SIZE * 2, /\* since mips TLB table mmap is double pages.  *\/ */
                /*                             (PROT_READ|PROT_WRITE), MAP_SHARED, mmapfd, paddr); */
                vaddr = (unsigned int)mmap ((void *)imem_info[memdev_count].vaddr,
                                            pages * PAGE_SIZE, /* since mips TLB table mmap is double pages.  */
                                            (PROT_READ|PROT_WRITE), MAP_SHARED, mmapfd, paddr);

                // touch the space to create TLB table in CPU
                printf ("+++ Jz47:  get address 0x%x memory.\n", vaddr);
                unsigned int pc = vaddr;
                int pg = 0;
                for (pg = 0; pg < pages; pg++) {
                        *(volatile unsigned int *)pc = 0x0;
                        pc += PAGE_SIZE;
                }
                pc--;
                printf ("+++ Jz47:  get address pc 0x%x memory.\n", pc);
//                *(volatile unsigned int *)pc = 0x0;
                printf ("+++ Jz47:  1.\n");

                jz47_memdev[memdev_count].vaddr = vaddr;
                printf ("+++ Jz47:  1.\n");
                jz47_memdev[memdev_count].paddr = paddr;
                printf ("+++ Jz47:  2.\n");
                jz47_memdev[memdev_count].totalsize = (pages * PAGE_SIZE);
                printf ("+++ Jz47:  3.\n");
                jz47_memdev[memdev_count].usedsize = 0;
                printf ("+++ Jz47:  4.\n");
                memdev_count++;
                printf ("Jz47 Dev alloc: vaddr = 0x%x, paddr = 0x%x, size = 0x%x ++\n",
                        vaddr, paddr, (pages * PAGE_SIZE));
        }
        return;
}

/*************************************************************************************
 JZ VPU Memory Alloc Function
*************************************************************************************/
void *JZMalloc(int align, int size)
{
        int i, alloc_size, used_size, total_size;
        unsigned int vaddr;

        /* use the valid align value.  */
        if (align >= 1024)  align = 1024;
        else if (align >= 512)  align = 512;
        else if (align >= 256)  align = 256;
        else if (align >= 128)  align = 128;
        else if (align >= 64)  align = 64;
        else if (align >= 32)  align = 32;
        else if (align >= 16)  align = 16;
        else if (align >= 8)  align = 8;
        else if (align >= 4)  align = 4;
        else if (align >= 2)  align = 2;
        else align = 1;

        /* alloc the memory.  */
        for (i = 0; i < MEM_ALLOC_DEV_NUM; i++) {
                if (i >= memdev_count) {
                        jz47_alloc_memdev();
                }
                used_size = jz47_memdev[i].usedsize;
                used_size = (used_size + align - 1) & ~ (align - 1);
                jz47_memdev[i].usedsize = used_size;
                // get memory size for allocated.  */
                total_size = jz47_memdev[i].totalsize;
                alloc_size = total_size - used_size;
                if (alloc_size >= size) {
                        /* if((((jz47_memdev[i].vaddr & 0xFFFFFF) + used_size)&0x1C00000) != */
                        /*    (((jz47_memdev[i].vaddr & 0xFFFFFF) + used_size + size) & 0x1C00000)){ */
                        /*         vaddr = ((jz47_memdev[i].vaddr + used_size + size) & 0xFFC00000); */
                        /*         jz47_memdev[i].usedsize = size + (vaddr -jz47_memdev[i].vaddr); */
                        /* }else{ */
                                jz47_memdev[i].usedsize = used_size + size;
                                vaddr = jz47_memdev[i].vaddr + used_size;
                        /* } */
                        return (void *)vaddr;
                }
        }

        printf ("++ JZ alloc: memory allocated is failed (size = %d) ++\n", size);
        return NULL;
}
