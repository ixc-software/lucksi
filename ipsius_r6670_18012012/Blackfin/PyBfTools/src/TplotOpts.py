"""
http://antilamer.livejournal.com/297388.html

Usage: tplot [-o OFILE] [-of {png|pdf|ps|svg|x}] [-or 640x480] -if IFILE [-tf TF] 
             [-k Pat1 Kind1 -k Pat2 Kind2 ...] [-dk KindN]
  -o  OFILE - output file (required if -of is not x)
  -of       - output format (x means draw result in a window, default: extension of -o)
  -or       - output resolution (default 640x480)
  -if IFILE - input file
  -tf TF    - time format: 'num' means that times are integer numbers less than 2^31
              (for instance, line numbers); 'date PATTERN' means that times are dates
              in the format specified by PATTERN - see http://linux.die.net/man/3/strptime,
              for example, [%Y-%m-%d %H:%M:%S] parses dates like [2009-10-20 16:52:43]. 
              Default: 'date %Y-%m-%d %H:%M:%S'
  -k P K    - set diagram kind for tracks matching pattern P to K 
              (-k clauses are matched till first success)
  -dk       - set default diagram kind
  
  
Diagram kinds:
  'event' is for event diagrams: during events are drawn like --[===]--- , pulse events like --|--
  'hist N' is for histograms: a histogram is drawn with granularity of N time units, where
     the bin corresponding to [t..t+N) has value 'what was the maximal number of active events
     in that interval'.
  'freq N [TYPE]' is for event frequency histograms: a histogram of type TYPE (stacked or 
     clustered, default clustered) is drawn for each time bin of size N, about the distribution 
     of various ` events
  'count N [TYPE]' is for event frequency histograms: a histogram of type TYPE (stacked or 
     clustered, default clustered) is drawn for each time bin of size N, about the counts of 
     various ` events
  'quantile N q1,q2,..' (example: quantile 100 0.25,0.5,0.75) - a bar chart of corresponding
     quantiles in time bins of size N
  'binf N v1,v2,..' (example: binf 100 1,2,5,10) - a bar chart of frequency of values falling
     into bins min..v1, v1..v2, .., v2..max in time bins of size N
  'binc N v1,v2,..' (example: binf 100 1,2,5,10) - a bar chart of counts of values falling
     into bins min..v1, v1..v2, .., v2..max in time bins of size N
     
  
"""

from CDUtils import ListClassFields

# -------------------------------------------

class _DiagramKind:
    def __init__(self, s : str):
                
        def addN(s): 
            self.N = int(s)
        
        def addType(s): 
            val = 'clustered'
            if s != "":
                if s != 'stacked' and s != 'clustered': raise Exception('')
                val = s
            self.Type = val
        
        def addFloats(s): 
            self.Bins = [float(v) for v in s.split(',')]
                
        kinds = {}
        kinds['event']     = ()
        kinds['hist']      = (addN,)
        kinds['freq']      = (addN, addType)
        kinds['count']     = (addN, addType)
        kinds['quantile']  = (addN, addFloats)
        kinds['bindf']     = (addN, addFloats)
        kinds['binc']      = (addN, addFloats)
                
        parts = s.split(' ')
        self.Kind = parts[0]
        options = parts[1:]
                
        if self.Kind not in kinds: raise Exception('Unknown diagram kind {0}'.format(self.Kind))
                
        addFunctions = kinds[self.Kind]
        for i, fn in enumerate(addFunctions):
            s = ''
            if i < len(options): s = options[i]
            fn(s)            
                
    def __str__(self):
        return "(" + ", ".join( ListClassFields(self) ) + ")"

# -------------------------------------------

class _Opt:    
    
    def __init__(self, key : str, fn, paramsCount = 1, multiply = False, req = False):
        self.Key = key
        self.Fn = fn
        self.ParamsCount = paramsCount
        self.Multiply = multiply
        self.Req = req
        
class _Resolution:
    
    def __init__(self, s : str):
        pos = s.find("x")
        if pos < 0: raise Exception('Bad resolution!')
        self.Width = int(s[:pos])
        self.Height = int(s[pos + 1:])
        
    def __str__(self):
        return "{0}x{1}".format(self.Width, self.Height) 
        
