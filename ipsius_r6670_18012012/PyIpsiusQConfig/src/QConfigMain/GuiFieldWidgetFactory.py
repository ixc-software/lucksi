'''
Created on Mar 25, 2010

@author: root
'''


from PyQt4 import QtCore, QtGui
from QConfigMain.CoreFieldConverter import Converter, StrIntConverter
from CDUtilsPack.MetaUtils import Property
from QConfigMain.CoreFieldConverter import Converter
from QConfigMain.CoreFieldValidator import Validator
from PackUtils.CoreBaseException import CoreBaseException
from CDUtilsPack.MiscUtils import AssertForException

# ------------------------------------------------------------

class GuiFieldWidgetFactoryErr(CoreBaseException):
    
    def __init__(self, widget : QtGui.QWidget, text: str):
        err = "{0} ({1}): {2}".format(widget.objectName(), 
                                      widget.metaObject().className(), text)
        Exception.__init__(self, err)
        

# ------------------------------------------------------------

class GuiFieldUnknownMetaErr(GuiFieldWidgetFactoryErr):
    
    def __init__(self, notFoundName : str, widget : QtGui.QWidget, isInner = False):
        inner = "" if not isInner else " inner"
        err = "Unknown {0} value: \"{1}\"".format(inner, notFoundName)
        GuiFieldWidgetFactoryErr.__init__(self, widget, err)

# ------------------------------------------------------------

class GuiFieldEnumLenErr(GuiFieldWidgetFactoryErr):
    
    def __init__(self, widget : QtGui.QWidget, enum : [str], 
                 minLen : int, maxLen : int):
        if maxLen >= 0: assert minLen <= maxLen
        enumStr = str(enum) if enum else ""
        lenStr = ("equal {0}".format(minLen) if (minLen == maxLen) 
                  else "in range[{0}, {1}]".format(minLen, maxLen)) 
        err = "Enum length must be {0}: {1}".format(lenStr, enumStr)
        GuiFieldWidgetFactoryErr.__init__(self, widget, err)

# ------------------------------------------------------------
      
class GuiFieldEnumDublicateErr(GuiFieldWidgetFactoryErr):
    
    def __init__(self, widget : QtGui.QWidget, enum : [str]):
        err = "Enum has duplicate items: {0}".format(enum)
        GuiFieldWidgetFactoryErr.__init__(self, widget, err)

# ------------------------------------------------------------

class GuiFieldEnumUsageErr(GuiFieldWidgetFactoryErr):
    
    def __init__(self, widget, typeWithEnum : bool):
        use = ("have to be described" if typeWithEnum 
               else "represents 'bool' or text type and can't be used")
        err = "Type {0} with enum".format(use)
        GuiFieldWidgetFactoryErr.__init__(self, widget, err)

# ------------------------------------------------------------

class GuiFieldUnknownTypeErr(GuiFieldWidgetFactoryErr):
    
    def __init__(self, widget, valType):
        valType = valType.__name__ if valType != None else None
        err = ("Unsupported combination of widget type"
               " and type '{0}' of value".format(valType))
        GuiFieldWidgetFactoryErr.__init__(self, widget, err)


# ------------------------------------------------------------

class GuiFieldBoolTypeErr(GuiFieldWidgetFactoryErr):
    
    def __init__(self, widget, itemCount : int):
        err = ("Invalid item count for widget that represents "
               "'bool' type (must be 2): {0}".format(itemCount))
        GuiFieldWidgetFactoryErr.__init__(self, widget, err)

# ------------------------------------------------------------

class  GuiFieldWidgetType:
    """Wrapper for custom fidget types."""
    
    def __init__(self, widget : QtGui.QWidget, widgetWrapperType, 
                 converter : Converter, 
                 enum : [] = None):
        """
        'widgetWrapperType' - class with methods Set, Get, OnChanged
        """
        if enum == None:
            self.impl = widgetWrapperType()
        else: 
            self.impl = widgetWrapperType(enum)
        self.converter = converter
        self.setter = lambda val: self.impl.Set(widget, val)
        self.getter = lambda: self.impl.Get(widget)
        self.onChanged = lambda fn: self.impl.OnChanged(widget, fn)
        self.setEnabled = lambda state: widget.setEnabled(state)
        self.setToolTip = lambda val: widget.setToolTip(val)
    
    
    def Set(self, val): self.setter(val)
        
    def Get(self): return self.getter()
    
    def OnChanged(self, fn): self.onChanged(fn)   
    
    def SetEnabled(self, state : bool): self.setEnabled(state)
        
    def SetToolTip(self, val : str): self.setToolTip(val)
        
    @property
    def CustomTypeName(self): return type(self.impl).__name__
    
    ValueConverter = Property("converter")

