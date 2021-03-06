/* MANAGED-BY-SYSTEM-BUILDER                                    */
/* VisualDSP++ 5.0 Update 7                                     */
/* LDF Printer version: 5.7.0.3                                 */
/* ldfgen.exe version: 5.7.0.3                                  */
/* VDSG version: 5.7.0.3                                        */

/*
** User heap source file generated on Oct 07, 2010 at 14:11:36.
**
** Copyright (C) 2000-2009 Analog Devices Inc., All Rights Reserved.
**
** This file is generated automatically based upon the options selected
** in the LDF Wizard. Changes to the LDF configuration should be made by
** changing the appropriate options rather than editing this file.
**
** Configuration:-
**     crt_doj:                                BfFlashDriverTest_basiccrt.doj
**     processor:                              ADSP-BF537
**     product_name:                           VisualDSP++ 5.0 Update 7
**     si_revision:                            0.3
**     default_silicon_revision_from_archdef:  0.3
**     cplb_init_cplb_ctrl:                    63
**     cplb_init_dcache_ctrl:                  dcache_wt
**     cplb_init_cplb_src_file:                c:\eugene_v\Hg\trunk-hg\Blackfin\BfFlashDriverTest\BfFlashDriverTest_cplbtab.c
**     cplb_init_cplb_obj_file:                BfFlashDriverTest_cplbtab.doj
**     using_cplusplus:                        true
**     mem_init:                               false
**     use_vdk:                                true
**     use_eh:                                 true
**     use_argv:                               false
**     running_from_internal_memory:           true
**     user_heap_src_file:                     c:\eugene_v\Hg\trunk-hg\Blackfin\BfFlashDriverTest\BfFlashDriverTest_heaptab.c
**     search_path:                            "C:\Program Files\Analog Devices\VisualDSP 5.0\Blackfin\lib\src"
**     libraries_use_stdlib:                   true
**     libraries_use_fileio_libs:              true
**     libraries_use_ieeefp_emulation_libs:    false
**     libraries_use_eh_enabled_libs:          true
**     system_heap:                            L3
**     system_heap_min_size:                   512k
**     system_stack:                           L3
**     system_stack_size:                      32k
**     use_sdram:                              true
**     use_sdram_size:                         16MB
**     use_sdram_partitioned:                  default
**
*/


#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_1_1)
#pragma diag(suppress:misra_rule_2_2)
#pragma diag(suppress:misra_rule_6_3)
#pragma diag(suppress:misra_rule_8_10)
#pragma diag(suppress:misra_rule_10_1_a)
#pragma diag(suppress:misra_rule_11_3)
#pragma diag(suppress:misra_rule_12_7)
#endif /* _MISRA_RULES */



extern "asm" int ldf_heap_space;
extern "asm" int ldf_heap_length;


struct heap_table_t
{
  void          *base;
  unsigned long  length;
  long int       userid;
};

#pragma file_attr("libData=HeapTable")
#pragma section("constdata")
struct heap_table_t heap_table[2] =
{


  { &ldf_heap_space, (int) &ldf_heap_length, 0 },


  { 0, 0, 0 }
};



#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */


