#include "paging.h"

void page_init() {
    int i;
    //memset(page_directory, 0, PAGE_SIZE*sizeof(*page_directory));
    page_directory[0].pde_page_table.present = 0;
    page_directory[0].pde_page_table.read_write = 0;
    page_directory[0].pde_page_table.user_supervisor = 0;
    page_directory[0].pde_page_table.write_through = 0;
    page_directory[0].pde_page_table.cache_disabled = 0;
    page_directory[0].pde_page_table.accessed = 0;
    page_directory[0].pde_page_table.page_size = 0; // page size is 4KiB
    page_directory[0].pde_page_table.global_page = 0;
    page_directory[0].pde_page_table.avail = 0;
    page_directory[0].pde_page_table.page_base_addr = 0;
    for (i = 1; i < PAGE_SIZE; i++) {
        page_directory[i].pde_4mb_page.present = 0; // set all bits to 0 for each page directory entry
        page_directory[i].pde_4mb_page.read_write = 1;
        page_directory[i].pde_4mb_page.user_supervisor = 0;
        page_directory[i].pde_4mb_page.write_through = 0;
        page_directory[i].pde_4mb_page.cache_disabled = 0;
        page_directory[i].pde_4mb_page.accessed = 0;
        page_directory[i].pde_4mb_page.dirty = 0;
        page_directory[i].pde_4mb_page.page_size = 1; // page size is 4MiB
        page_directory[i].pde_4mb_page.global_page = 0;
        page_directory[i].pde_4mb_page.avail = 0;
        page_directory[i].pde_4mb_page.page_table_attribute_index = 0;
        page_directory[i].pde_4mb_page.reserved = 0;
        page_directory[i].pde_4mb_page.page_base_addr = 0;
    }

    // map the kernel
    page_directory[KERNEL_ADDR >> 22].pde_4mb_page.present = 1;
    page_directory[KERNEL_ADDR >> 22].pde_4mb_page.user_supervisor = 0;
    page_directory[KERNEL_ADDR >> 22].pde_4mb_page.page_base_addr = KERNEL_ADDR >> 22;  

    //map the video memory
    page_directory[VID_MEM_ADDR >> 22].pde_page_table.present = 1;
    page_directory[VID_MEM_ADDR >> 22].pde_page_table.read_write = 1;
    page_directory[VID_MEM_ADDR >> 22].pde_page_table.page_base_addr = (uint32_t) page_table >> 22;

    //map the page table pointed to by the video memory address
    for (i = 0; i < PAGE_SIZE; i++) {
        page_table[i].present = 0;
        page_table[i].read_write = 0;
        page_table[i].user_supervisor = 0;
        page_table[i].write_through = 0;
        page_table[i].cache_disabled = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].page_table_attribute_index = 0;
        page_table[i].global_page = 0;
        page_table[i].avail = 0;
        page_table[i].page_base_addr = 0;
    }
    page_table[VID_MEM_ADDR >> 12].present = 1;
    page_table[VID_MEM_ADDR >> 12].read_write = 1;
    page_table[VID_MEM_ADDR >> 12].page_base_addr = VID_MEM_ADDR >> 12;
    // set other fields of the pagetable

}
