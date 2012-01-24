'''
Created on 08.02.2010

@author: Alex
'''

import CDUtils

from CDUtils import Property

# -------------------------------------------------------------

def SlFind(sl : [str], toFind : str, startIndx = 0, throw = False) -> int:
    
    assert startIndx < len(sl)
    
    for indx in range(startIndx, len(sl)):
        if sl[indx].find(toFind) >= 0: return indx
        
    if throw: raise Exception('Not found ' + toFind)
        
    return -1

# -------------------------------------------------------------

class TBO: pass   # forward

class Division:
    
    def __init__(self, owner : TBO, number : int, total : int, cols : [int]):
        
        self.owner = owner
        self.number = number
        self.total = total
        self.cols = cols
        
        # verify cols
        assert sum(cols) == total
        
    def Calc(self, colsRatio : [int]):
        
        assert len(self.cols) == len(colsRatio) 

        # find diffSum and max info
        diffSum = 0
        
        maxIndx = 0
        maxValue = 0  # absolute diff
                
        for i, colVal in enumerate(self.cols):
            refVal = self.total * colsRatio[i]
            diff = colVal - refVal
            diffSum += abs(diff)
            
            if i == 0 or diff > maxValue:
                maxIndx = i
                maxValue = diff                     
                
        # set error
        self.errorRatio = (diffSum / 2) / self.total
        
        # set max info
        maxRatio = (self.cols[maxIndx] + maxValue) / self.cols[maxIndx]
        self.maxInfo = "{0} {1:.1f}".format(self.owner.ColsNames[maxIndx], maxRatio) 
        
    @property 
    def ErrorRatio(self):
        return self.errorRatio * 100 

    Number      = Property('number')
    Cols        = Property('cols')
    Total       = Property('total')
    MaxInfo     = Property('maxInfo') 

# -------------------------------------------------------------

class TBO:
        
    def __init__(self, data : [str]):
        
        def GetTitle() -> str:
            i = SlFind(data, "<TITLE>", throw = True)
            s = data[i]
            nBeg = s.find('>')
            nEnd = s.find('<', nBeg + 1)
            assert nBeg >= 0 and nEnd >= 0
            s = s[nBeg+1:nEnd].strip()
            return s

        # исключая первую колонку -- "всего"
        def GetColumns() -> [str]:
            
            nonlocal data
                        
            i = SlFind(data, "<td class=td1small>Дільн.", throw = True)
            i2 = SlFind(data, "<tr>", i + 1, throw = True)
            
            res = []
            
            for indx in range(i + 2, i2):
                s = data[indx]
                pos = s.rfind('>')
                assert pos >= 0
                s = s[pos + 1:].strip()
                if s[2] == ' ': s = s[3:]
                res.append(s)
                
            data = data[i2 + 1:]  # cut data
                            
            return res
                        
        self.title = GetTitle()
        self.colsNames = GetColumns()
                        
        self.divisions = self.__MakeDivisions(data)
        
        self.__Calc()
        
    ColsNames = Property('colsNames')   
    Title = Property('title')     
        
    def __Calc(self):
        
        # calc total
        self.total = sum( map(lambda d: d.Total, self.divisions) )
                
        # calc sum per cols
        sumCols = [0 for i in self.colsNames]
        
        for d in self.divisions:
            for i, c in enumerate(d.Cols): sumCols[i] += c
                    
        assert self.total == sum(sumCols) # verify   
        
        # calc ratio
        self.colsRatio = [i/self.total for i in sumCols]
        
        # info
