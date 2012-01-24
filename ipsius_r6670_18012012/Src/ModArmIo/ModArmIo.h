#ifndef _MODARMIO_H_
#define _MODARMIO_H_

/* Not include this file to C++ code directly, only through ModArmIoDetail.h !*/

#include "SharedConsts.h"

enum Cmd {cmdRead = 0, cmdWrite = 1};

static
const unsigned long CMagic = 1234567890;

enum ReturnCode
{
    retOk = -1,
    retErr = -2,
    retErrRange = -3, // phisical address out of implemented range
    retErrMagicNum = -4
};

struct UserDataItem
{	
    unsigned long m_magic;
    unsigned long m_value; // input if write, output if read command
    unsigned long m_addr;
    unsigned long m_cmd;
    unsigned long m_mask; // it makes no difference if m_cmd == mai_cmdRead
    unsigned long m_usleep; // sleep in uSec
};

struct UserData
{
    unsigned long m_magic;
    struct UserDataItem* m_pItem;
    unsigned long m_itemCount;
};

#endif
