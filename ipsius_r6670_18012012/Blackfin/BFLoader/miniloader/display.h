#ifndef __display__
#define __display__


#include "macros.h"

void Lcd_send_char(byte data);
void Lcd_send_msg(byte *data, byte cf);
void wr_byte_hex(byte d, byte cf);
void wr_byte_dec(byte d, byte cf);
void wr_word_dec(word e, byte cf);
void wr_word_hex(word d, byte cf);

#endif


