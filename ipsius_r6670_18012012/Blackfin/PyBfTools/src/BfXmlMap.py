
import io
import xml.parsers.expat

# -----------------------------------------------------------------

def _LoadFile(fileName : str, maxLines : int) -> [str]:
    counter = 0    
    data = []    
    with io.open(fileName) as f:
        for s in f: 
            data.append(s)
            counter += 1
            if maxLines > 0 and counter >= maxLines: break
                
    return data

# -----------------------------------------------------------------

def Foreach(iter, fn, *arg):
    for i in iter: fn(i, *arg)
    
def FindFirst(iter, fn, *arg):
    for i in iter: 
        if fn(i, *arg): return i
    return None    
    
# -----------------------------------------------------------------

"""

INPUT_SECTION {} -> fix (size etc.)

INPUT_FILE -> chars (name), end

// add items 
SYMBOL {}, DEMANGLED_NAME (chars), end, end

end (INPUT_SECTION)

Store:

Module
    Section
        Symbol

"""

# -----------------------------------------------------------------

class Symbol:
    
    def __init__(self, symbol : dict):
        self.m_name = symbol['name']
        self.m_size = int(symbol['size'], 16)
        
    @property
    def Name(self): return self.m_name
    
    @property
    def Size(self): return self.m_size
    
    def __str__(self):
        return self.Name + ' ' + str(self.Size)  
        
# -----------------------------------------------------------------

class ModuleSection:
    
    def __init__(self, name):
        self.m_name = name
        self.m_size = 0   # cached  
        self.m_sizeChanged = False 
        self.m_symbols = [] # [Symbol]
    
    def Update(self, section : dict):
        # self.m_size += int(section['size'], 16)
        pass
                
    def AddSymbol(self, symbol : dict):
        size = int(symbol['size'], 16)
        if size == 0: return  # don't add empty, no dublicates check
        
        self.m_sizeChanged = True
        self.m_symbols.append( Symbol(symbol) )
        
    @property 
    def Symbols(self):
        return self.m_symbols
        
    def UpdateByOther(self, other : 'ModuleSection'):
        # copy symbols
        self.m_sizeChanged = True 
        for i in other.Symbols: self.m_symbols.append(i)
        
    @property 
    def Name(self): return self.m_name
    
    @property
    def Size(self):
        
        # update cached m_size
        if self.m_sizeChanged:
            sum = [0]
            def f(item, sum): sum[0] += item.Size
            Foreach(self.m_symbols, f, sum)
            
            self.m_size = sum[0]
            self.m_sizeChanged = False 
        
        return self.m_size
    
    def __str__(self):
        return str("{0} size {1} symbols {2}").format( self.m_name, self.Size, len(self.m_symbols) )
    
    def FullInfo(self) -> str:
        s = self.m_name + '\n'
        self.m_symbols = sorted(self.m_symbols, key = lambda x: x.Size)
        res = ['    ' + i.__str__() for i in self.m_symbols]
        return s + str('\n').join(res)

# -----------------------------------------------------------------
    
class ModuleSectionList:
    
    def __init__(self):
        self.m_sections = []  # [ModuleSection]
            
    def Add(self, section : ModuleSection):
        
        s = FindFirst(self.m_sections, lambda x: x.Name == section.Name)
        
        if s is None: 
            self.m_sections.append(section)
        else:
            s.UpdateByOther(section)
            
    def __str__(self):        
        self.m_sections = sorted(self.m_sections, key = lambda x: x.Size)        
        res = [i.__str__() for i in self.m_sections]                
        return str('\n').join(res)
    
    def __iter__(self):
        for i in self.m_sections: yield i
        
# -----------------------------------------------------------------

class Module:
    
    def __init__(self, moduleName):
        self.m_name = moduleName
        self.m_sections = {} # { 'section name' : ModuleSection }
        self.m_currSection = None # ModuleSection 
        self.m_size = -1
        
    def SetSection(self, section : dict):
        name = section['name']
        
        if not name in self.m_sections:
            self.m_sections[name] = ModuleSection(name) 
        
        self.m_currSection = self.m_sections[name]
        self.m_currSection.Update(section)
        
    def AddSymbol(self, symbol : dict):
        self.m_currSection.AddSymbol(symbol)
        
    @property
    def Name(self): return self.m_name
        
    @property
    def Sections(self) -> ModuleSectionList:
        res = ModuleSectionList()
        for i in self.m_sections.values(): res.Add(i)
        return res
        
    @property
    def Size(self):
        
        if self.m_size < 0:
            sum = [0]
            def f(x, sum): sum[0] += x.Size
            Foreach(self.m_sections.values(), f, sum)
            self.m_size = sum[0]
            
        return self.m_size
                            
    def __str__(self):
        
        sections = []
        
        sect_list = sorted(self.m_sections.values(), key = lambda x: x.Size, reverse = True)
        
        for val in sect_list:
            sections.append( val.__str__() )

        return str("{0} {1} [{2}]").format( self.m_name, self.Size, str("; ").join(sections) )
    
    def FullInfo(self) -> str:
        s = 'Module ' + self.m_name + ':\n'
        sect = [i.FullInfo() for i in self.m_sections.values()]
        return s + str('\n').join(sect)
            
