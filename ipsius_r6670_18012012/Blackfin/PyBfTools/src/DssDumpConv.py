import CDUtils

# ---------------------------------------------------

if __name__ == "__main__":
    
    CFileName = r"d:\proj\Ipsius\Blackfin\PyBfTools\Dss1MsgDump.txt"
    sl = CDUtils.LoadStringList(CFileName)
    
    res = []
    
    for s in sl:
        s = s.strip()
        if not s: continue
        
        n = s.find('{')
        n2 = s.find('}')
        assert n >= 0 and n2 >= 0
        
        s = s[n+1:n2].strip()
        # print(s)

        valList = s.split(",")
        
        valLen = int(valList[0].strip())
        valList = valList[1:]
        assert valLen == len(valList)
        
        # remove L2
        if valLen <= 4: continue
        valLen -= 4
        valList = valList[4:]
        
        # make line
        valList = ["\\" + s[1:] for s in valList]
        line = 'Add({0}, "{1}");'.format(valLen, "".join(valList) )
        # print(line)
        res.append(line)
        
    CDUtils.SaveStringList(res, "c:/result.txt")
   
    print("Done!")
   