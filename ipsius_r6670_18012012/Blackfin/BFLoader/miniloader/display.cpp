#include "macros.h"
#include "display.h"
#include "uart0.h"


byte WinToDos(byte b)
{
//   if ((b >= 0xc0) && (b <= 0xef)) return(b - 0x40);
//   if (b >= 0xf0) return(b - 0x10);
   return(b);
}

//----------------------------------------------------------

void Lcd_send_msg(byte *data, byte cf)    //
{                                                         //
  while(*data) Lcd_send_char(WinToDos(*data++)); //
  if (cf)
  {
    Lcd_send_char(10);
    Lcd_send_char(13);    
  }
}                           

//----------------------------------------------------------
void Lcd_send_char(byte data)     //
{                                           //
  PutChar(data);        
}                                           //
//-------------------------------------------

void wr_byte_hex(byte d, byte cf)
{
  byte a = d>>4;
  d = d & 0x0F;
  if (a < 10) a = a + 48; else a = a + 55;
  if (d < 10) d = d + 48; else d = d + 55;
  Lcd_send_char(a);
  Lcd_send_char(d);
  if (cf)
  {
    Lcd_send_char(10);    
    Lcd_send_char(13);        
  }      
}
//------------------------------------------------------

void wr_word_hex(word d, byte cf)
{
  byte a = d>>12;
  byte b = (d&0x0F00)>>8;
  byte c = (d&0x00F0)>>4;
  d = d & 0x000F;
  if (a < 10) a = a + 48; else a = a + 55;
  if (b < 10) b = b + 48; else b = b + 55;
  if (c < 10) c = c + 48; else c = c + 55;
  if (d < 10) d = d + 48; else d = d + 55;
  Lcd_send_char(a);
  Lcd_send_char(b);
  Lcd_send_char(c);
  Lcd_send_char(d);
  if (cf)
  {
    Lcd_send_char(10);    
    Lcd_send_char(13);        
  }        
}

void wr_byte_dec(byte d, byte cf)
{
  byte a = d / 100;
  byte b;
  d = d - (a * 100);
  b = d / 10;
  d = d - (b * 10);
  if (a != 0) Lcd_send_char(a + 48); else Lcd_send_char(' ');
  if ((a|b) != 0) Lcd_send_char(b + 48); else Lcd_send_char(' ');
  Lcd_send_char(d + 48);
  if (cf)
  {
    Lcd_send_char(10);    
    Lcd_send_char(13);        
  }      

}

void wr_word_dec(word e, byte cf)
{
  byte a, b, c, d;
  a = e / 10000;
  e = e - (a * 10000);
  b = e / 1000;
  e = e - (b * 1000);
  c = e / 100;
  e = e - (c * 100);
  d = e / 10;
  e = e - (d * 10);
  if (a != 0) Lcd_send_char(a + 48); else Lcd_send_char(' ');
  if ((a|b) != 0) Lcd_send_char(b + 48); else Lcd_send_char(' ');
  if ((a|b|c) != 0) Lcd_send_char(c + 48); else Lcd_send_char(' ');
  if ((a|b|c|d) != 0) Lcd_send_char(d + 48); else Lcd_send_char(' ');
  Lcd_send_char(e + 48);
  if (cf)
  {
    Lcd_send_char(10);    
    Lcd_send_char(13);        
  }      

}