# ------------------------------------------------------------

class _QRadioGroup:
    
    def __init__(self, radioSet, radioGet, checkCountFn):
        """
        'radioSet'      - fn(obj, val) -> bool, return True if value set,
        'radioGet'      - fn(obj) -> val or None,
        'checkCountFn'  - fn(obj, radioCount : int) -> None.
        """
        self.radioSet = radioSet
        self.radioGet = radioGet
        self.checkCountFn = checkCountFn
    
    
    def __IsRadioBtn(self, obj : QtGui.QWidget) -> bool:
        return obj.metaObject().className() == 'QRadioButton'
    
    
    def __RadioButtonCount(self, widget : QtGui.QRadioButton) -> int:
        res = 0
        for obj in widget.children():
            if self.__IsRadioBtn(obj): res += 1
        return res
    
    
    def __CheckType(self, widget : QtGui.QGroupBox):
        oneChecked = False
        count = 0
        for obj in widget.children():
            if self.__IsRadioBtn(obj):
                count += 1
                if obj.isChecked(): oneChecked = True
        assert oneChecked
        self.checkCountFn(widget, count)
    
    
    def Set(self, widget : QtGui.QGroupBox, val):
        self.__CheckType(widget)
        found = False
        count = 0
        for obj in widget.children():
            if not self.__IsRadioBtn(obj): continue
            wasSet = self.radioSet(obj, val, count)
            count += 1
            if not wasSet: continue
            found = True
            break 
        if not found: 
            raise GuiFieldUnknownMetaErr(val, widget, True)
    
    
    def Get(self, widget : QtGui.QGroupBox): # return val
        self.__CheckType(widget)
        count = 0
        for obj in widget.children():
            if not self.__IsRadioBtn(obj): continue
            val = self.radioGet(obj, count)
            count += 1
            if val != None: return val
    
    
    def OnChanged(self, widget : QtGui.QRadioButton, fn):
        self.__CheckType(widget)
        for obj in widget.children():
            if self.__IsRadioBtn(obj): obj.clicked.connect(fn)    
    
    
# ------------------------------------------------------------

class _QRadioGroupAsEnum(_QRadioGroup):
    """
    Interpret QGroupBox as enum.
    Widget must have at least one QRadioButton, widgets of other types 
    will be ignored. 
    Get() returns value from inner list for checked QRadioButton.
    Set(val) select QRadioButton witch linked to value in the list == val.
    """
    
    def __init__(self, enum : [str]):
        if not enum or len(enum) < 1:
            raise GuiFieldEnumLenErr(enum, 1, -1)
        self.enum = enum
        
        def RadioSet(widget : QtGui.QRadioButton, val : str, id : int) -> bool:
            """If QRadioButton name match 'val' select it and return True."""
            # if widget.accessibleName() != val: return False
            # widget.setChecked(True)
            # return True                
            assert len(self.enum) > id
            if self.enum[id] != val: return False
            widget.setChecked(True)
            return True
        
        def RadioGet(widget : QtGui.QRadioButton, id : int) -> str or None:
            """If QRadioButton name match 'val' return it state."""
            # if not widget.isChecked(): return
            # return widget.accessibleName()      
            if not widget.isChecked(): return
            assert len(self.enum) > id
            return self.enum[id]
        
        def CheckCount(widget : QtGui.QGroupBox, radioButtonsCount : int):
            count = len(self.enum)
            if radioButtonsCount != count:
                raise GuiFieldEnumLenErr(widget, enum, radioButtonsCount, 
                                         radioButtonsCount)
        
        _QRadioGroup.__init__(self, RadioSet, RadioGet, CheckCount)
    
    Type = QtGui.QGroupBox
    ValueTypes = [str, int] 
    
    @staticmethod
    def UsedWithEnum(): return True
    
# ------------------------------------------------------------
        
