'''
Created on May 19, 2010

@author: root
'''

from PyQt4 import QtCore, QtGui
from CDUtilsPack.MetaUtils import Property

from QConfigMain.GuiFieldWidgetFactory import GuiFieldWidgetFactory
from QConfigMain.CoreFieldValidator import IsValidator, ValidatorName
from DRIDomain.TelnetClientDri import DriError
from QConfigMain.GuiDeviceWidget import GuiDeviceWidget
from DRIDomain.TelnetClientDriAsync import CommandDriHandler
from CDUtilsPack.MiscUtils import AssertForException


class GuiDeviceConfigField:
    """Device config property. Can be mapped to widget, if not - using inner 
    variable to store value."""
    def __init__(self, name : str, widgetName : str, defValue, validator,
                 useInRunScript : bool, useInStopScript : bool, 
                 needSaveToFile : bool, runtimeDriCommand : str, 
                 toolTip : str, enum : [], help : str):
        """
        'name'              - field name, also used as define-name in script,
        'widgetName'        - assigned widget name,
        'defValue'          - default value of type: str, int or bool or 
                              executable object,
        'validator'         - class that used to check field value,
        'useInRunScript'    - use field run script, 
        'useInStopScript'   - use field in stop script,
        'needSaveToFile'    - indicate that we need to save this field 
                              value to config file,
        'runtimeDriCommand' - called when field modified while device is
                              running, if None widget will be disabled 
                              during that time,
        'toolTip'           - widget's toolTip,
        'enum'              - range of widget values; can be used with QCheckBox, 
                              QRadioGroup with QRadioButtons and QComboBox
        'help'              - field description.
        """
        assert isinstance(name, str)
        assert widgetName == None or isinstance(widgetName, str)
        assert isinstance(useInRunScript, bool)
        assert isinstance(useInStopScript, bool)
        assert IsValidator(validator)
        assert isinstance(help, str)
        assert runtimeDriCommand == None or isinstance(runtimeDriCommand, str)
        assert isinstance(toolTip, str) or toolTip == None
        driNameValidator = ValidatorName(False)
        driNameValidator.Check(name, "Script parameter name")
        
        self.driDefineName = name
        self.defValue = defValue
        self.value = defValue
        # self.prevVal = defValue # using to revert widget on DRI error
        self.validator = validator
        self.help = help if help != None else ''
        self.useInRunScript = useInRunScript
        self.useInStopScript = useInStopScript
        self.needSaveToFile = needSaveToFile
        self.deviceIsRunning = False
        # True if CustomSetup() was called
        self.customized = False
        # connected to widget signals like stateChanged(), clicked(), etc;
        # if None, indicates that this is non-runtime field 
        # and it have to be disabled on run 
        self.execOnChangeFn = None
        
        self.widgetName = widgetName
        # if runtimeDriCommand: execute it on this signal call-back,
        #self.widgetChangedSig = None
        self.widgetToolTip = toolTip if toolTip != None else ""
        self.widgetEnum = enum
        self.metaWidget = None # GuiFieldWidgetFactory
        
        self.runtimeDriCommand = runtimeDriCommand
                
        self.validator.Check(self.defValue, self.widgetName)
    
    
    def IsCustomized(self) -> bool:
        return self.customized
    
    
    def __LinkDriExecuter(self, execDriCmdFn):
        
        def Exec():
            if not self.deviceIsRunning: return
            self.Read()
            cmd = "{0} {1}".format(self.runtimeDriCommand, self.Value)
            
            def Callback(res : CommandDriHandler):
                # DriError is handled by execDriCmdFn
                # if we try revert value, we will have one move DRI error
                # if not res.ResultsOK: 
                #    self.SetValue(self.prevVal)
                #    self.Write()
                pass
                        
            execDriCmdFn(cmd, Callback)
        
        assert self.IsRuntime()
        self.execOnChangeFn = Exec
        
        
    def __LinkWidget(self, parentWidget : GuiDeviceWidget, ):
        assert parentWidget
        parentDict = parentWidget.Ui.__dict__
        if not self.widgetName in parentDict.keys():
            return
        
        self.metaWidget.SetToolTip(self.widgetToolTip)
        
        if self.execOnChangeFn:
            self.metaWidget.OnChanged(self.execOnChangeFn)
        self.widgetConnected = True 
        
    
    def CustomSetup(self, parentWidget : GuiDeviceWidget, 
                    meta : GuiFieldWidgetFactory, execDriCmdFn):
        """
        Find widget in 'parentWidget' by name: 'self.widgetName'.
        Initialize widget's Set(), Get(), setEnable() and connect runtime 
        command to changed() slot. 
        'execDriCmdFn' - fn(cmd : str, callback) -> None, check command 
                         retsult AFTER calling 'callback'. It's needed for 
                         runtime changeable field only.                         
        """
        assert not self.IsCustomized()
        self.customized = True
        assert parentWidget
        
        self.help = parentWidget.tr(self.help)
        self.widgetToolTip = parentWidget.tr(self.widgetToolTip)
        
        if not self.widgetName:
            if self.IsRuntime():
                assert 0 and "No widget for runtime field!"
            return
        
        parentDict = parentWidget.Ui.__dict__
        
        if not self.widgetName in parentDict.keys():
            raise AssertionError("Unknown widget name: '{}'".format(self.widgetName))
        
        subWidget = parentDict[self.widgetName]
        self.metaWidget = meta.Create(subWidget, self.Default, self.widgetEnum)
        
        if self.IsRuntime(): self.__LinkDriExecuter(execDriCmdFn)
        self.__LinkWidget(parentWidget)        
    
    
    def __Write(self, val):  # can raise ValidatorErr, ConverterErr
        """Write 'value' to widget."""
        assert self.IsCustomized()
        self.validator.Check(val, self.widgetName)
        if not self.metaWidget: return
        # self.prevVal = self.value
        val = self.metaWidget.ValueConverter.ToWidgetType(self.widgetName, val)
        self.metaWidget.Set(val)
    
    
    def Write(self):  # can raise ValidatorErr, ConverterErr
        """Write 'self.Value' to widget."""
        self.__Write(self.Value)
    
    
    def WriteDefault(self):
        """Write 'self.Default' to widget."""
        self.__Write(self.Default)
        
        
    def Read(self):  # can raise ValidatorErr, ConverterErr
        """Read widget to 'self.Value'."""
        assert self.IsCustomized()
        if not self.metaWidget: return
        val = self.metaWidget.Get()
        val = self.metaWidget.ValueConverter.FromWidgetType(self.widgetName, val)        
        self.validator.Check(val, self.widgetName)
        self.SetValue(val) 
    
    
