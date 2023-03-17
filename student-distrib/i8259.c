/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

// Mask to block all interrupts
#define INIT_MASK   0xFF

/*
*  i8259_init
*   DESCRIPTION: Initialize the 8259 PIC
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Reset master_mask that only enables SLAVE_IRQ,
*               reset slave_mask that masks all interrupts
*/
void i8259_init(void) {
    master_mask = INIT_MASK;
    slave_mask = INIT_MASK;

    // Mask interrupts
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);

    // Send ICWs
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);

    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    // enable mask on master for cascade
    enable_irq(SLAVE_IRQ);
}

/*
* enable_irq
*   DESCRIPTION: Enable (unmask) the specified IRQ
*   INPUTS: irq_num - IRQ number
*   RETURN VALUE: none
*   SIDE EFFECTS: Do nothing if irq_num not in [0, 15];
*           change master_mask/slave_mask
*/
void enable_irq(uint32_t irq_num) {
    // check irq_num
    if (irq_num >= MAX_IRQ_NUM) {
        return;
    }
    if (irq_num >= MAX_IRQ_NUM_MASTER) {
        slave_mask &= ~(1 << (irq_num - MAX_IRQ_NUM_MASTER));
        outb(slave_mask, SLAVE_8259_DATA);
        return;
    }
    master_mask &= ~(1 << irq_num);
    outb(master_mask, MASTER_8259_DATA);
}

/*
* disable_irq
*   DESCRIPTION: Disable (mask) the specified IRQ
*   INPUTS: irq_num - IRQ number
*   RETURN VALUE: none
*   SIDE EFFECTS: Do nothing if irq_num not in [0, 15];
*           change master_mask/slave_mask
*/
void disable_irq(uint32_t irq_num) {
    // check irq_num
    if (irq_num >= MAX_IRQ_NUM) {
        return;
    }
    if (irq_num >= MAX_IRQ_NUM_MASTER) {
        slave_mask |= (1 << (irq_num - MAX_IRQ_NUM_MASTER));
        outb(slave_mask, SLAVE_8259_DATA);
        return;
    }
    master_mask |= (1 << irq_num);
    outb(master_mask, MASTER_8259_DATA);
}

/*
* send_eoi
*   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
*   INPUTS: irq_num - IRQ number
*   RETURN VALUE: none
*   SIDE EFFECTS: Send EOI to slave and master if IRQ on slave PIC;
*           send EOI only to master if IRQ on master PIC
*/
void send_eoi(uint32_t irq_num) {
    // check irq_num
    if (irq_num >= MAX_IRQ_NUM) {
        return;
    }

    // 0-7: Master PIC
    // 8-15: Slave PIC
    if (irq_num >= MAX_IRQ_NUM_MASTER) {
        // -8: offset for slave PIC
        outb(EOI | (irq_num - MAX_IRQ_NUM_MASTER), SLAVE_8259_PORT);
        outb(EOI | SLAVE_IRQ, MASTER_8259_PORT);
        return;
    }
    outb(EOI | irq_num, MASTER_8259_PORT);
}