class _QRadioGroupAsBool(_QRadioGroup):
    """
    Interpret QRagioGroup as boolean variable.
    Widget must have exactly two QRadioButtons, 
    1st added(!) QRadioButton interpret as True if it's checked, 
    2nd - as False.
    """
    
    def __init__(self):
        
        def RadioSet(widget : QtGui.QRadioButton, val : bool, id : int) -> bool:
            if (id == 0 and val) or (id == 1 and not val):
                widget.setChecked(True)
                return True
            return False
                                    
        def RadioGet(widget : QtGui.QRadioButton, id : int) -> bool or None:
            if id == 0:
                return widget.isChecked()
            
        def CheckCount(widget : QtGui.QGroupBox, radioButtonsCount : int):
            if radioButtonsCount != 2:
                raise GuiFieldBoolTypeErr(widget, radioButtonsCount)
            
        _QRadioGroup.__init__(self, RadioSet, RadioGet, CheckCount)

    Type = QtGui.QGroupBox
    ValueTypes = [bool]
    
    @staticmethod
    def UsedWithEnum(): return False
    

# ------------------------------------------------------------

class _QComboBoxAsEnum:
    """
    Store value in the inner list. Link QComboBox and list by indexes.
    """
    
    def __init__(self, enum : [str]):
        if not enum or len(enum) < 1:
            raise GuiFieldEnumLenErr(enum, 1, -1)
        self.enum = enum 
    
    def Set(self, widget : QtGui.QComboBox, val):
        """
        Find index for 'text' within stored records and
        set currentIndex to that index. If 'text' is empty or 'None', 
        currentIndex will be set to -1.
        """
        if len(self.enum) != widget.count():
            raise GuiFieldEnumLenErr(widget, self.enum, 
                                     widget.count(), widget.count())
        
        if not val: 
            widget.setCurrentIndex(-1)
            return
        
        for i, value in enumerate(self.enum):
            if value == val:
                widget.setCurrentIndex(i)
                return
            
        raise GuiFieldUnknownMetaErr(val, widget, True)
    
    
    def Get(self, widget : QtGui.QComboBox): # return val from enum
        if len(self.enum) != widget.count():
            raise GuiFieldEnumLenErr(self.enum, widget.count(), widget.count())
        
        i = widget.currentIndex()
        if i < 0: return ""
        return self.enum[i]
    
    
    def OnChanged(self, widget : QtGui.QComboBox, fn):
        
        def Slot(index : int):
            fn()
            
        widget.currentIndexChanged.connect(Slot)
        
    Type = QtGui.QComboBox
    ValueTypes = [None, str, int] 
    
    @staticmethod
    def UsedWithEnum(): return True
    
# ------------------------------------------------------------

class _QComboBoxAsBool:
    """
    First value (index = 0) interprets as True, second - as False.
    """
    
    def __CheckCount(self, widget : QtGui.QComboBox):
        if widget.count() != 2:
            raise GuiFieldBoolTypeErr(widget, widget.count())
    
    def Set(self, widget : QtGui.QComboBox, val : bool):
        self.__CheckCount(widget)
        index = 0 if val else 1
        widget.setCurrentIndex(index)    
    
    
    def Get(self, widget : QtGui.QComboBox) -> bool:
        self.__CheckCount(widget)
        i = widget.currentIndex()
        if i < 0: widget.setCurrentIndex(0) 
        return True if widget.currentIndex() == 0 else False
    
    
    def OnChanged(self, widget : QtGui.QComboBox, fn):
        
        def Slot(index : int):
            fn()
            
        widget.currentIndexChanged.connect(Slot)
    
    Type = QtGui.QComboBox
    ValueTypes = [bool]
    
    @staticmethod
    def UsedWithEnum(): return False

# ------------------------------------------------------------

class _QCheckBoxAsBool:
    
    def Set(self, widget : QtGui.QCheckBox, val : bool):
        state = QtCore.Qt.Unchecked
        if val: state = QtCore.Qt.Checked
        widget.setChecked(state)  
                         
    
    def Get(self, widget : QtGui.QCheckBox) -> bool:
        if widget.isChecked() == QtCore.Qt.Unchecked: return False
        return True
    
    
    def OnChanged(self, widget : QtGui.QCheckBox, fn):
        
        def Slot(state : bool):
            fn()
        
        widget.stateChanged.connect(Slot)
    
    Type = QtGui.QCheckBox
    ValueTypes = [bool] 
    
    @staticmethod
    def UsedWithEnum(): return False

# ------------------------------------------------------------

