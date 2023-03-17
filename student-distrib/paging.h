#ifndef PAGING_H
#define PAGING_H
#include "types.h"

#define PAGE_SIZE 1024
#define ALIGNMENT4KIB 4096
#define VID_MEM_ADDR 0xB8000
#define KERNEL_ADDR 0x400000
#ifndef ASM

typedef union direc_entry {
    struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor :1;
        uint32_t write_through :1;
        uint32_t cache_disabled :1;
        uint32_t accessed :1;
        uint32_t dirty :1;
        uint32_t page_size :1;
        uint32_t global_page :1;
        uint32_t avail :3;
        uint32_t page_table_attribute_index :1;
        uint32_t reserved :9;
        uint32_t page_base_addr: 10;

    } pde_4mb_page __attribute__((packed));

    struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor :1;
        uint32_t write_through :1;
        uint32_t cache_disabled :1;
        uint32_t accessed :1;
        uint32_t reserved :1;
        uint32_t page_size :1;
        uint32_t global_page :1;
        uint32_t avail : 3;
        uint32_t page_base_addr: 20;
    } pde_page_table __attribute__((packed));
} pde_t; 

typedef struct table_entry {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor :1;
    uint32_t write_through :1;
    uint32_t cache_disabled :1;
    uint32_t accessed :1;
    uint32_t dirty :1;
    uint32_t page_table_attribute_index :1;
    uint32_t global_page :1;
    uint32_t avail :3;
    uint32_t page_base_addr :20;
} pte_t __attribute__((packed)); 

pde_t page_directory[PAGE_SIZE] __attribute__ ((aligned(ALIGNMENT4KIB))); //remember to align to 4096
pte_t page_table[PAGE_SIZE] __attribute__ ((aligned(ALIGNMENT4KIB)));



extern void page_init();
extern void loadPageDirectory(unsigned int*);
extern void enablePaging();

#endif /* ASM */

#endif /* _x86_DESC_H */
