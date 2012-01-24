import io

# -------------------------------------------------------------

def LoadStringList(fileName : str, encoding = "utf_8", linesLimit = 0, joinWith = '') -> [str] or str:
    
    res = []
    count = 0
    
    with io.open(fileName, mode = "rt", encoding = encoding) as f:
        for line in f:
            line = line.strip('\x0d\x0a')
            res.append(line)
            count += 1
            if (linesLimit > 0 and count >= linesLimit): break
            
    if len(joinWith): 
        res = joinWith.join(res)            
            
    return res 

# -------------------------------------------------------------

class UIK:
    
    def __init__(self, lx : [str]):
        self.names = lx[:6]                                   # 0 .. 5  
        self.votes = [ int(x) for x in lx[6:13] ]             # 6 .. 12 
        self.votesSum = int( lx[13] )
        assert sum(self.votes) == self.votesSum
        self.total    = int( lx[14] )
        
    def PercentFor(self, partyIndex : int) -> float:
        return (self.votes[partyIndex] / self.votesSum) * 100
        
    def Attendance(self):
        return (self.votesSum / self.total) * 100

# -------------------------------------------------------------

def GetUIK(data : [str], selName : str) -> [UIK]:
    
    def FixItems(lx : [str]) -> [str]:
        res = []
        for s in lx:
            s = s.strip()
            if s.startswith('"') and s.endswith('"'):
                s = s[1:-1]
            res.append(s)                    
        return res
            
    res = []
    
    for s in data:
        lx = s.split(",")
        lx = FixItems(lx)        
        if not lx[0]: continue   # skip first
        if not selName or (selName and selName == lx[0]):
            res.append( UIK(lx) )

    return res             

# -------------------------------------------------------------

class Hist:
    
    class Item:
        
        def __init__(self, partyIndex, loPercent, hiPercent, uikCount):
            self.partyIndex  = partyIndex
            self.loPercent   = loPercent
            self.hiPercent   = hiPercent
            self.uikCount    = uikCount 
            self.items       = []   # [UIK]
            
        def Add(self, u : UIK) -> bool:
            p = u.PercentFor(self.partyIndex)
            
            if p >= self.loPercent and p <= self.hiPercent:
                self.items.append(u)
                return True
                
            return False
            
        def __str__(self):
            avgAttend = sum( map(lambda x: x.Attendance(), self.items) ) / len(self.items)
            uikPercent = (len(self.items) / self.uikCount) * 100
            return "Votes: {0:.1f} - {1:.1f}%  UIK(%): {2:<10.1f} Attend(%): {3:.1f}".format(
                self.loPercent, self.hiPercent, uikPercent, avgAttend )
    
    def __init__(self, data : [UIK], partyIndex : int, splitParts : int):
        
        def AllPercent(): 
            return map(lambda x: x.PercentFor(partyIndex), data)
                
        absMin = min( AllPercent() )
        absMax = max( AllPercent() )
        
        # print(absMin, absMax)
        
        step = (absMax - absMin) / splitParts
        
        self.items = []

        # create ranges        
        for i in range(splitParts):
            lo = absMin + step * i
            self.items.append( Hist.Item(partyIndex, lo, lo + step, len(data)) )
            
        # add items to ranges
        for u in data:
            
            added = False
            
            for i in self.items:
                if i.Add(u):
                    added = True
                    break
                    
            assert added

    def __str__(self):
        s = [str(x) for x in self.items]    
        return "\n".join(s)


# -------------------------------------------------------------

if __name__ == "__main__":
    
    data = LoadStringList(r"d:\MyDox\Downloads\cik-Dec_06_2011-12_05am.csv")
    
    sel = "Город Москва"
    sel = "Город Санкт-Петербург"
    data = GetUIK(data, sel)    
    print(len(data))
    
    h = Hist(data, 5, 20)
    print( h )
    