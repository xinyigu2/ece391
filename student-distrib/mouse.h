#include "lib.h"
void mouse_wait(unsigned char type);
uint32_t mouse_init(void);
int32_t mouse_read (void);
int32_t mouse_write (unsigned char a_write);
int32_t mouse_open ();
int32_t mouse_close ();
void do_mouse_handler(void);
extern void mouse_handler();
