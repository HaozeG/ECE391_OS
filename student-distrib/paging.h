#ifndef PAGING_H
#define PAGING_H
#include "types.h"

#define PAGE_SIZE 1024
#define ALIGNMENT4KIB 4096
#ifndef ASM

typedef union direc_entry {
    struct {
        uint8_t present : 1;
        uint8_t read_write : 1;
        uint8_t user_supervisor :1;
        uint8_t write_through :1;
        uint8_t cache_disabled :1;
        uint8_t accessed :1;
        uint8_t dirty :1;
        uint8_t page_size :1;
        uint8_t global_page :1;
        uint8_t avail :3;
        uint8_t page_table_attribute_index :1;
        uint8_t reserved :9;
        uint8_t page_base_addr: 10;

    } pde_4mb_page __attribute__((packed));
        uint8_t present : 1;
        uint8_t read_write : 1;
        uint8_t user_supervisor :1;
        uint8_t write_through :1;
        uint8_t cache_disabled :1;
        uint8_t accessed :1;
        uint8_t dirty :1;
        uint8_t page_table_attribute_index :1;
        uint8_t global_page :1;
        uint8_t avail : 3;
        uint8_t page_base_addr: 20;
    struct {
        
    } pde_page_table __attribute__((packed));
} pde_t; 

typedef struct table_entry {
    uint8_t present : 1;
    uint8_t read_write : 1;
    uint8_t user_supervisor :1;
    uint8_t write_through :1;
    uint8_t cache_disabled :1;
    uint8_t accessed :1;
    uint8_t dirty :1;
    uint8_t page_table_attribute_index :1;
    uint8_t global_page :1;
    uint8_t avail :3;
    uint8_t page_base_addr :20;
} pte_t __attribute__((packed)); 

pde_t page_directory[PAGE_SIZE] __attribute__ ((aligned(ALIGNMENT4KIB))) //remember to align to 4096
pte_t page_table[PAGE_SIZE] __attribute__ ((aligned(ALIGNMENT4KIB)))


#endif /* ASM */

#endif /* _x86_DESC_H */
