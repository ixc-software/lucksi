import copy

# ------------------------------------------------------

def _FindInStr(s : str, values : [str]) -> [int]:
    res = []
    pos = 0
    
    while pos < len(s):
        
        for v in values:
            if s[pos:pos + len(v)] == v:
                res.append(pos)
                break
        
        pos += 1
        
    return res         

# ------------------------------------------------------
    
def Eval(exp : str, vars : dict) -> float:

    CParenthesisBonus = 10
                                                
    COpsUnary = {
        '-': lambda x: -x
    }
    
    # (priority, lambda); priority must > 0 and < CParenthesisBonus
    COpsBinary = {                            
        '-': (1, lambda x, y: x - y),
        '+': (1, lambda x, y: x + y),
        '*': (2, lambda x, y: x * y),
        '/': (2, lambda x, y: x / y),           
    }     
    
    # one of Number, OpUnary, OpBinary
    class Item: pass
        
    class Number(Item):
        
        def __init__(self, val : str or float):
            self.val = float(val)
            
        def Val(self) -> float:
            return self.val
            
    class OpUnary(Item):
        
        def __init__(self, val : str):
            self.val = val
            self.calcFn = COpsUnary[val]
            
        def Calc(self, num : Number) -> Number:
            return Number( self.calcFn(num.Val()) )
            
    class OpBinary(Item):
        
        def __init__(self, val : str, psisLevel : int):
            self.val = val
            
            desc = COpsBinary[val]            
            self.prio   = desc[0] + psisLevel * CParenthesisBonus
            self.calcFn = desc[1]

        def Calc(self, num0, num1 : Number) -> Number:
            return Number( self.calcFn(num0.Val(), num1.Val()) )
            
        def Priority(self): return self.prio
                                                                                        
    def Parse(exp : str) -> [str]:
        
        def Add(s : str):
            s = s.strip()
            if not s: return
            res.append(s)
        
        CSpliters = ['(', ')', '-', '+', '*', '/']
        
        nodes = _FindInStr(exp, CSpliters)
        assert len(nodes)
        
        res = []
        if nodes[0] > 0: 
            Add( exp[:nodes[0]] )
                
        for indx, n in enumerate(nodes):
            Add( exp[n] )  # add node
            if indx + 1 < len(nodes):
                Add( exp[n + 1 : nodes[indx + 1]] )                
            else:
                Add( exp[n + 1 : ] )                    
            
        return res            
        
    def FirstPass(lx : [str]) -> [Item]:
        
        """ remove parenthesis, setup vars, makes items """
        
        res = []
        psisLevel = 0
        
        for s in lx:
            
            if s == '(': 
                psisLevel += 1
                continue
                
            if s == ')':
                assert psisLevel > 0
                psisLevel -= 1
                continue 
                
            if s[0].isdigit(): 
                res.append( Number(s) )                
                continue
                
            if s[0].isalpha():
                res.append( Number(vars[s]) )
                continue
                
            # s is operator                             
            if len(res) == 0 or not isinstance(res[len(res)-1], Number):
                res.append( OpUnary(s) )
            else:                
                res.append( OpBinary(s, psisLevel) )
            
        assert psisLevel == 0
                
        return res    
                
    def ProcessUnary(lx : [Item]) -> [Item]:

        def FindUnaryNextToNumber() -> int:
            
            for indx, i in enumerate(lx):
                if not isinstance(i, OpUnary): continue
                if isinstance(lx[indx + 1], Number): 
                    return indx
                    
            return -1                    
        
        lx = copy.copy(lx) 

        while True:
            indx = FindUnaryNextToNumber()
            if indx < 0: break
            res = lx[indx].Calc( lx[indx+1] )
            lx = lx[:indx] + [res] + lx[indx + 2:]
                                    
        return lx
        
    def ProcessBinary(lx : [Item]) -> [Item]:
        
        def FindHighestPrioOp() -> int:
            
            maxIndx = -1
            maxPrio = 0
            
            for indx, i in enumerate(lx):
                if isinstance(i, OpBinary) and i.Priority() > maxPrio:
                    maxIndx = indx
                    maxPrio = i.Priority()
                    
            return maxIndx                    
        
        lx = copy.copy(lx) 
        
        while True:
            indx = FindHighestPrioOp()
            if indx < 0: break
            res = lx[indx].Calc( lx[indx-1], lx[indx+1] )
            lx = lx[:indx-1] + [res] + lx[indx + 2:]
                    
        return lx        
                    
    lx = Parse(exp)        
    lx = FirstPass(lx)    
    lx = ProcessUnary(lx)
        
    lx = ProcessBinary(lx)
    assert len(lx) == 1
    
    return lx[0].Val()
    
# ------------------------------------------------------


def PyEval(exp : str, vars : dict) -> float:
    return eval(exp, vars)

# ------------------------------------------------------
    
def Test(exp : str, vars : dict):
    res    = Eval(exp, vars)
    resRef = PyEval(exp, vars)
    
    if res != resRef:
        raise Exception( "{0} != {1}".format(res, resRef) ) 
        
    print("OK, " + str(res))           
        
# ------------------------------------------------------

if __name__ == "__main__":
    
    CExp = '16 - (-a + 10 * b)+7*--c'
    CVars = {'a':1, 'b':2, 'c':-15}
        
    Test(CExp, CVars)
    