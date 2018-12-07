#include "mouse.h"
#include "i8259.h"
#include "systemcall.h"
#include "paging.h"
#include "lib.h"


int32_t cycle;
int8_t mouse_bytes[3];
int mouse_x =0;
int mouse_y = 0;
void mouse_wait(unsigned char type)
{
    unsigned int _time_out=100000;
    if(type==0){
        while(_time_out--){ //Data
            if((inb(0x64) & 1)==1){
                return;
            }
        }
        return;
    }
    else{
        while(_time_out--){ //Signal
            if((inb(0x64) & 2)==0){
                return;
            }
        }
        return;
    }
}


uint32_t mouse_init(void){
    enable_irq(12);
    mouse_wait(1);
    outb(0xA8,0x64);
    mouse_wait(1);
    outb(0x20,0x64);

    unsigned char status_byte;//store status byte
    mouse_wait(0);
    //enabling the IRQ12
    status_byte = (inb(0x60) | 2);
    //status_byte = status_byte & 0x27; //enable mouse click & keyboard

    //send command 0x60 to 0x64 to modify status byte
    mouse_wait(1);
    outb(0x60, 0x64);

    mouse_wait(1);
    outb(status_byte,0x60);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();
    cycle = 1;
    return 1;

}

void do_mouse_handler(void){
    send_eoi(12);
    if(((mouse_bytes[0]>>6) == 0 )&& ((mouse_bytes[0]&0x08)==0)){
        cycle = 0;
    }
    mouse_bytes[cycle] = inb(0x60);
    // printf("mouse byte %d is %x\n",cycle,mouse_bytes[cycle] );
    cycle++;
    char x,y;

    if (cycle == 3) { // if we have all the 3 bytes
        cycle = 0; // reset the counter

        if ((mouse_bytes[0] & 0x80) || (mouse_bytes[0] & 0x40)){
        // printf("overflow!!!!!!!!!!!!!\n");
          x = 0;
          y = 0;
        }
        else{
            x = mouse_bytes[1];
            y = mouse_bytes[2];
        }

        restore_attribute(mouse_x,mouse_y);
        if(mouse_x+x/10>=0&&mouse_x+x/10<80){
            mouse_x+=x/10;
        }
        if(mouse_y-y/10>=0&&mouse_y-y/10<25){
            mouse_y-=y/10;
        }
        //printf("x movement %x\n",mouse_x);
        //printf("y movement %x\n",mouse_y);
        set_attribute(mouse_x,mouse_y);
        //if (mouse_bytes[0] & 0x4)
        //  printf("Middle\n");
        //if (mouse_bytes[0] & 0x2)
        //      printf("Right\n");
        //    if (mouse_bytes[0] & 0x1)
        //  printf("Left\n");
    }
}

int32_t mouse_write (unsigned char a_write)
{

    //Wait to be able to send a command
    mouse_wait(1);
    //Tell the mouse we are sending a command
    outb(0xD4,0x64);
    //Wait for the final part
    mouse_wait(1);
    //Finally write
    outb(a_write,0x60);
    return 1;
}


int32_t mouse_read (void)
{
    //Get response from mouse
    mouse_wait(0);
    return inb(0x60);
}

int32_t mouse_open (){
    /*
    cli();
    if(mouse_init() == -1){
        sti();
        return -1;// not success
    }
    sti();
    */
    return 0;
}

int32_t mouse_close (){
    return 0;
}
