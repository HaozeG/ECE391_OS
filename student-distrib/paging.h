#ifndef PAGING_H
#define PAGING_H
#include "types.h"

#define NUM_PROCESS_MAX 8

#define PAGE_SIZE 1024
#define ALIGNMENT4KIB 4096
#define VID_MEM_ADDR 0xB8000
#define VID_MEM_TERM0 0xB9000
#define VID_MEM_TERM1 0xBA000
#define VID_MEM_TERM2 0xBB000
#define KERNEL_ADDR 0x400000
#define USER_ADDR_VIRTUAL 0x8000000 // start at 128MB
#define fourKB 0x01000
#define fourMB 0x400000
#define VID_MEM_ADDR_MODEX 0xA0000	// 0xA0000 - 0xAFFFF
#define VID_MEM_SIZE_MODEX 0x20	// (2 * 0x10000 / 0x1000(4KiB)) double_buffer
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
}__attribute__((packed)) pte_t;

// pack up pd, pt for each process
typedef struct {
    pde_t page_directory[PAGE_SIZE];
    pte_t page_table[PAGE_SIZE];
    pte_t pte_vidmap[PAGE_SIZE];
}__attribute__((packed)) process_paging_t;

process_paging_t process_paging[NUM_PROCESS_MAX] __attribute__((aligned(ALIGNMENT4KIB)));
// pde_t page_directory[PAGE_SIZE] __attribute__ ((aligned(ALIGNMENT4KIB))); //remember to align to 4096
// pte_t page_table[PAGE_SIZE] __attribute__ ((aligned(ALIGNMENT4KIB)));



extern void page_init(uint32_t pid);
extern void loadPageDirectory(pde_t*);
extern void enablePaging();
extern void paging_init_mode_X(uint32_t pid);

#endif /* ASM */

#endif /* PAGING_H */
