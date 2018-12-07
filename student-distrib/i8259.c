/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* i8259_init
Input: none
Output: none
Return value: none
Description: Initialize the 8259 PIC
*/
void i8259_init(void) {
	outb(mask_all,MASTER_8259_PORT+0x01); /*mask all MASTER interrupt*/
	outb(mask_all,SLAVE_8259_PORT+0x01); /*mask all SLAVE interrupt*/

	outb(ICW1, MASTER_8259_PORT);	/* ICW1: select 8259A-1 init */
	outb(ICW2_MASTER, MASTER_8259_PORT+0x01);	/* ICW2: 8259A-1 IR0-7 mapped to 0x20-0x27 */
	outb(ICW3_MASTER,  MASTER_8259_PORT+0x01);	/* 8259A-1 (the master) has a slave on IR2 */
    outb(ICW4, MASTER_8259_PORT+0x01); /*ICW4*/

	outb(ICW1, SLAVE_8259_PORT);	/* ICW1: select 8259A-2 init */
	outb(ICW2_SLAVE, SLAVE_8259_PORT+0x01);	/* ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f */
	outb(ICW3_SLAVE, SLAVE_8259_PORT+0x01);	/* 8259A-2 is a slave on master's IR2 */
	outb(ICW4, SLAVE_8259_PORT+0x01); /*ICW4*/
	is_invalid_irq_input = 0; // all inputs are valid
    //slave in in master's IRQ2
    enable_irq(ICW3_SLAVE);
}

/* enable_irq
Input: irq_num: irq port to enable
Output: none
Return value: none
Description: Enable (unmask) the specified IRQ
*/
void enable_irq(uint32_t irq_num) {
    //unsigned int temp;

    if(irq_num > MAX_NUM_IRQ){ // if input is greater than maximum irq
		is_invalid_irq_input = 1; //  irq number not valid
		return;
    }
    if(irq_num <= MAX_IRQ_MASTER){//master
        //master_mask = inb(MASTER_8259_PORT+0x01); //get old IMR
        master_mask = master_mask & ~(1<<irq_num); // unmask selected IRQ
        outb(master_mask,MASTER_8259_PORT+0x01); // outb MASK
        return;
    }else{//slave
		irq_num -= MAX_IRQ_MASTER + 0x01;
        slave_mask = slave_mask & ~(1<<irq_num); // unmask selected IRQ
        outb(slave_mask,SLAVE_8259_PORT+0x01); // outb MASK
        return;
    }
}

/* disable_irq
Input: irq_num: irq port to disable
Output: none
Return value: none
Description: Disable (mask) the specified IRQ
*/
void disable_irq(uint32_t irq_num) {
    //unsigned int temp;

    if(irq_num > MAX_NUM_IRQ){ // if input is greater than maximum irq
		is_invalid_irq_input = 1; //  irq number not valid
	    return;
    }
    if(irq_num <= MAX_IRQ_MASTER){//master
        //master_mask = inb(MASTER_8259_PORT+0x01); //get old IMR
        master_mask = master_mask  | (1<<irq_num); // mask selected IRQ
        outb(master_mask,MASTER_8259_PORT+0x01); // outb MASK
        return;
    }else{//slave
		irq_num -= MAX_IRQ_MASTER + 0x01;
        //slave_mask = inb(SLAVE_8259_PORT+0x01); //get old IMR
        slave_mask = slave_mask  | (1<<irq_num); // mask selected IRQ
        outb(slave_mask,SLAVE_8259_PORT+0x01); // outb MASK
        return;
    }
}

/* send_eoi
Input: irq_num: irq port to send eoi to
Output: none
Return value: none
Description: Send end-of-interrupt signal for the specified IRQ
*/
void send_eoi(uint32_t irq_num) {
    if(irq_num > MAX_NUM_IRQ){ // if input is greater than maximum irq
		is_invalid_irq_input = 1; //  irq number not valid
		return;
    }
    if(irq_num > MAX_IRQ_MASTER){//slave
        outb((EOI | ICW3_SLAVE),MASTER_8259_PORT); // send eoi | 0x02 to master
		irq_num -= MAX_IRQ_MASTER + 0x01;
		outb((EOI | irq_num),SLAVE_8259_PORT); // send slave irq | eoi to slave
    }else{
		outb((EOI | irq_num),MASTER_8259_PORT); //send master irq | eoi to master
	}
    return;
}
