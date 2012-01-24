#ifndef __PSB2170REG__
#define __PSB2170REG__

namespace PsbCofidec
{	
	enum AceRegAddress
    {
		SCTL = 0x60,
		SATT1 = 0x72,
		SATT2 = 0x73,

		SLGA = 0x7e,
        SAELEN = 0x80,
        SAEATT = 0x81,
        SAEGS = 0x82,
        SAEPS = 0x83,
		SAEBL = 0x84,

		SAEWFL = 0x85,
        SAEWFT = 0x86,

		LECCTL = 0x25,
        LECLEV = 0x26,
        LECATT = 0x27,
        LECMGN = 0x28,

		REV = 0x00,
		CCTL = 0x01,
        AFECTL = 0x03,
        IFS2 = 0x07,	//afe ch2
        IFS3 = 0x0c,	//iom ch1
		SDCONF = 0x0a,
        SDCHN1 = 0x0b,
        SDCHN2 = 0x0D,
        TGCTL = 0x30,
		TGTON = 0x31,
		TGTOFF = 0x32,

		SSRC1 = 0x62,	//SPhoneAcousticInput
		SSRC2 = 0x63,	//SPFlineInput

		SSDX1 = 0x64,
		SSDX2 = 0x65,
		SSDX3 = 0x66,
		SSDX4 = 0x67,
		SSDR1 = 0x68,
		SSDR2 = 0x69,
		SSDR3 = 0x6a,
		SSDR4 = 0x6b,
		SSCAS1 = 0x6c,
		SSCAS2 = 0x6d,
		SSCAS3 = 0x6e,
		SSCLS1 = 0x6f,
		SSCLS2 = 0x70,
		SSCLS3 = 0x71,
		SAGX1 = 0x74,
		SAGX2 = 0x75,
		SAGX3 = 0x76,
		SAGX4 = 0x77,
		SAGR1 = 0x79,
		SAGR2 = 0x7a,
		SAGR3 = 0x7b,
		SAGR4 = 0x7c,

		SCSD1 = 0x90,
		SCSD2 = 0x91,
		SCSD3 = 0x92,
		SCSD4 = 0x93,

		SCLPT = 0x94,
		SCCR = 0x95,
		SCCRN = 0x96,
		SCCRS = 0x97,
		SCCRL = 0x98,
		SCDT = 0x99,
		SCDTN = 0x9a,
		SCDTS = 0x9b,
		SCDTL = 0x9c,

		SCATTN = 0x9d,
		SCATTS = 0x9e,
		SCATTL = 0x9f,
		SCAECL = 0xa0,

		SCSTGP = 0xa1,
		SCSTATT = 0xa2,
		SCSTNL = 0xa3,
		SCSTS = 0xa4,
		SCSTTIM = 0xa5,
		SCSTIS = 0xa6,
		SCSTDS = 0xa7,

		SCLSPN = 0xa8,
		SCLSPS = 0xa9,
		SCLSPL = 0xaa,
		SCCN1 = 0xab,
		SCCN2 = 0xac,
		SCCN3 = 0xad,
    };
} //namespace Ds2155

    
#endif