# -----------------------------------------------------------------

class ModuleList:

    def __init__(self, items : [Module]):
        self.m_items = sorted(items, key = lambda x: x.Size)
        
    def __str__(self):
        res = []  
        for v in self.m_items:
            res.append( v.__str__() )

        return str('\n').join(res)
            
    @property
    def Sections(self) -> ModuleSectionList:
        res = ModuleSectionList()
        
        for i in self.m_items:
            for s in i.Sections:
                res.Add(s)
        
        return res
    
    def Module(self, moduleName) -> Module:
        return FindFirst(self.m_items, lambda x: x.Name == moduleName) 
                
# -----------------------------------------------------------------

""" Parse xml file -> ModuleList """
class XmlMap:
    
                  
    CPrintMode = False
    
    CSection            = 'INPUT_SECTION'
    CModule             = 'INPUT_FILE'
    CSymbol             = 'SYMBOL'
    CDemangledName      = 'DEMANGLED_NAME'
    CEndOfFile          = 'LDF_SYMBOLS'
    
    def __init__(self, fileName : str, maxLines : int):

        # fields
        self.m_items = {} # {'Name' : Module }
        
        self.m_currChars = ''
        self.m_eof = False
        
        self.m_currSection = None  # section dict
        self.m_currModule = None # Module
        self.m_currSymbol = None # symbol dict
        
        # parser
        p = xml.parsers.expat.ParserCreate()        
        p.StartElementHandler   = self.__start_element
        p.EndElementHandler     = self.__end_element
        p.CharacterDataHandler  = self.__char_data        
        self.m_parser = p

        # parse
        self.m_lines = _LoadFile(fileName, maxLines)
        
        try:
            p.Parse(str('').join(self.m_lines))
        except xml.parsers.expat.ExpatError as e:
            raise
        
    @property 
    def List(self) -> ModuleList:
        return ModuleList(self.m_items.values())
                
    def __start_element(self, name, attrs):
        if XmlMap.CPrintMode: print('Start element:', name, attrs)
        
        if self.m_eof: return
        
        if name == XmlMap.CEndOfFile:
            self.m_eof = True
        
        if name == XmlMap.CSection:
            self.m_currSection = attrs
            
        if name == XmlMap.CModule:
            self.m_currChars = ''
        
        if name == XmlMap.CSymbol:
            assert self.m_currSymbol is None
            self.m_currSymbol = attrs            
        
        if name == XmlMap.CDemangledName:
            self.m_currChars = ''
            
                                            
    def __end_element(self, name):
        if XmlMap.CPrintMode: print('End element:', name)

        if self.m_eof: return

        if name == XmlMap.CSection:
            self.m_currModule = None
            
        if name == XmlMap.CModule:
            self.__SetModule(self.m_currChars, self.m_currSection)
            self.m_currChars = ''
            self.m_currSection = None
        
        if name == XmlMap.CSymbol:
            if (self.m_currSymbol is None):
                print('Non closed symbol at', self.m_parser.CurrentLineNumber)
                return
            
            if (self.m_currModule is None):
                s = 'Symbol {0} outside module at {1} line'
                print( s.format(self.m_currSymbol['name'], self.m_parser.CurrentLineNumber) )
                assert False
                return
                                
            self.m_currModule.AddSymbol(self.m_currSymbol)
            self.m_currSymbol = None
            self.m_currChars = ''
        
        if name == XmlMap.CDemangledName:
            name = self.m_currChars.strip()
            # replace name
            if len(name) > 0: self.m_currSymbol['name'] = name
            self.m_currChars = ''
                                
        pass
            
    def __char_data(self, data):
        if XmlMap.CPrintMode: print('Character data:', repr(data))
        
        if self.m_eof: return
                
        self.m_currChars += data
                
        pass
    
    def __SetModule(self, moduleName : str, section : dict):
        
        pos = moduleName.find('[')
        if pos >= 0: moduleName = moduleName[:pos]
        
        # module not exists in m_items
        if not moduleName in self.m_items:
            self.m_items[moduleName] = Module(moduleName)
                        
        self.m_currModule = self.m_items[moduleName]
        self.m_currModule.SetSection(section)
        