#    def Enable(self, state: bool):
#        if not self.widgetEnable: return
#        self.widgetEnable(state)
        
        
    def DeviceIsRunning(self, state : bool):
        """
        Notify field about device state. Using to know if we need 
        to execute DRI commands if widget state was changed.
        """
        assert self.IsCustomized()
        if not self.execOnChangeFn:
            if self.metaWidget: self.metaWidget.SetEnabled(not state)
        self.deviceIsRunning = state
    
    
    def IsRuntime(self) -> bool:
        return self.runtimeDriCommand
    
    
    def SetValue(self, val):
        """
        Write 'val' to 'self.Value'. Don't change widget.
        """
        self.validator.Check(val, self.widgetName)
        # self.prevVal = self.value
        self.value = val
    
    
    Name                = Property('driDefineName')
    Default             = Property('defValue')
    Value               = Property('value')
    UseInRunScript      = Property('useInRunScript')
    UseInStopScript     = Property('useInStopScript')
    NeedSaveToFile      = Property('needSaveToFile')
    Help                = Property('help')
    
# -----------------------------------------------
# Tests
# -----------------------------------------------

from QConfigMain.CoreFieldValidator import ValidatorInt, ValidatorErr
from PackUtils.Misc import IntToStr
from QConfigMain.CoreFieldConverter import ConverterErr
from PackUtils.CorePathes import TempFiles
from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams

@UtTest
def GuiDeviceConfigFieldTest():
    name = 'x'
    defVal = 1
    widgetName = 'host'
    useInRunScript = False 
    useInStopScript = True
    runtimeDriCommand = 'CS_Set'
    needSaveToFile = True
    toolTip = "tool tip"
    enum = None
    help = 'help'
    
    def NonGuiFieldTest(parent : GuiDeviceWidget, meta : GuiFieldWidgetFactory):
        f = GuiDeviceConfigField(name, None, defVal, ValidatorInt(), 
                                 useInRunScript, useInStopScript, needSaveToFile, 
                                 None, None, [], help)
        assert f.Name == name
        assert f.Default == defVal
        assert f.Value == defVal
        assert f.UseInRunScript == useInRunScript
        assert f.UseInStopScript == useInStopScript
        assert f.NeedSaveToFile == needSaveToFile
        assert f.Help == help
        
        assert not f.IsCustomized()
        
        # don't link to widget
        f.CustomSetup(parent, meta, None)
        assert f.IsCustomized()
        assert not f.IsRuntime()
        
        f.Write()
        assert f.Value == defVal
        
        f.WriteDefault()
        assert f.Value == defVal
        
        f.Read()
        assert f.Value == defVal
        
        f.DeviceIsRunning(True)
        f.DeviceIsRunning(False)
        
        f.SetValue(8)
        assert f.Value == 8 and f.Default == defVal
        
        fn = lambda: f.SetValue('x')
        AssertForException(fn, ValidatorErr)
    
    def GuiFieldTest(parent : GuiDeviceWidget, meta : GuiFieldWidgetFactory,
                     widget : GuiDeviceWidget):
        f = GuiDeviceConfigField(name, widgetName, defVal, ValidatorInt(), 
                                 useInRunScript, useInStopScript, needSaveToFile, 
                                 None, toolTip, enum, help)
        f.CustomSetup(parent, meta, None)
        assert not f.IsRuntime()
        
        f.DeviceIsRunning(True)
        assert not widget.isEnabled()
        
        f.DeviceIsRunning(False)
        assert widget.isEnabled()
        
        AssertForException(f.Read, ConverterErr)
        
        f.WriteDefault()
        assert widget.text() == IntToStr(f.Default)
        
        f.SetValue(8)
        f.Write()
        assert widget.text() == '8'
        
        f.Read()
        assert f.Value == 8
        assert f.Default == defVal
        
        assert widget.toolTip() == toolTip
        

    def RuntimeFieldTest(parent : GuiDeviceWidget, meta : GuiFieldWidgetFactory,
                         widget : GuiDeviceWidget):
        def ExecDriCmdFn(cmd : str, callback):
            pass
        
        f = GuiDeviceConfigField(name, widgetName, defVal, ValidatorInt(), 
                                 useInRunScript, useInStopScript, needSaveToFile, 
                                 runtimeDriCommand, toolTip, enum, help)
        f.CustomSetup(parent, meta, ExecDriCmdFn)
        assert f.IsRuntime()
        
        f.DeviceIsRunning(True)
        assert widget.isEnabled()
        
        f.DeviceIsRunning(False)
        assert widget.isEnabled()
    
        widget.returnPressed.emit()        

    
    def Impl(p : TestParams):
        p.Output("GuiDeviceConfigFieldTest started ...")
        
        meta = GuiFieldWidgetFactory()
        module = TempFiles.UiPyModule('Test')
        parent = GuiDeviceWidget(None, module)
        widget = parent.Ui.__dict__[widgetName]
        
        NonGuiFieldTest(parent, meta)
        GuiFieldTest(parent, meta, widget)
        RuntimeFieldTest(parent, meta, widget)   
        
        p.Output("GuiDeviceConfigFieldTest: OK")    
        p.Complete()
    
    
    GTestRunner.RunApp(Impl, traceTest = False)

# -----------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    
    
    
    
