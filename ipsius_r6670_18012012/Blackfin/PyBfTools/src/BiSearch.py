
from collections import Sequence
from types import FunctionType

# ---------------------------------------------

# binary search without recursion, return index of val inside ordered lx
def BiSearch(lx : Sequence, val) -> int:
    
    def search(lo, hi : int) -> FunctionType or int:
        count = hi - lo
        if count == 0: return -1
            
        avgIndex = lo + count // 2
        assert avgIndex >= lo and avgIndex < hi

        if lx[avgIndex] == val: return avgIndex
            
        if lx[avgIndex] < val:
            return lambda: search(avgIndex + 1, hi)
                            
        return lambda: search(lo, avgIndex)

    if (len(lx) == 0) or (val < lx[0]) or (val > lx[-1:][0]): return -1

    result = lambda: search(0, len(lx))
        
    while True:                
        if isinstance(result, int): return result
        result = result()

# ---------------------------------------------

def BiSearchTest():
    
    def Test(seqSize : int):
        lx = [i for i in range(seqSize)]
        
        for i in range(-1, seqSize + 1):
            result = BiSearch(lx, i)
            if result < 0: 
                assert (i not in lx)
            else:
                assert result == lx.index(i)                
        
    for i in range(10):
        Test(i)
    
# ---------------------------------------------

if __name__ == "__main__":
    
    BiSearchTest()    
    
    print("Done")
    
    