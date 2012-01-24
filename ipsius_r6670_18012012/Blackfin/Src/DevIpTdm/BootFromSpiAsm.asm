
#include "defBF537.h";

    .file "..\Src\DevIpTdm\BfBootSpiAsm.cpp";

    .section/DOUBLE32 program;

.epctext:
    .align 2;
_BootFromSpiAsm__FUlUs:
.LN_BootFromSpiAsm__FUlUs:
    

	CLI R2;
	
	P0.L = lo(PORTF_FER); 			
	P0.H = hi(PORTF_FER);
	//R0 = 0x3800(Z);
	R2 = 0x3800(Z);
	W[P0] = R2;//R0;
	SSYNC;					
		
    P0.H = HI(_BOOTROM_BOOT_DXE_SPI) ; 
    P0.L = LO(_BOOTROM_BOOT_DXE_SPI) ;
    
    R7 = R0;                // SPI address is zero 
    R6 = 10(z) ;            // SPI's /CS connects to PF4 pin 

    R5 = R1;                // SPI clock divider
    JUMP (P0);              // jump to Boot ROM 


    RTS;
.LN._BootFromSpiAsm__FUlUs.end:
._BootFromSpiAsm__FUlUs.end:
    .global _BootFromSpiAsm__FUlUs;
    .type _BootFromSpiAsm__FUlUs,STT_FUNC;

    .file_attr FuncName="_BootFromSpiAsm__FUlUs";
    .file_attr Content="Code";
.epctext.end:

    .section/DOUBLE32 data1;

    .align 4;
    .type .epcdata,STT_OBJECT;
.epc.cplus.compiled:
    .type .epc.cplus.compiled,STT_OBJECT;
    .byte .epcdata[1];
.epcdata.end:


//---

/*

#include "defBF537.h";

	.file ".\BootFromSpiAsm.cpp";

	.section/DOUBLE32 program;

.epctext:
	.align 2;
_BootFromSpiAsm__FPUlUs:
.LN_BootFromSpiAsm__FPUlUs:
	LINK 0;
	
	
	CLI R2;
	
	P0.L = lo(PORTF_FER); 			
	P0.H = hi(PORTF_FER);
	R0 = 0x3800(Z);
	W[P0] = R0;
	SSYNC;					
		
    P0.H = HI(_BOOTROM_BOOT_DXE_SPI) ; 
    P0.L = LO(_BOOTROM_BOOT_DXE_SPI) ;
    
    R7 = R0;                // SPI address is zero 
    R6 = 10(z);           // SPI's /CS connects to PF4 pin 

    R5 = R1;                // SPI clock divider 
    JUMP (P0);              // jump to Boot ROM 
   
    
    
    //   P0.H = HI(0xEF00000A) ;
	//   P0.L = LO(0xEF00000A) ;
   	//   R7 = [FP + 8] ;      
    //   R6 = PF4 (z) ;       
    //   R5 = W[FP + 12](Z) ; 
    //   JUMP (P0) ;           
    	
	
	UNLINK;
	RTS;
.LN._BootFromSpiAsm__FPUlUs.end:
._BootFromSpiAsm__FPUlUs.end:
	.global _BootFromSpiAsm__FPUlUs;
	.type _BootFromSpiAsm__FPUlUs,STT_FUNC;

	.file_attr FuncName="_BootFromSpiAsm__FPUlUs";
	.file_attr Content="Code";
.epctext.end:

	.section/DOUBLE32 data1;

	.align 4;
	.type .epcdata,STT_OBJECT;
.epc.cplus.compiled:
	.type .epc.cplus.compiled,STT_OBJECT;
	.byte .epcdata[1];
.epcdata.end:


*/