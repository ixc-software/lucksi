/* MANAGED-BY-SYSTEM-BUILDER                                    */
/* VisualDSP++ 5.0 Update 6                                     */
/* LDF Printer version: 5.6.0.4                                 */
/* ldfgen.exe version: 5.6.0.4                                  */
/* VDSG version: 5.6.0.4                                        */

/*
** CPLB table definitions generated on Nov 25, 2009 at 11:58:05.
**
** Copyright (C) 2000-2008 Analog Devices Inc., All Rights Reserved.
**
** This file is generated automatically based upon the options selected
** in the LDF Wizard. Changes to the LDF configuration should be made by
** changing the appropriate options rather than editing this file.
**
** Configuration:-
**     crt_doj:                                BfRamTest_basiccrt.doj
**     processor:                              ADSP-BF537
**     product_name:                           VisualDSP++ 5.0 Update 6
**     si_revision:                            0.3
**     default_silicon_revision_from_archdef:  0.3
**     cplb_init_cplb_ctrl:                    34
**     cplb_init_dcache_ctrl:                  dcache_a_wt
**     cplb_init_cplb_src_file:                D:\proj\Ipsius\Blackfin\BfRamTest\BfRamTest_cplbtab.c
**     cplb_init_cplb_obj_file:                BfRamTest_cplbtab.doj
**     using_cplusplus:                        true
**     mem_init:                               false
**     use_vdk:                                false
**     use_eh:                                 true
**     use_argv:                               false
**     running_from_internal_memory:           true
**     user_heap_src_file:                     D:\proj\Ipsius\Blackfin\BfRamTest\BfRamTest_heaptab.c
**     libraries_use_stdlib:                   true
**     libraries_use_fileio_libs:              false
**     libraries_use_ieeefp_emulation_libs:    false
**     libraries_use_eh_enabled_libs:          false
**     system_heap:                            L1
**     system_heap_min_size:                   8k
**     system_stack:                           L1
**     system_stack_min_size:                  8k
**     use_sdram:                              false
**
*/


#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_2_2)
#pragma diag(suppress:misra_rule_8_10)
#pragma diag(suppress:misra_rule_10_1_a)
#endif /* _MISRA_RULES */



#define CACHE_MEM_MODE CPLB_DDOCACHE_WT



#include <sys/platform.h>
#include <cplbtab.h>

#pragma section("cplb_data")

cplb_entry dcplbs_table[] = {



/*$VDSG<customizable-data-cplb-table>                           */
/* This code is preserved if the CPLB tables are re-generated.  */


   // L1 Data A & B, (set write-through bit to avoid 1st write exceptions)
   {0xFF800000, (PAGE_SIZE_4MB | CPLB_DNOCACHE | CPLB_LOCK | CPLB_WT)}, 

   // Async Memory Bank 2 (Second)
   // Async Memory Bank 1 (Prim B)
   // Async Memory Bank 0 (Prim A)
   {0x20200000, (PAGE_SIZE_1MB | CPLB_DNOCACHE)}, 
   {0x20100000, (PAGE_SIZE_1MB | CACHE_MEM_MODE)}, 
   {0x20000000, (PAGE_SIZE_1MB | CACHE_MEM_MODE)}, 

      // 512 MB (Maximum) SDRAM memory space 
   {0x00000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE | CPLB_DIRTY | CPLB_LOCK)}, 
   {0x00400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE | CPLB_DIRTY | CPLB_LOCK)}, 
   {0x00800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE | CPLB_DIRTY | CPLB_LOCK)}, 
   {0x00C00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE | CPLB_DIRTY | CPLB_LOCK)}, 
   
   // CPLBs covering 496MB
   {0x01000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x01400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x01800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x01c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x02000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x02400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x02800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x02c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x03000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x03400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x03800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x03c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x04000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x04400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x04800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x04c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x05000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x05400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x05800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x05c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x06000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x06400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x06800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x06c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x07000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x07400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x07800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x07c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x08000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x08400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x08800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x08c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x09000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x09400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x09800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x09c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0a000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0a400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0a800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0ac00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0b000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0b400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0b800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0bc00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0c000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0c400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0c800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0cc00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0d000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0d400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0d800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0dc00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0e000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0e400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0e800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0ec00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0f000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0f400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0f800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x0fc00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x10000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x10400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x10800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x10c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x11000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x11400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x11800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x11c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x12000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x12400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x12800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x12c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x13000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x13400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x13800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x13c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x14000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x14400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x14800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x14c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x15000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x15400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x15800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x15c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x16000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x16400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x16800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x16c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x17000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x17400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x17800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x17c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x18000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x18400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x18800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x18c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x19000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x19400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x19800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x19c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1a000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1a400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1a800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1ac00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1b000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1b400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1b800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1bc00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1c000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1c400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1c800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1cc00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1d000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1d400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1d800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1dc00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1e000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1e400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1e800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1ec00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1f000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1f400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1f800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x1fc00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},

   // Async Memory Bank 3
   {0x20300000, (PAGE_SIZE_1MB | CPLB_DNOCACHE)}, 

   // end of section - termination
   {0xffffffff, 0}, 