class _QCheckBoxAsEnum:
    """
    If QCheckBox is checked, using 1st value from the inner list, else - 2nd. 
    """
    
    def __init__(self, enum : [str]):
        self.enum = enum
    
    def __CheckLen(self, widget : QtGui.QCheckBox):
        if len(self.enum) != 2:
            raise GuiFieldEnumLenErr(widget, self.enum, 2, 2)
    
    def Set(self, widget : QtGui.QCheckBox, val : str):
        self.__CheckLen(widget)
        if not val in self.enum:
            raise GuiFieldUnknownMetaErr(val, widget, True)
        
        state = QtCore.Qt.Checked if (val == self.enum[0]) else QtCore.Qt.Unchecked  
        widget.setChecked(state)
                     
    
    def Get(self, widget : QtGui.QCheckBox): # return val from enum
        self.__CheckLen(widget)
        if widget.isChecked() == QtCore.Qt.Unchecked: return self.enum[1]
        return self.enum[0]
    
    
    def OnChanged(self, widget : QtGui.QCheckBox, fn):
        def Slot(state : bool):
            fn()
        
        widget.stateChanged.connect(Slot)

    Type = QtGui.QCheckBox
    ValueTypes = [str, int] 
    
    @staticmethod
    def UsedWithEnum(): return True

# ------------------------------------------------------------

class _QLineEdit:
            
    def OnChanged(self, widget : QtGui.QLineEdit, fn):
        widget.returnPressed.connect(fn)
    
    def Set(self, widget : QtGui.QLineEdit, val : str):
        widget.setText(val)
        
    def Get(self, widget : QtGui.QLineEdit):
        return widget.text()
    
    Type = QtGui.QLineEdit
    ValueTypes = [None, str, int] 
    
    @staticmethod
    def UsedWithEnum(): return False

# ------------------------------------------------------------

class  _QPushButton:
    
    def OnChanged(self, widget : QtGui.QPushButton, fn):
        widget.clicked.connect(fn)
    
    def Set(self, widget : QtGui.QPushButton, val : str):
        widget.setText(val)
    
    def Get(self, widget : QtGui.QPushButton) -> str:
        return widget.text()
    
    Type = QtGui.QPushButton
    ValueTypes = [None, str, int]
    
    @staticmethod
    def UsedWithEnum(): return False

# ------------------------------------------------------------

class _QLabel:
    
    def OnChanged(self, widget : QtGui.QPushButton, fn):
        pass
    
    def Set(self, widget : QtGui.QLabel, val : str):
        widget.setText(val)
    
    def Get(self, widget : QtGui.QLabel) -> str:
        return widget.text()
    
    Type = QtGui.QLabel
    ValueTypes = [None, str, int]
    
    @staticmethod
    def UsedWithEnum(): return False
    
# ------------------------------------------------------------
            
class _QTextBrowser:
    
    def OnChanged(self, widget : QtGui.QPushButton, fn):
        pass

    def Set(self, widget : QtGui.QTextBrowser, val : str):
        widget.setText(val)
        
    def Get(self, widget : QtGui.QTextBrowser) -> str:
        return widget.toPlainText()
    
    Type = QtGui.QTextBrowser
    ValueTypes = [None, str, int]
    
    @staticmethod
    def UsedWithEnum(): return False
    
# ------------------------------------------------------------

class _QSpinBox:
    
    def OnChanged(self, widget : QtGui.QPushButton, fn):
        pass

    def Set(self, widget : QtGui.QSpinBox, val : int):
        widget.setValue(val)
        
    def Get(self, widget : QtGui.QSpinBox) -> int:
        return widget.value()
    
    Type = QtGui.QSpinBox
    ValueTypes = [int, ]   
    
    @staticmethod
    def UsedWithEnum(): return False 
    

# ------------------------------------------------------------