class TplotOpts:
    
    def __init__(self, args : [str]):
        
        optsList = self.__SplitOptsByKeys(args)
        
        self.OutputFormat = ""
        self.OutputFile = ""
        self.OutputResolution = None
        self.InputFile = ""
        self.TimeFormat = ""
        self.PatternsToKindMap = {}
        self.DefaultKind = None

        # TODO: bind with real application format classes
        def fnOutputFormat(p):
            CFormats = ("png", "pdf", "ps", "svg", "x")  
            if p not in CFormats: raise Exception('Bad format {0}'.format(p)) 
            self.OutputFormat = p

        def fnOutputFile(p): 
            self.OutputFile = p
            if self.OutputFormat == "": 
                dotPos = p.rfind('.')
                if dotPos < 0: return
                fnOutputFormat(p[dotPos + 1:]) 
        
        def fnOutputResolution(p): 
            self.OutputResolution = _Resolution(p)
        
        def fnInputFile(p): 
            self.InputFile = p

        # TODO: parse num/date format - ?
        def fnTimeFormat(p): 
            self.TimeFormat = p
        
        def fnDiagKind(params): 
            p = params[0]
            k = params[1]
            if p in self.PatternsToKindMap: 
                raise Exception('Dublicate P to K map for P {0}'.format(p))
            self.PatternsToKindMap[p] = _DiagramKind(k) 
                    
        def fnDiagDefaultKind(p): 
            self.DefaultKind = _DiagramKind(p)
        
        optsDesc = [_Opt("-of", fnOutputFormat),                    
                    _Opt("-o", fnOutputFile),       # depends from "-of" 
                    _Opt("-or", fnOutputResolution),
                    _Opt("-if", fnInputFile, req = True),
                    _Opt("-tf", fnTimeFormat),
                    _Opt("-k", fnDiagKind, paramsCount = 2, multiply = True),
                    _Opt("-dk", fnDiagDefaultKind)]  # depends from "-k" - ?
        
        self.__Parse(optsList, optsDesc)
        
        # post parse checks
        if self.OutputFormat != "" and self.OutputFormat != "x":
            if self.OutputFile == "": raise Exception("Output format without filename!")

    @staticmethod
    def __SplitOptsByKeys(opts : [str]) -> [[str]]:
        keys = [i for i, o in enumerate(opts) if o.startswith('-')]
        keys.append(len(opts)) # for last key slice
        return [ opts[keys[i]:keys[i+1]] for i in range(len(keys)-1) ]

    @staticmethod    
    def __Parse(optsList : [[str]], optsDesc : [_Opt]):
        
        for desc in optsDesc:
            opts = [x for x in optsList if x[0] == desc.Key]
            if desc.Req and len(opts) == 0: 
                raise Exception('No required key {0}!'.format(desc.Key))
            if len(opts) > 1 and not desc.Multiply: 
                raise Exception('Key {0} allowed only once!'.format(desc.Key))
            
            for opt in opts:
                params = opt[1:]
                if len(params) != desc.ParamsCount: 
                    raise Exception('Bad params count for key {0}'.format(desc.Key))
                if desc.ParamsCount == 1: desc.Fn(params[0])
                else: desc.Fn(params)

            optsList = [x for x in optsList if x[0] != desc.Key] # remove processed
            
        if len(optsList) != 0: raise Exception('Unknown keys {0}'.format(optsList))

    def __str__(self):
        return "\n".join( ListClassFields(self) )        
        
#    def __str__(self):
#        CFields = ("OutputFormat", "OutputFile", "OutputResolution",
#        "InputFile", "TimeFormat", "DefaultKind")
#        
#        fields = [f + ": " + str(getattr(self, f)) for f in CFields]
#        
#        if self.PatternsToKindMap is not None:
#            s = [k + " = " + str(self.PatternsToKindMap[k]) for k in self.PatternsToKindMap]
#            s = ', '.join(s)
#            fields.append("PatternsToKindMap: " + s)
#        
#        return "\n".join(fields) 
        
# -------------------------------------------

def _StringToOptList(s : str) -> [str]:    
    res = []    
    inQuotes = False
    startPos = 0 
    for i, c in enumerate(s):
        if c == "'": inQuotes = not inQuotes
        if (c == " " and not inQuotes) or (i == len(s) - 1):
            res.append(s[startPos:i].strip("'"))
            startPos = i + 1 
    
    return res
    

# -------------------------------------------

if __name__ == '__main__':
    
    opts = "-of x -or 640x480 -if a.txt -k pat0 'event' -k pat1 'hist 100 stacked' -k pat2 'freq 7' -k pat3 'binc 8 1.5,2,5,10' -dk 'event'"

    optList = _StringToOptList(opts)
        
    tplotOpts = TplotOpts(optList)
    print(tplotOpts)
            