/*$VDSG<customizable-data-cplb-table>                           */


}; /* dcplbs_table */

#pragma section("cplb_data")

cplb_entry icplbs_table[] = {



/*$VDSG<customizable-instr-cplb-table>                          */
/* This code is preserved if the CPLB tables are re-generated.  */


   // L1 Code
   {0xFFA00000, (PAGE_SIZE_1MB | CPLB_I_PAGE_MGMT)}, 

   // Async Memory Bank 2 (Secnd)
   // Async Memory Bank 1 (Prim B)
   // Async Memory Bank 0 (Prim A)
   {0x20200000, (PAGE_SIZE_1MB | CPLB_INOCACHE)}, 
   {0x20100000, (PAGE_SIZE_1MB | CPLB_IDOCACHE)}, 
   {0x20000000, (PAGE_SIZE_1MB | CPLB_IDOCACHE)}, 

   // 512 MB (Maximum) SDRAM memory space 
   
   // CPLBs covering 512MB
   {0x00000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x00400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x00800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x00c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x01000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x01400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x01800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x01c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x02000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x02400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x02800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x02c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x03000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x03400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x03800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x03c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x04000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x04400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x04800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x04c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x05000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x05400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x05800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x05c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x06000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x06400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x06800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x06c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x07000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x07400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x07800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x07c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x08000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x08400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x08800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x08c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x09000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x09400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x09800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x09c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0a000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0a400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0a800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0ac00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0b000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0b400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0b800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0bc00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0c000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0c400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0c800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0cc00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0d000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0d400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0d800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0dc00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0e000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0e400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0e800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0ec00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0f000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0f400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0f800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x0fc00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x10000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x10400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x10800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x10c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x11000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x11400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x11800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x11c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x12000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x12400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x12800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x12c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x13000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x13400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x13800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x13c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x14000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x14400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x14800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x14c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x15000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x15400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x15800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x15c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x16000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x16400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x16800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x16c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x17000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x17400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x17800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x17c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x18000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x18400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x18800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x18c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x19000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x19400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x19800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x19c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1a000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1a400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1a800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1ac00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1b000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1b400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1b800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1bc00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1c000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1c400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1c800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1cc00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1d000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1d400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1d800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1dc00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1e000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1e400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1e800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1ec00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1f000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1f400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1f800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x1fc00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},

   // Async Memory Bank 3 
   {0x20300000, (PAGE_SIZE_1MB | CPLB_INOCACHE)}, 

   // end of section - termination
   {0xffffffff, 0}, 
/*$VDSG<customizable-instr-cplb-table>                          */


}; /* icplbs_table */



#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */


