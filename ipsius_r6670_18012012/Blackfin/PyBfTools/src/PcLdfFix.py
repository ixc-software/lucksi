'''
Created on 10.11.2009

@author: Alex
'''

import sys
import os

import CDUtils

# -----------------------------------------------------     

if __name__ == '__main__':
    
    CSectionName = 'sdram_stack_heap'
    CCommand = 'FORCE_CONTIGUITY'
    
    if len(sys.argv) != 2:
        print("Usage: <ldf file name>")
        exit()
    
    fileName = sys.argv[1] # os.getcwd() + sys.argv[1]
    lines = CDUtils.LoadStringList(fileName)
        
    sectionBegin = CDUtils.FindFirst(lines, lambda x: x.strip() == CSectionName, returnItem = False)
    if sectionBegin is None:
        print('Section {0} not found'.format(CSectionName))
        exit()
            
    if lines[sectionBegin+2].strip() == CCommand:    # nothing to do
        exit()
        
    lines.insert(sectionBegin+2, ' ' * 9 + CCommand)
        
    CDUtils.SaveStringList(lines, fileName)
    print('LDF fixed!')
    
    