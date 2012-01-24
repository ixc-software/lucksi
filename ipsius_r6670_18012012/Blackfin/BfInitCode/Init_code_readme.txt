Init_Code.asm

Please use this file for initialization code.  The file includes 3 sections:
1) A Pre-Init Section
2) A Init Code Section
3) A Post-Init Section

The Pre-Init and Post-Init Sections save and restore the core registers.  
These 2 sections should not be modified.  The Init Code Section
is provided for initialization code.  As an example, a SDRAM initialization
is provided.

This file will be executed on the processor prior to application code booting.  
Hence, any code you would like to run such as initializing SDRAM, etc.. should
be included within this file.
