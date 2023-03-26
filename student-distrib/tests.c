#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "wrapper.h"
#include "paging.h"
#include "filesys.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER \
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result) \
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure()
{
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test()
{
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i)
	{
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL))
		{
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

/*
 * Divide zero test
 *   DESCRIPTION: Test exception 0
 *   INPUTS: none
 *	OUTPUTS: none
 *   SIDE EFFECTS: stuck in loop if handled correctly
 *	Coverage: IDT entry 0
 */
int divide_zero_test()
{
	//TEST_HEADER;
	int i = 0;
	int j = 5;
	j = 5 / i;
	return 1;
}

/*
 * System call test
 *   DESCRIPTION: Test system call
 *   INPUTS: none
 *	OUTPUTS: none
 *   SIDE EFFECTS: stuck in loop if handled correctly
 *	Coverage: IDT entry 0x80(system call)
 */
int syscall_test()
{
	//TEST_HEADER;
	ECE391_TEMP();
	return 1;
}

/*
 * Keyboard Interrupt test
 *   DESCRIPTION: Test keyboard interrupt
 *   INPUTS: none
 *	OUTPUTS: none
 *   SIDE EFFECTS: stuck in dead loop
 *	Coverage: IDT entry 0x21 (keyboard vector)
 */
int keyboard_test()
{
	//TEST_HEADER;
	clear();
	do
	{
		// printf("here!\n");
	} while (1);

	return 1;
}

int paging_general_test()
{
	TEST_HEADER;
	printf("passed");
	// char ret;
	// char *ptr;
	// ptr = (char*) KERNEL_ADDR;
	// ret = *ptr;
	// ptr = (char*) VID_MEM_ADDR;
	// ret = *ptr;
	printf("passed");
	return PASS;

}

int paging_fail_test()
{
	TEST_HEADER;
	int ret;
	int* ptr = (int*) 0x1;
	ret = *ptr;
	printf("passed");
	return PASS;

}

int paging_null_test()
{
	TEST_HEADER;
	int ret;
	int* ptr = (int*) 0;
	ret = *ptr;
	printf("passed");
	return PASS;

}
// add more tests here

/* Checkpoint 2 tests */
int file_general_test()
{
	TEST_HEADER;
	clear();
	uint8_t fname[] = "frame0.txt";
	directory_entry_t dentry; 
	if(read_dentry_by_name(fname, &dentry) == 0) {
		printf("passed\n");
		return PASS;
	} else {
		printf("did not pass, read_dentry_by_name did not copy the name properly \n");
		return FAIL;
	}

}

int file_name_test()
{
	TEST_HEADER;
	uint8_t fname[] = "youWILLNOTPASS.txt";
	directory_entry_t dentry; 
	if(read_dentry_by_name(fname, &dentry) == 0) {
		printf("passed \n");
		return PASS;
	} else {
		printf("did not pass, read_dentry_by_name did not copy the name properly \n");
		return FAIL;
	}

}

int long_name_test()
{
	TEST_HEADER;
	uint8_t fname[] = "LONGNAMEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE.txt";
	directory_entry_t dentry; 
	if(read_dentry_by_name(fname, &dentry) == 0) {
		printf("passed \n");
		return PASS;
	} else {
		printf("did not pass, read_dentry_by_name did not copy the name properly \n");
		return FAIL;
	}

}

int name_test()
{
	TEST_HEADER;
	int i;
	uint8_t fname[] = "frame0.txt";
	directory_entry_t dentry; 
	read_dentry_by_name(fname, &dentry);
	for (i = 0; i < strlen((int8_t*) fname); i++) {
		if (fname[i] != dentry.file_name[i]) {
			printf("did not pass, copied name is not equivalent. \n");
			return FAIL;
		}
	}
	printf("passed \n");
	return PASS;
}

int entire_fish_test()
{
	clear();
	TEST_HEADER;
	int i;
	int res;
	uint8_t buf[200];
	uint8_t fname[] = "frame0.txt";
	directory_entry_t dentry; 
	read_dentry_by_name(fname, &dentry);
	res = read_data(dentry.inode_num, 0, buf, 200);
	printf("bytes read: %d \n", res);
	printf("inode num: %d", dentry.inode_num);
	printf("\n");
	for(i = 0; i < res; i++) {
		printf("%c", buf[i]);
	}
	return PASS;
	
}

int partial_fish_offset_test()
{
	clear();
	TEST_HEADER;
	int i;
	int res;
	uint8_t buf[200];
	uint8_t fname[] = "frame0.txt";
	directory_entry_t dentry; 
	read_dentry_by_name(fname, &dentry);
	res = read_data(dentry.inode_num, 60, buf, 200);
	printf("bytes read: %d \n", res);
	printf("inode num: %d", dentry.inode_num);
	printf("\n");
	for(i = 0; i < res; i++) {
		printf("%c", buf[i]);
	}
	return PASS;
	
}

int partial_fish_byteRead_test()
{
	clear();
	TEST_HEADER;
	int i;
	int res;
	uint8_t buf[200];
	uint8_t fname[] = "frame0.txt";
	directory_entry_t dentry; 
	read_dentry_by_name(fname, &dentry);
	res = read_data(dentry.inode_num, 0, buf, 100);
	printf("bytes read: %d \n", res);
	printf("inode num: %d", dentry.inode_num);
	printf("\n");
	for(i = 0; i < res; i++) {
		printf("%c", buf[i]);
	}
	return PASS;
	
}

int verylongname()
{
	clear();
	TEST_HEADER;
	int i;
	uint8_t fname[] = "verylargetextwithverylongname.txt";
	directory_entry_t dentry; 
	if(read_dentry_by_name(fname, &dentry) == 0) {
		printf("passed\n");
		for (i = 0; i < 32; i++) {
			printf("%c", dentry.file_name[i]);
		}
		return PASS;
	} else {
		printf("did not pass, read_dentry_by_name did not copy the name properly \n");
		return FAIL;
	}
	
}

int fish_binary() // 36164 bytes
{
	clear();
	//TEST_HEADER;
	int i;
	int res;
	uint8_t buf[40000];
	uint8_t fname[] = "fish";
	directory_entry_t dentry; 
	read_dentry_by_name(fname, &dentry);
	res = read_data(dentry.inode_num, 4096, buf, 48);
	// printf("bytes read: %d \n", res);
	// printf("inode num: %d", dentry.inode_num);
	// printf("\n");
	for(i = 0; i < res; i++) {
		if (i % 16 == 0 && i != 0) {
			printf("\n");
		}
		if (buf[i] == 0 || buf[i] == 1  || buf[i] == 2 || buf[i] == 3 || buf[i] == 4 || buf[i] == 5 || buf[i] == 6 || buf[i] == 7 || buf[i] == 8 || buf[i] == 9) {
			printf("%x",0);
			printf("%x",buf[i]);
		} else {
			printf("%x", buf[i]);
		}
	}
	return PASS;
}

int verylongprint() // verylong file should have 5277 bytes of information
{
	clear();
	TEST_HEADER;
	int i;
	int res;
	uint8_t buf[5500];
	uint8_t fname[] = "verylargetextwithverylongname.txt";
	directory_entry_t dentry; 
	read_dentry_by_name(fname, &dentry);
	res = read_data(dentry.inode_num, 5200, buf, 5500);
	printf("bytes read: %d \n", res);
	printf("inode num: %d", dentry.inode_num);
	printf("\n");
	for(i = 0; i < res; i++) {
		printf("%c", buf[i]);
	}
	printf("TO ENSURE THAT THE NEWLINE CHARACTER WAS INCLUDED");
	return PASS;
}

int cat_binary() // 36164 bytes
{
	clear();
	//TEST_HEADER;
	int i;
	int res;
	uint8_t buf[40000];
	uint8_t fname[] = "cat";
	directory_entry_t dentry; 
	read_dentry_by_name(fname, &dentry);
	res = read_data(dentry.inode_num, 0x1520, buf, 1000);
	// printf("bytes read: %d \n", res);
	// printf("inode num: %d", dentry.inode_num);
	// printf("\n");
	for(i = 0; i < res; i++) {
		if (i % 16 == 0 && i != 0) {
			printf("\n");
		}
		if (buf[i] == 0 || buf[i] == 1  || buf[i] == 2 || buf[i] == 3 || buf[i] == 4 || buf[i] == 5 || buf[i] == 6 || buf[i] == 7 || buf[i] == 8 || buf[i] == 9) {
			printf("%x",0);
			printf("%x",buf[i]);
		} else {
			printf("%c", buf[i]);
		}
	}
	return PASS;
}
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests()
{
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("divide zero", divide_zero_test());
	//TEST_OUTPUT("keyboard test", keyboard_test());
	//TEST_OUTPUT("paging_general_test", keyboard_test());
	//TEST_OUTPUT("paging_fail_test", keyboard_test());
	//TEST_OUTPUT("paging_null_test", keyboard_test());
	//TEST_OUTPUT("filesys_general_test", file_general_test());
	//TEST_OUTPUT("filesys_name_doesn't_exist_test", file_name_test());
	//TEST_OUTPUT("filesys_long_name_test", long_name_test());
	//TEST_OUTPUT("filesys_name_comparison_test", name_test());
	//TEST_OUTPUT("filesys_read_data_test", entire_fish_test());
	//TEST_OUTPUT("filesys_read_data_test", partial_fish_offset_test());
	//TEST_OUTPUT("filesys_read_data_test", partial_fish_byteRead_test());
	//TEST_OUTPUT("filesys_read_data_test", verylongname());
	//TEST_OUTPUT("filesys_read_data_test", fish_binary());
	//TEST_OUTPUT("filesys_read_data_test", verylongprint());
	//verylongprint();
	//fish_binary();
	cat_binary();
	// launch your tests here
}
