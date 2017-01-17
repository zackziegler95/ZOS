#ifndef _KERNEL_KMALLOC_H
#define _KERNEL_KMALLOC_H

#include <stdint.h>

void* kmalloc(uint16_t);
uint8_t kfree(void*);
void kmalloc_init(void);

#endif
