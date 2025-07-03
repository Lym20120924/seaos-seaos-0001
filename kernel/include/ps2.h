#ifndef PS2_H
#define PS2_H

void ps2_init(void);
void ps2_keyboard_reset(void);
void ps2_mouse_reset(void);
void ps2_keyboard_handler(void);
void ps2_mouse_handler(void);

#endif