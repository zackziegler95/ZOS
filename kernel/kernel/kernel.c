#include <stdio.h>
#include <stdint.h>

#include <kernel/multiboot.h>
#include <kernel/tty.h>
#include <kernel/setuppaging.h>
#include <kernel/gdt.h>
#include <kernel/pmmngr.h>
#include <kernel/vmmngr.h>
#include <kernel/kmalloc.h>
#include <kernel/intex.h>
#include <kernel/kb.h>
#include <kernel/timer.h>
#include <drivers/ata.h>
#include <drivers/fat32.h>
 
extern uint32_t kernelStart;
extern uint32_t kernelEnd;

void kernel_main(multiboot_info_t *mbd, unsigned int magic) {
    uint32_t kernelpages = init_paging((uint32_t)&kernelStart, (uint32_t)&kernelEnd);
    gdt_install();
    
    idt_install();
    isrs_install();
    irq_install();

    terminal_initialize();
    
    pmmngr_init(mbd, kernelpages);
    //printf("kernelStart: %x\n", (uint32_t)&kernelStart);
    //printf("kernelEnd: %x\n", (uint32_t)&kernelEnd);
    //printf("mem_lower (bytes): %x\n", mbd->mem_lower*1024);
    //printf("mem_upper (bytes): %x\n", mbd->mem_upper*1024);
    unmap_idmap(); // For now we'll do this here, beacuse mbd uses physical addresses, so we need the ID map still
    
    kmalloc_init();

    kb_install();
    timer_install();

    __asm__ __volatile__ ("sti");

    ide_initialize(0x1F0, 0x3F4, 0x170, 0x374, 0x000);

    unsigned int pnum = 1; // Just for now, assume that we want to use hdd and partition 5
    
    init_fat32(hdd, pnum);

    //const char* diskID = kmalloc(11);
    //memcpy(diskID, (unsigned char*) (ptr+0x1b4), 10);
    //printf("disk ID: %s", diskID);
    //kfree(diskID);

    /*printf("First 100 shorts = 200 bytes of the disk:\n");
    for (int i = 0; i < 100; i++) {
        printf("%x ", buffer[i]);
    }*/

    //uint32_t newaddr = pmmngr_alloc_block();
    //printf("new address: %x\n", newaddr);

    //print_map(0, 20);

    /*
    uint32_t* newpage = (uint32_t*)allocate_page();
    printf("newpage virtual address: %x, value: %x\n", newpage, *newpage);

    print_map(0, 20);

    uint32_t vaddr = (uint32_t) newpage;
    int pde_i = vaddr / 0x400000; // Each page table maps 4 MB = 0x400000
    int pte_i = (vaddr % 0x400000) / 0x1000;

    printf("new page directory index: %d, table index: %d\n", pde_i, pte_i);
    printf("new page physical address: %x\n", get_pagetable(pde_i)[pte_i]);
    *newpage = 0xDEADC0DE;
    free_page(newpage);

    print_map(0, 20);
    printf("newpage virtual address: %x, value: %x\n", newpage, *newpage);
    printf("page directory entry: %x\n", get_pagedir()[pde_i]);
    printf("page table entry: %x\n", get_pagetable(pde_i)[pte_i]);
    */

    /*uint32_t* newpage;
    for (int i = 0; i < 0x2D9; i++) {
        newpage = (uint32_t*)allocate_page();
    }

    printf("newpage virtual address: %x, value: %x\n", newpage, *newpage);   

    uint32_t* pd = get_pagedir();
    printf("pagedir[769]: %x\n\n", (uint32_t)(pd[769]));

    free_page(newpage);
    printf("pagedir[769]: %x\n\n", (uint32_t)(pd[769]));*/


    //print_map(0, 20);

    //NEXT UP, SET UP A SYSTEM TO MAP THE NEW PAGE INTO THE VIRTUAL ADDRESS SPACE, AND THINK ABOUT REORGANIZING MAYBE
}
