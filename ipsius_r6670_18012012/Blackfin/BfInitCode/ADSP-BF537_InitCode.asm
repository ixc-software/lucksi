/********************************************************************/
/*  This file contains 3 sections:																	*/
/*																																	*/
/*	1) A Pre-Init Section - this section saves off all the					*/
/*		registers of the DSP.																					*/
/*																																	*/
/*	2) A Init Code Section - this section is the customer						*/
/*		initialization code which can be modified by the customer.		*/
/*		As an example, an SDRAM initialization code is supplied.			*/
/*																																	*/
/*	3) A Post-Init Section - this section restores all the					*/
/*		register from the stack.																			*/
/*																																	*/
/*	Customers should not modify the Pre-Init and Post-Init Sections.*/
/*	The Init Code Section can be modified for application use.			*/
/********************************************************************/

#include <defBF537.h>

.section program;

/*******Pre-Init Section*********************************************/
/*******DO NOT MODIFY************************************************/

[--SP] = ASTAT;			//Save Regs onto stack
	[--SP] = RETS;
	[--SP] = (r7:0);
	[--SP] = (p5:0);
	[--SP] = I0;
	[--SP] = I1;
	[--SP] = I2;
	[--SP] = I3;
	[--SP] = B0;
	[--SP] = B1;
	[--SP] = B2;
	[--SP] = B3;
	[--SP] = M0;
	[--SP] = M1;
	[--SP] = M2;
	[--SP] = M3;
	[--SP] = L0;
	[--SP] = L1;
	[--SP] = L2;
	[--SP] = L3;
/********************************************************************/

/****************************************************************
*							Init Code Section																	*
*			Please insert Initialization code in this section					*
****************************************************************/

/*******SDRAM Setup************/
Setup_SDRAM:
	P0.L = lo(EBIU_SDRRC); 
	P0.H = hi(EBIU_SDRRC); 		//SDRAM Refresh Rate Control Register
	R0 = 0x03A0(Z); 			 					
	W[P0] = R0;							
	SSYNC;	

	P0.L = lo(EBIU_SDBCTL); 			
	P0.H = hi(EBIU_SDBCTL); 	//SDRAM Memory Bank Control Register
//	r0 = 0x23(z);   
	r0 = 0x13(z);   
	[P0] = R0;
	SSYNC;		

	P0.L = lo(EBIU_SDGCTL); 			
	P0.H = hi(EBIU_SDGCTL);		//SDRAM Memory Global Control Register
	R0.L = 0x998D;
	R0.H = 0x0091;
	[P0] = R0;
	SSYNC;	
		
		
	P0.L = lo(PORTGIO_DIR); 			
	P0.H = hi(PORTGIO_DIR); 	//SET LED port as output
	R0 = 0x0070(Z); 			 						
	W[P0] = R0;
	SSYNC;		

	P0.L = lo(PORTGIO_SET); 			
	P0.H = hi(PORTGIO_SET); 	//SET LED to YELLOW
	R0 = 0x0070(Z); 			 						
	W[P0] = R0;
	SSYNC;					
	
	
	P0.L = lo(SPI_BAUD); 			
	P0.H = hi(SPI_BAUD); 	//set 10MHz speed for SPI
	R0 = 0x0003(Z); 			 						
	W[P0] = R0;
	SSYNC;		
	
/******************************/



/********************************************************************/

/*******Post-Init Section********************************************/
/*******DO NOT MODIFY************************************************/
	L3 = [SP++];		//Restore Regs from Stack
	L2 = [SP++];	
	L1 = [SP++];
	L0 = [SP++];
	M3 = [SP++];
	M2 = [SP++];
	M1 = [SP++];
	M0 = [SP++];
	B3 = [SP++];
	B2 = [SP++];
	B1 = [SP++];
	B0 = [SP++];
	I3 = [SP++];
	I2 = [SP++];
	I1 = [SP++];
	I0 = [SP++];
	(p5:0) = [SP++];  		
	(r7:0) = [SP++];
	RETS = [SP++];
	ASTAT = [SP++];
/********************************************************************/

	RTS;
