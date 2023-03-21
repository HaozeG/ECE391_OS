#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "wrapper.h"
#include "idt.h"

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
 * Asserts that all IDT entries are not NULL; check assignment of present, size, seg_selector, dpl are correct
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test()
{
	clear();
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < NUM_VEC; ++i)
	{
		// Check existence of handler(All are connected with specific handlers or reserved ones)
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL))
		{
			assertion_failure();
			result = FAIL;
		}
		if (i < NUM_EXCEPTION || i == KEYBOARD_VEC || i == SYS_CALL_VEC) {
		// if (i < NUM_EXCEPTION || i == KEYBOARD_VEC || i == SYS_CALL_VEC || i == RTC_VEC) {
			if (idt[i].present != 1) {
				assertion_failure();
				result = FAIL;
			}
		} else {
			if (idt[i].present != 0) {
				assertion_failure();
				result = FAIL;
			}
		}
		if (idt[i].seg_selector != KERNEL_CS || idt[i].size != 1) {
			assertion_failure();
			result = FAIL;
		}
		if (i == 0x80) {
			if (idt[i].dpl != 3) {
				assertion_failure();
				result = FAIL;
			}
		} else {
			if (idt[i].dpl != 0) {
				assertion_failure();
				result = FAIL;
			}
		}
	}
	return result;
}

/*
 * int test
 *   DESCRIPTION: Test certain interrupt handling
 *   INPUTS: none
 *	 OUTPUTS: none
 *   SIDE EFFECTS: stuck in loop if handled correctly
 *	Coverage: all IDT entry
 */
int int_test() {
	clear();
	TEST_HEADER;
	asm volatile("int $1");

	// should never reach here
	printf("Returned from interrupt handler!\n");
	return 1;
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
	clear();
	TEST_HEADER;
	int i = 0;
	int j = 5;
	j = 5 / i;
	// should never reach here
	printf("Divide zero executed!\n");
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
	clear();
	TEST_HEADER;
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
	clear();
	TEST_HEADER;
	do
	{
		// printf("here!\n");
	} while (1);

	return 1;
}

/*
 * Dereference test
 *   DESCRIPTION: Test the borders of paging; Test page fault exception
 *   INPUTS: none
 *	 OUTPUTS: none
 *   SIDE EFFECTS: Stuck in page fault handler
 *	Coverage: IDT entry 15; Paging
 */
int dereference_test()
{
	clear();
	TEST_HEADER;
	uint32_t b;
	uint32_t *addr;
	uint32_t i;
	
	// Start of video mem
	addr = (uint32_t *)0xB8000;
	printf("Test addr %x\n", addr);
	b = *addr;

	// Byte-addressable, but reads out 4 consecutive bytes
	for (i = 1; i <= (4*1024 - 4); i++) {
		addr = (uint32_t *)(0xB8000 + i);
		printf("Test addr %x\n", addr);
		b = *addr;
	}

	// Start of kernel page(4MB page)
	addr = (uint32_t *)0x400000;
	printf("Test addr %x\n", addr);
	b = *addr;
	// End of kernel page(4MB page)
	addr = (uint32_t *)(8*1024*1024 - 4);
	printf("Test addr %x\n", addr);
	b = *addr;

	// ------

	// // dereference accessible addr
	// addr = &b;
	// printf("Test addr %x\n", addr);
	// b = *addr;

	// dereference non-accessible addr 0
	addr = (uint32_t *)0;
	printf("Dereference addr 0\n");
	b = *addr;
	// should never reach here
	printf("FAIL! Dereferenced address 0!\n");

	return PASS;
}

// add more tests here

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests()
{
	// TEST_OUTPUT("idt_test", idt_test());
	TEST_OUTPUT("dereference test", dereference_test());
	// TEST_OUTPUT("int_test", int_test());
	// TEST_OUTPUT("divide zero", divide_zero_test());
	// TEST_OUTPUT("keyboard test", keyboard_test());
}