class GuiFieldWidgetFactory:
    """List of widget types used to display device configuration."""
    
    def __init__(self):
        self.types = [] # [type]
        
        Add = lambda customType: self.types.append(customType)
            
        Add(_QLabel)
        Add(_QLineEdit)
        Add(_QPushButton)
        Add(_QTextBrowser)
        Add(_QCheckBoxAsBool)
        Add(_QCheckBoxAsEnum)
        Add(_QComboBoxAsEnum)
        Add(_QComboBoxAsBool)
        Add(_QRadioGroupAsBool)
        Add(_QRadioGroupAsEnum)      
        Add(_QSpinBox)
    
    
    def Create(self, widget : QtGui.QWidget, widgetDefaultValue, 
               enum : [str] = None) -> GuiFieldWidgetType:
        """
        Use StrIntConverter if 'widgetDefaultValue' is int or None.
        Otherwise return Converter.
        """
        widgetType = type(widget)
        
        def CheckEnumUnique():
            if not enum: return
            unique = []
            for e in enum:
                if e not in unique:
                    unique.append(e)
            if not len(unique) == len(enum):
                raise GuiFieldEnumDublicateErr(widget, enum)
        
        
        def Find(): # return (customType, converter)
            valType = (type(widgetDefaultValue) if widgetDefaultValue != None 
                       else None)
            
            converter = Converter()
            withStrIntConverter = [QtGui.QLineEdit, QtGui.QLabel, 
                                   QtGui.QTextBrowser, QtGui.QPushButton]
            if (valType in [int, None]) and (widgetType in withStrIntConverter):
                converter = StrIntConverter()                
            
            for t in self.types:
                if (t.Type == widgetType and 
                    (valType == None or valType in t.ValueTypes)):
                    return (t, converter)
                
            raise GuiFieldUnknownTypeErr(widget, valType)
               
                
        CheckEnumUnique()
        widgetType, converter = Find()
        
        if not enum and widgetType.UsedWithEnum():
            raise GuiFieldEnumUsageErr(widget, True)
        if enum != None and not widgetType.UsedWithEnum():
            raise GuiFieldEnumUsageErr(widget, False)
        
        return GuiFieldWidgetType(widget, widgetType, converter, enum)


# -------------------------------------------------------------
# Test
# -------------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams

@UtTest
def GuiFieldWidgetFactoryTest():
    
    class CheckClass:
        
        def __init__(self, outputFn):
            self.f = GuiFieldWidgetFactory()
            self.outputFn = outputFn
    
        def Check(self, widget, defaultVal, val, implTypeName : str, 
                  converterType = Converter, enum = None):
            
            def CheckPrint(typeName : str, widgetVal):
                val = " with {0}".format(type(widgetVal).__name__)
                self.outputFn("Checking: {0}{1}".format(typeName, val))
            
            def CheckPrintInv(customType, e : Exception):
                self.outputFn("Checking invalid: {0}".format(str(e))) 
                
            widget.setEnabled(True)
            name = widget.objectName()
            t = self.f.Create(widget, defaultVal, enum)
            assert t.CustomTypeName == implTypeName
            CheckPrint(t.CustomTypeName, defaultVal)
            
            assert widget.isEnabled()
            t.SetEnabled(False)
            assert not widget.isEnabled()
                
            OnChange = lambda: None    
            t.OnChanged(OnChange)
            
            c = t.ValueConverter
            assert isinstance(c, converterType)
            t.Set(c.ToWidgetType(name, val))
            assert c.FromWidgetType(name, t.Get()) == val
            
            t.SetToolTip("custom tooltip")
            assert widget.toolTip() == "custom tooltip"
            
            
        def CheckInvalid(self, widget, defaultVal, enum, errType):
            
            def fn():
                t = self.f.Create(widget, defaultVal, enum)
                t.Set(defaultVal)
            
            AssertForException(fn, GuiFieldWidgetFactoryErr)

    
    def CheckTextType(widgetType, implTypeName : str, c : CheckClass): # QLineEdit, QLabel, etc    
        w = widgetType()
        w.setObjectName("textWidget")
        c.Check(w, "default", "val", implTypeName, Converter, None)
        c.Check(w, 1, 2, implTypeName, StrIntConverter, None)
        
        c.CheckInvalid(w, True, None, GuiFieldUnknownTypeErr)
        c.CheckInvalid(w, "val", ["val", "val2"], GuiFieldEnumUsageErr)
        w = None
    
    def CheckCheckBox(c : CheckClass):
        # QCheckBox as bool and as enum
        checkBox = QtGui.QCheckBox()
        checkBox.setObjectName("checkBox")
        c.Check(checkBox, "Active", "Inactive", "_QCheckBoxAsEnum", Converter, 
              ["Active", "Inactive"])
        c.Check(checkBox, False, True, "_QCheckBoxAsBool", Converter)
        
        c.CheckInvalid(checkBox, "Active", None, GuiFieldEnumUsageErr)
        c.CheckInvalid(checkBox, "Active", [], GuiFieldEnumUsageErr)
        c.CheckInvalid(checkBox, "Active", ["Active"], GuiFieldEnumLenErr)
        c.CheckInvalid(checkBox, "Active", 
                     ["Active", "Inactive", "extra"], GuiFieldEnumLenErr)
    
    def CheckComboBox(c : CheckClass):
        # QComboBox as bool and as enum
        comboBox = QtGui.QComboBox()
        comboBox.setObjectName("comboBox")
        comboBox.addItems(["1x", "2x"])
        c.Check(comboBox, "2", "1", "_QComboBoxAsEnum", Converter, ["1", "2"])
        c.Check(comboBox, 1, 2, "_QComboBoxAsEnum", Converter, [1, 2])
        c.Check(comboBox, True, False, "_QComboBoxAsBool", Converter)
        
        c.CheckInvalid(comboBox, 1, [1], GuiFieldEnumLenErr)
        c.CheckInvalid(comboBox, True, [True, False], GuiFieldEnumUsageErr)
        c.CheckInvalid(comboBox, 1, None, GuiFieldEnumUsageErr)
        c.CheckInvalid(comboBox, 1, [], GuiFieldEnumUsageErr)
        c.CheckInvalid(comboBox, 1, [1, 2, 3, 4], GuiFieldEnumLenErr)
        c.CheckInvalid(comboBox, 1, ["1", 2], GuiFieldUnknownMetaErr)
    
    def CheckRadioGroup(c : CheckClass):
        
        def MakeRadioGroup(name1 : str, name2 : str = None, name3 : str = None):
            res = QtGui.QGroupBox()
            res.setObjectName("radioGroup")
            vbox = QtGui.QVBoxLayout()
            rb1 = QtGui.QRadioButton(name1)
            vbox.addWidget(rb1)
            rb1.setChecked(True)
            if name2 != None:
                rb2 = QtGui.QRadioButton(name2)
                vbox.addWidget(rb2)
            if name3 != None:
                rb3 = QtGui.QRadioButton(name3)
                vbox.addWidget(rb3)
            
            res.setLayout(vbox)
            return res
        
        radioGroup = MakeRadioGroup("1st", "2nd", "3rd")
        c.Check(radioGroup, "x", "y", "_QRadioGroupAsEnum", Converter, 
              ["x", "y", "z"])
        c.Check(radioGroup, 1, 2, "_QRadioGroupAsEnum", Converter, [1, 2, 3])
        
        c.CheckInvalid(radioGroup, True, None, GuiFieldBoolTypeErr)
        c.CheckInvalid(radioGroup, "x", ["x", "y"], GuiFieldEnumLenErr)
        c.CheckInvalid(radioGroup, "x", ["x", "y", "x"], GuiFieldEnumDublicateErr)
        c.CheckInvalid(radioGroup, "x", 
                     ["x", "y", "z", "a"], GuiFieldEnumLenErr)
        c.CheckInvalid(radioGroup, "x", [1, 2, 3], GuiFieldUnknownMetaErr)
        
        radioGroup = MakeRadioGroup("1st", "2nd")
        c.Check(radioGroup, True, False, "_QRadioGroupAsBool", Converter)
        
        c.CheckInvalid(radioGroup, True, [True, False], GuiFieldEnumUsageErr)
    
    
    def Impl(p : TestParams):
        
        outputFn = lambda text : p.Output(p)
        
        outputFn("GuiFieldWidgetFactoryTest: started ...")
        
        c = CheckClass(outputFn)
        CheckTextType(QtGui.QLineEdit, "_QLineEdit", c)
        CheckTextType(QtGui.QLabel, "_QLabel", c)
        #CheckTextType(QtGui.QTextBrowser, "_QTextBrowser", c) 
        CheckTextType(QtGui.QPushButton, "_QPushButton", c)
        CheckCheckBox(c)
        CheckComboBox(c)
        CheckRadioGroup(c)
        
        outputFn("GuiFieldWidgetFactoryTest: OK")
        p.Complete()        
    
    GTestRunner.RunApp(Impl, traceTest = False) 
    
# -------------------------------------------------------------

#@UtTest
#def Test():
#    import sys
#    
#    app = QtGui.QApplication(sys.argv)
#    
#    w = QtGui.QLineEdit()
#    # cause segfault
#    w2 = QtGui.QTextBrowser(w)
#    

# -------------------------------------------------------------

if __name__ == "__main__":

    import unittest
    unittest.main()
    


    
    