#        for i in range(len(self.colsNames)):
#            print(self.colsNames[i], self.colsRatio[i] * 100)

        # calc divisions
        for d in self.divisions:
            d.Calc( self.colsRatio ) 
        
    def __MakeDivisions(self, data) -> [Division]:
        
        def NumFromLine(s) -> int:
            n = s.rfind('>')
            assert n >= 0
            return int( s[n + 1:] )
        
        res = []
        currDiv = 1
        colsCount = len(self.colsNames)
        
        while True:
            
            # verify number
            assert data[0].endswith("<b>{0}</b>".format(currDiv))
            
            # total
            total = NumFromLine(data[1])
            
            if total:                            
                cols = []
                for i in range(colsCount):
                    s = data[2 + i]
                    cols.append( NumFromLine(s) )
                                    
                res.append( Division(self, currDiv, total, cols) )
            
            # eof detection
            lastLine = 2 + colsCount
            s = data[lastLine]
            
            if s == '<tr>':
                currDiv += 1 
                data = data[lastLine + 1:]
            elif s == '</table>':
                break
            else:
                assert False
                                                                        
        return res
    
    def ListDivisions(self) -> [str]:
        
        res = []
        
        for d in self.divisions:
            res.append( '{0} {1:.2f}%'.format(d.Number, d.ErrorRatio) )
        
        return res
    
    def HistList(self, colsNumber = 10, sep = '\t', divListLimit = 5) -> [str]:
        
        total = self.total
        
        class HistCol:
            
            def __init__(self, vMin, vMax):
                self.vMin = vMin
                self.vMax = vMax
                self.divisions = []
            
            def TryAdd(self, d : Division) -> bool:
                r = d.ErrorRatio
                
                if r >= self.vMin and r <= self.vMax:
                    self.divisions.append(d)
                    return True
                
                return False
            
            def __str__(self):
                
                vSum = sum( map(lambda d: d.Total, self.divisions) )
                ratio = (vSum / total) * 100 
                                
                divList = ""
                if len(self.divisions) <= divListLimit:
                    divList = "; ".join( [str(d.Number) + " (" + d.MaxInfo + ")" for d in self.divisions] )
                if len(self.divisions) == 0: divList = ""
                                    
                def Percent(val) -> str:
                    return format(val, ".1f").replace('.', ',')
                                
                return "[{min}-{max}%] {count} участков {sep} {ratio} {sep} {divList}".format( 
                            min = Percent(self.vMin), 
                            max = Percent(self.vMax), 
                            ratio = Percent(ratio), 
                            count = len(self.divisions),
                            sep = sep,
                            divList = divList )
            
        def MakeHistCols() -> [HistCol]:
            # make cols
            maxValue = max( map(lambda d: d.ErrorRatio, self.divisions) )
            maxValue += 0.1  # small extra range
            colRange = maxValue / colsNumber 
            
            res = []
            curr = 0
            
            for i in range(colsNumber):
                res.append( HistCol(curr, curr + colRange) )
                curr += colRange
                
            return res 
                
        cols = MakeHistCols()
        
        # data to cols
        for d in self.divisions:
            addOk = False
            for c in cols:
                if c.TryAdd(d):
                    addOk = True 
                    break
            assert addOk
        
        # output
        res = []
        
        for c in cols: 
            res.append( str(c) )
                
        return res

# -------------------------------------------------------------

"""

    Аналитика выборов по области 
    http://www.cvk.gov.ua/vp2010/wp336pt001f01=700pt005f01=136.html
    http://www.cvk.gov.ua/vp2010/wp0011.html
    посчитать полное отклонение по участку (в процентах)
    1. посчитать средний процент по области
    2. посчитать разность в людях по каждому кандидату как отклонение от среднего
    3. просуммировать, разделить на два, и взять процент (есть отклонение от среднего)
    Exel -- именованные точки x,y 
    процент на одно оси; процент отклонения/размер участка; гистограммы процента отклонения (по числу людей, по числу участков?)
    гистограмма: 0-1.5%, 7 участков (...), xxx человек 
    
    Полный цикл -- файл + число колонок -> файл результат (печать?)
    Львов, Одесса, Киев
    второй тур 


"""
                    
# -------------------------------------------------------------

if __name__ == '__main__':
    
#    CFile = r"d:\Down\CVK\Результати голосування по дільницях ТВО №136, Одеська область.htm"  # зэки
#    CFile = r"d:\Down\CVK\Результати голосування по дільницях ТВО №138, Одеська область.htm"  # больницы
    CFile = r"d:\Down\CVK\Результати голосування по дільницях ТВО №217, м.Київ.htm"          # все ровно
#    CFile = r"d:\Down\CVK\Результати голосування по дільницях ТВО №120, Львівська область.htm"  # зэки
    
    t = TBO(CDUtils.LoadStringList(CFile, encoding = "windows-1251")) 

#    print(*t.ListDivisions(), sep='\n')

    sl = t.HistList(6)
    
    print(t.Title)
    print( *sl, sep='\n' )
    
#    CDUtils.SaveStringList(sl, "c:\cvk.txt")
    
    pass
    
    