
#if !defined(__COMMANDS__)

#define __COMMANDS__

  #include "macros.h"


  #define BOOTER_FLAG                 (ADMUX)

  // -------------------------------------------

  // command opcodes
  #define COM_SYNC                    0xFF
  #define COM_EXIT                    0xFE
  #define COM_ERROR                   0x80

  #define COM_READ_BUFF               0x01
  #define COM_READ_TO_BUFF            0x02
  #define COM_WRITE_BUFF              0x03
  #define COM_WRITE_FROM_BUFF         0x04

  #define COM_GET_ID                  0x10
  #define COM_SET_SPEED               0x11
  #define COM_BOOT_VER                0x12
    
  #define COM_RESET                   0x20

  #define COM_EF_CHECK                0x30
  #define COM_EF_READ_TO_BUFF         0x31
  #define COM_EF_WRITE_FROM_BUFF      0x32

  #define COM_TRANSFER_ON             0x40
  #define COM_TRANSFER_OFF            0x41
  #define COM_EXT_TRANSF_NOPE         0x45
  #define COM_EXT_TRANSF_MODE1        0x46
  #define COM_EXT_TRANSF_MODE2        0x47
  
  #define ECOM_GET_INFO               0x60
  #define ECOM_DEVELOP                0x61
  #define ECOM_WRITE_TO_BUFF          0x63
  #define ECOM_WRITE_FROM_BUFF        0x64
  #define ECOM_WRITE_EEPROM           0x65


#endif
