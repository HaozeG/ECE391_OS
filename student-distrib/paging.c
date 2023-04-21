#include "paging.h"
#include "lib.h"
void page_init(uint32_t pid) {
    int i;
    uint32_t user_addr_physical;
    //memset(page_directory, 0, PAGE_SIZE*sizeof(*page_directory));
    process_paging[pid].page_directory[0].pde_page_table.present = 0;
    process_paging[pid].page_directory[0].pde_page_table.read_write = 0;
    process_paging[pid].page_directory[0].pde_page_table.user_supervisor = 0;
    process_paging[pid].page_directory[0].pde_page_table.write_through = 0;
    process_paging[pid].page_directory[0].pde_page_table.cache_disabled = 0;
    process_paging[pid].page_directory[0].pde_page_table.accessed = 0;
    process_paging[pid].page_directory[0].pde_page_table.page_size = 0; // page size is 4KiB
    process_paging[pid].page_directory[0].pde_page_table.global_page = 0;
    process_paging[pid].page_directory[0].pde_page_table.avail = 0;
    process_paging[pid].page_directory[0].pde_page_table.page_base_addr = 0;
    for (i = 1; i < PAGE_SIZE; i++) {
        process_paging[pid].page_directory[i].pde_4mb_page.present = 0; // set all bits to 0 for each page directory entry
        process_paging[pid].page_directory[i].pde_4mb_page.read_write = 1;
        process_paging[pid].page_directory[i].pde_4mb_page.user_supervisor = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.write_through = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.cache_disabled = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.accessed = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.dirty = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.page_size = 1; // page size is 4MiB
        process_paging[pid].page_directory[i].pde_4mb_page.global_page = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.avail = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.page_table_attribute_index = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.reserved = 0;
        process_paging[pid].page_directory[i].pde_4mb_page.page_base_addr = 0;
    }

    // map the kernel
    process_paging[pid].page_directory[KERNEL_ADDR >> 22].pde_4mb_page.present = 1;
    process_paging[pid].page_directory[KERNEL_ADDR >> 22].pde_4mb_page.user_supervisor = 0;
    process_paging[pid].page_directory[KERNEL_ADDR >> 22].pde_4mb_page.global_page = 1;
    process_paging[pid].page_directory[KERNEL_ADDR >> 22].pde_4mb_page.page_base_addr = KERNEL_ADDR >> 22;

    //map the video memory
    process_paging[pid].page_directory[VID_MEM_ADDR >> 22].pde_page_table.present = 1;
    process_paging[pid].page_directory[VID_MEM_ADDR >> 22].pde_page_table.read_write = 1;
    process_paging[pid].page_directory[VID_MEM_ADDR >> 22].pde_page_table.page_base_addr = (uint32_t) process_paging[pid].page_table >> 12;

    //map the page table pointed to by the video memory address
    for (i = 0; i < PAGE_SIZE; i++) {
        process_paging[pid].page_table[i].present = 0;
        process_paging[pid].page_table[i].read_write = 0;
        process_paging[pid].page_table[i].user_supervisor = 0;
        process_paging[pid].page_table[i].write_through = 0;
        process_paging[pid].page_table[i].cache_disabled = 1;
        process_paging[pid].page_table[i].accessed = 0;
        process_paging[pid].page_table[i].dirty = 0;
        process_paging[pid].page_table[i].page_table_attribute_index = 0;
        process_paging[pid].page_table[i].global_page = 0;
        process_paging[pid].page_table[i].avail = 0;
        process_paging[pid].page_table[i].page_base_addr = 0;
    }
    /*generically fill vidmap Page Table*/
    for (i = 0; i < PAGE_SIZE; i++){
        process_paging[pid].pte_vidmap[i].page_base_addr = 0;
        process_paging[pid].pte_vidmap[i].present = 0;
        process_paging[pid].pte_vidmap[i].read_write = 1;
        process_paging[pid].pte_vidmap[i].user_supervisor = 0;
        process_paging[pid].pte_vidmap[i].write_through = 0;
        process_paging[pid].pte_vidmap[i].cache_disabled = 1;
        process_paging[pid].pte_vidmap[i].accessed = 0;
        process_paging[pid].pte_vidmap[i].dirty = 0;
        process_paging[pid].pte_vidmap[i].page_table_attribute_index = 0;
        process_paging[pid].pte_vidmap[i].global_page = 0;
        process_paging[pid].pte_vidmap[i].avail = 0;
    }
    // set other fields of the pagetable
    process_paging[pid].page_table[VID_MEM_ADDR >> 12].present = 1;
    process_paging[pid].page_table[VID_MEM_ADDR >> 12].read_write = 1;
    process_paging[pid].page_table[VID_MEM_ADDR >> 12].page_base_addr = VID_MEM_ADDR >> 12;
    process_paging[pid].page_table[VID_MEM_TERM0 >> 12].present = 1;
    process_paging[pid].page_table[VID_MEM_TERM0 >> 12].read_write = 1;
    process_paging[pid].page_table[VID_MEM_TERM0 >> 12].page_base_addr = VID_MEM_TERM0 >> 12;
    process_paging[pid].page_table[VID_MEM_TERM1 >> 12].present = 1;
    process_paging[pid].page_table[VID_MEM_TERM1 >> 12].read_write = 1;
    process_paging[pid].page_table[VID_MEM_TERM1 >> 12].page_base_addr = VID_MEM_TERM1 >> 12;
    process_paging[pid].page_table[VID_MEM_TERM2 >> 12].present = 1;
    process_paging[pid].page_table[VID_MEM_TERM2 >> 12].read_write = 1;
    process_paging[pid].page_table[VID_MEM_TERM2 >> 12].page_base_addr = VID_MEM_TERM2 >> 12;
    // did not mark as present
    process_paging[pid].page_directory[(USER_ADDR_VIRTUAL + fourMB) >> 22].pde_page_table.page_size = 0;
    process_paging[pid].page_directory[(USER_ADDR_VIRTUAL + fourMB) >> 22].pde_page_table.user_supervisor = 1;
    process_paging[pid].page_directory[(USER_ADDR_VIRTUAL + fourMB) >> 22].pde_page_table.page_base_addr = ((uint32_t)process_paging[pid].pte_vidmap) >> 12; // shift from 32 bits to 20 bits
    //page of the vidmap page table
    process_paging[pid].pte_vidmap[0].present = 1;
    process_paging[pid].pte_vidmap[0].user_supervisor = 1;
    process_paging[pid].pte_vidmap[0].page_base_addr = VID_MEM_ADDR >> 12;   //video memory address

    // map user program image
    // only map one 4MB page: maximum file size less than 4MB
    user_addr_physical = (fourMB << 1) + (pid * fourMB);  // 8MB + (process number * 4MB)
    process_paging[pid].page_directory[USER_ADDR_VIRTUAL >> 22].pde_4mb_page.present = 1;
    process_paging[pid].page_directory[USER_ADDR_VIRTUAL >> 22].pde_4mb_page.user_supervisor = 1;
    process_paging[pid].page_directory[USER_ADDR_VIRTUAL >> 22].pde_4mb_page.page_base_addr = user_addr_physical >> 22;

    // TLB flushed by writing to cr3
    loadPageDirectory(process_paging[pid].page_directory);
    enablePaging();
}
