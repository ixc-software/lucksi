#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h> // register_chrdev
#include <linux/io.h> // readl/writel

#include <asm/uaccess.h>
//#include <asm/delay.h>
#include <mach/map.h>

//#include <asm/fasttimer.h>

//#include <linux/delay.h> //msleep
#include <linux/jiffies.h>
#include <linux/sched.h>

#include <linux/mutex.h>

#include "linux/moduleparam.h"
#include "linux/stat.h"

#include "ModArmIo.h"

// Module input parametrs (insmod ModArmIo.ko )
//----------------------------------------------------------------

int GModParMajor = 0;// if 0 - auto

// it`s bool, but macros module_param need int
int GModParTraceOn = 0;
int GModParDisableRead = 0;
int GModParDisableWrite = 0;

module_param_named(major, GModParMajor, int, S_IRUGO);
module_param_named(traceOn, GModParTraceOn, bool, S_IRUGO | S_IWUSR);
module_param_named(disableRead, GModParDisableRead, bool, S_IRUGO | S_IWUSR);
module_param_named(disableWrite, GModParDisableWrite, bool, S_IRUGO | S_IWUSR);

//----------------------------------------------------------------

DEFINE_MUTEX(GMutex);

//----------------------------------------

// config
static const unsigned long CPhysStart = S3C64XX_PA_GPIO;//0x7f008000; // GPIO start
static const unsigned long CPhysEnd = S3C64XX_PA_GPIO + S3C64XX_SZ_GPIO;//0x7f009000;   // GPIO end (next after last)

//unsigned long GJumpToVa = 0; // evaluate after remap

//----------------------------------------

static
void USleep(unsigned int usec)
{
    unsigned long timeout = usecs_to_jiffies(usec);
    schedule_timeout_uninterruptible(timeout);
}

//----------------------------------------

static 
void Dump(char *p, int count)
{		
    if (!GModParTraceOn) return;

    printk("Dump by addr %x begin:\n", p);
    if (count > 128 || count < 0) count = 64;

    int i;
    for(i = 0; i < count; ++i)
    {
        printk("%x ", p[i]);
    }
    printk("\n Dump end.");
}

//----------------------------------------

static
void Trace(struct UserDataItem* p, const char* msg)
{
    if (!GModParTraceOn) return;
    printk("%s;\n value=%lx,\n addr=%lx,\n cmd=%i,\n magic=%li\n\n", msg, p->m_value, p->m_addr, p->m_cmd, p->m_magic);
}

//----------------------------------------

static
void RunWrite(unsigned long addr, unsigned long value, unsigned long mask)
{
    if (GModParDisableWrite) return;

    if (mask != 0)
    {
        if (GModParDisableRead)
        {
            printk(KERN_WARNING "Cmd write with mask disabled in DebugNoRead mode");
            return;
        }

        unsigned long prevValue = __raw_readl(addr);
        prevValue &= ~mask; // clear
        value = prevValue | value; // add
    }    

    __raw_writel(value, addr);    
}

//----------------------------------------

static
enum ReturnCode RunCmd(struct UserDataItem* pData)
{
    Trace(pData, "RunCmd");

    if (pData->m_magic != CMagic) return retErr;
    if (GModParTraceOn) printk("CMagic Ok\n");

    // validate addr range
    if (pData->m_addr < CPhysStart && pData->m_addr >= CPhysEnd) return retErrRange;

    // pa --> va
    //unsigned long addr = pData->addr + GJumpToVa;
    //unsigned long addr = pData->addr + 0x754f8000;  // ok
    //unsigned long addr = phys_to_virt(pData->addr);
    unsigned long addr = pData->m_addr + (S3C64XX_VA_GPIO - S3C64XX_PA_GPIO); // ok

    if (pData->m_cmd == cmdRead)
    {
        if (!GModParDisableRead) pData->m_value = __raw_readl(addr);
    }
    else
        RunWrite(addr, pData->m_value, pData->m_mask);        

    if (pData->m_usleep != 0) USleep(pData->m_usleep);

    return retOk;
}

// module interface
//-----------------------------------------------------------------------------------------------------

static 
ssize_t Read (struct file *file, char __user*  user, size_t count, loff_t *offp)
{
    return 0;
}

//---------------------------------------

static
ssize_t Write (struct file *file, const char __user * user, size_t count, loff_t *offp)
{	
    return 0;
}

//---------------------------------------

static
int Open(struct inode* node, struct file* file)
{
    if (GModParTraceOn) printk("Drv Open \n");
    return 0;
}

//---------------------------------------

static
int Release(struct inode* node, struct file* file)
{

    if (GModParTraceOn) printk("Drv Release \n");
    return 0;
}

//---------------------------------------

static
int Ioctl(struct inode* i, struct file* f, unsigned int val, unsigned long pCmd)
{
    if (GModParTraceOn) printk("Ioctl(): Addr=%x\n", pCmd);
    Dump((char*)pCmd, sizeof(struct UserData));

    struct UserData* pData = (struct UserData*)pCmd;
    Trace(pData->m_pItem, "Input data as raw adress");

    if (pData->m_magic != CMagic) return retErrMagicNum;

    //if (pData->m_itemCount > CMaxCmdCount) return retErr;

    // procaess command list
    mutex_lock(&GMutex);

    int iter; enum ReturnCode ret;
    for (iter = 0; iter < pData->m_itemCount; ++iter)
    {
        struct UserDataItem* pItem = pData->m_pItem + iter;
        ret = RunCmd(pItem);
        if (ret != retOk) break;
    }

    mutex_unlock(&GMutex);

    Trace(pData->m_pItem, "Output data as raw adress");

    return ret;
}


//---------------------------------------

static 
struct file_operations GIntf = 
{
    llseek: 0,
    read: Read,
    write: Write,
    ioctl: Ioctl,
    open: Open,
    release: Release
};


//-----------------------------------------------------------------------------------------------------

int init_module() // insmod
{
    if (GModParTraceOn)
        printk(KERN_INFO "Mdule %s: call init_module \n", CName);

    struct file_operations *intf = &GIntf;
    int major = register_chrdev(GModParMajor, CName, intf);

    if (major < 0)
    {
        printk(KERN_WARNING "Can't register chardev");
        return -1;
    }

    if (GModParTraceOn)
        printk(KERN_INFO "%s registred. Major = %i.\n", CName, major);

    return 0;
}

void cleanup_module() // rmmod
{
    if (!GModParTraceOn) return;
        printk(KERN_INFO "Call cleanup_module");
}

MODULE_LICENSE("GPL"); // if undefined module was not linked
