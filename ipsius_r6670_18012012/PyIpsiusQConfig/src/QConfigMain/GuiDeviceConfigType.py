'''
Created on May 19, 2010

@author: root
'''

from PyQt4 import QtCore, QtGui
from inspect import ismethod, isfunction
from CDUtilsPack.MetaUtils import Property

from QConfigMain.CoreFieldValidator import (Validator, ValidatorHost,
                                     ValidatorName, ValidatorInt, ValidatorBool,
    ValidatorPort)
from QConfigMain.GuiDeviceConfigField import GuiDeviceConfigField
from DRIDomain.TelnetClientDri import DriError
from QConfigMain.GuiFieldWidgetFactory import GuiFieldWidgetFactory
from QConfigMain.GuiDeviceWidget import GuiDeviceWidget
from DRIDomain.TelnetClientDriAsync import CommandDriHandler

# ------------------------------------------------------

class GuiDeviceMonitorItem:
    """
    Using to execute DRI command and show it's result in widget.
    """
    
    def __init__(self, resultWidgetName : str, driCommand : str):
        """
        DRI command 'driCommand' that will be executed on device monitoring 
        request. Results will displayed in widget 'resultWidgetName'.
        """    
        self.resultWidgetName = resultWidgetName
        self.driCommand = driCommand
        self.execFn = None
    
    
    ResultWidgetName = Property('resultWidgetName')
    DriCommand       = Property('driCommand')
    
    
    def Setup(self, parentWidget : GuiDeviceWidget, meta : GuiFieldWidgetFactory, 
              execDriCmdFn):
        """
        'parentWidget' - parent widget of widget with name 'self.result*',
        'execDriCmdFn' - fn(cmd : str, callback) -> None
        """
        wDict = parentWidget.Ui.__dict__
        if not self.resultWidgetName in wDict.keys():
            raise AssertionError("Unknown widget name: '{}'".\
                                 format(self.resultWidgetName))
                
        def Exec(onCompleteNotifyFn):
            widget = wDict[self.resultWidgetName]
            type = widget.metaObject().className()
            
            def Set(val):
                wrapper = meta.Create(widget, val, None)  
                wrapper.Set(val)
            
            def Callback(res : CommandDriHandler):
                #if (res.IsAborted): return
                #if not res.ResultsOK: raise DriError(res.LastResult, 
                #                                     self.driCommand)
                 
                # DriError is handled by execDriCmdFn
                assert res.ResultsOK
                               
                def ParseRes():
                    parsed = []
                    for r in res.Results:
                        oneRes = r.Text
                        if oneRes.endswith('#'): oneRes = oneRes[:-1]
                        parsed.append(oneRes)
                    return "\n".join(parsed)    
                    
                Set(ParseRes())    
                onCompleteNotifyFn()    
                
            execDriCmdFn(self.driCommand, Callback)
        
        self.execFn = Exec
        
        
    def Execute(self, onCompleteNotifyFn):
        """Run command and call onCompleteNotifyFn(), after command 
        result was processed."""
        assert self.execFn
        assert onCompleteNotifyFn
        self.execFn(onCompleteNotifyFn)

# ------------------------------------------------------

class GuiDeviceConfigTypeItem:
    def __init__(self, name : str, widgetName : str, default):
        """
        'Name'            - field name, used also as define in script,
        'Default'         - default value of type: str, int or bool, or 
                            callable object,
        'Validator'       - class that used to check property value,
        'WidgetName'      - assigned widget name,
        'UseInRunScript'  - property will be used in run script, 
        'UseInStopScript' - property will be used in stop script,
        'RuntimeCommand'  - called when field modified while device is
                            running, if None widget will be disabled 
                            during that time,
        'NeedSaveToFile'  - indicate that we need to save this field 
                            value to config file,
        'ToolTip'         - widget's tooltip,
        'Enum'            - range of widget values; can be used with QCheckBox, 
                            QRadioGroup with QRadioButtons and QComboBox,
        'Help'            - field description.
        'ToolTip' and 'Help' will be translated using Qt when widget will be 
        connected. 
        """
        self.Name = name
        self.WidgetName = widgetName
        self.Default = default
        self.Validator = Validator()
        self.NeedSaveToFile = True
        self.UseInRunScript = True
        self.UseInStopScript = False
        self.RuntimeCommand = ''
        self.ToolTip = ''
        self.Enum = None # [str]
        self.Help = ''
    
# ------------------------------------------------------

class GuiDeviceConfigType:
    """
    Device config type template. Use ConfigMaker and MonitorMaker to add 
    custom config fields and monitor functions.
    """
    
    def __init__(self):
        self.configFields = [] # [GuiDeviceConfigTypeItem]
        self.monitorList = [] # [GuiDeviceMonitorItem] 
        self.version = 1
        self.devTypeID = None
        self.devTypeName = None        
    
    
    def ConfigFields(self, parentWidget : GuiDeviceWidget, 
                     meta : GuiFieldWidgetFactory, execDriCmdFn) -> [GuiDeviceConfigField]:
        res = []
        for field in self.configFields:
            default = field.Default
            if ismethod(default) or isfunction(default): default = default()
            custom = GuiDeviceConfigField(\
                    field.Name, field.WidgetName, default, 
                    field.Validator, field.UseInRunScript, field.UseInStopScript, 
                    field.NeedSaveToFile, field.RuntimeCommand, 
                    field.ToolTip, field.Enum, field.Help)
            custom.CustomSetup(parentWidget, meta, execDriCmdFn)
            res.append(custom)
        
        return res
    
    
    def MonitorList(self, parentWidget : GuiDeviceWidget, 
                    meta : GuiFieldWidgetFactory, execDriCmdFn) -> [GuiDeviceMonitorItem]:
        res = []
        for i in self.monitorList:
            copy = GuiDeviceMonitorItem(i.ResultWidgetName, i.DriCommand)
            copy.Setup(parentWidget, meta, execDriCmdFn)
            res.append(copy)
        
        return res        
    
    
    DeviceTypeID = Property('devTypeID', True)
    DeviceTypeName = Property('devTypeName', True)
    Version = Property('version', True)
            
    
    def AddField(self, field : GuiDeviceConfigField):
        """Add config field."""
        def CheckNameUnique():
            for existed in self.configFields:
                assert existed.Name != field.Name
        
        CheckNameUnique()        
        self.configFields.append(field)        
    
    
    def AddMonitor(self, monitorItem : GuiDeviceMonitorItem):
        """Add monitor item."""
        self.monitorList.append(monitorItem)

# ------------------------------------------------------

class ConfigMaker:
    """Using to add config fields to custom device type."""
    
    def __init__(self, type : GuiDeviceConfigType, boardNameFn, hwNumberFn):
        """
        'type'         - base type class, using to register custom config fields,
        'boardNameFn'  - fn() -> str,
        'hwNumberFn'   - fn() -> int. 
        """
        assert ismethod(boardNameFn) or isfunction(boardNameFn)
        self.type = type
        self.boardNameFn = boardNameFn
        self.hwNumberFn = hwNumberFn
        self.closed = False
            
            
    def __del__(self):
        assert self.closed and "Call Close() to complete config."
    
    
    @property
    def BoardName(self):
        return self.boardNameFn
    
    
    @property
    def HwNumber(self):
        return self.hwNumberFn
    
    
    vn = ValidatorName
    vi = ValidatorInt
    vh = ValidatorHost
    vp = ValidatorPort
    vb = ValidatorBool
    
    
    Field = GuiDeviceConfigTypeItem
    
    
    def Close(self, version : int, typeID : int, typeName : str):
        """
        Call it to complete config.
        'version' - current config version, should be >= 0,
        'typeID'  - device type ID (HwID returned by HwFunder),
        'typeName' - device type name.
        """
        assert version >= 0
        self.type.Version = version
        self.type.DeviceTypeID = typeID
        self.type.DeviceTypeName = typeName
        self.closed = True
    
    
    def Add(self, field : GuiDeviceConfigTypeItem):
        """Add field to config."""
        self.type.AddField(field)
        
# ------------------------------------------------------
    
class MonitorMaker:
    """Using to register monitor functions for custom device type."""
    def __init__(self, type : GuiDeviceConfigType):
        """
        'type'  - base type class, using to register custom monitor functions.
        """
        self.type = type
            
    
    def Add(self, resultWidgetName : str, driCommand : str):
        """
        Add DRI command 'driCommand' that will be executed on device 
        monitoring request. Results will displayed in widget 'resultWidgetName'.
        """    
        item = GuiDeviceMonitorItem(resultWidgetName, driCommand)
        self.type.AddMonitor(item)
 
# ------------------------------------------------------
# Tests
# ------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams
from PackUtils.CorePathes import TempFiles
from QConfigMain.GuiDeviceWidget import GuiDeviceWidget


@UtTest
def GuiDeviceConfigTypeTest():
    ExecDriCmdFn = lambda cmd, callback: None
        
    def GuiDeviceMonitorItemTest(parent : GuiDeviceWidget, 
                                 meta : GuiFieldWidgetFactory):
        mi = GuiDeviceMonitorItem('host', 'CS_Print')
        assert mi.ResultWidgetName == 'host'
        assert mi.DriCommand == 'CS_Print'
        
        OnCompleteNotifyFn = lambda: None
        mi.Setup(parent, meta, ExecDriCmdFn)
        mi.Execute(OnCompleteNotifyFn)
    
    def GuiDeviceConfigTypeTest(parent : GuiDeviceWidget, 
                                meta : GuiFieldWidgetFactory):
        t = GuiDeviceConfigType()
        
        t.DeviceTypeID = 1
        t.DeviceTypeName = 'Type'
        t.Version = 1
        
        t.AddField(GuiDeviceConfigTypeItem('DHost', 'host', ''))
        t.AddMonitor(GuiDeviceMonitorItem('ip', 'CS_Set'))
        
        fList = t.ConfigFields(parent, meta, ExecDriCmdFn)
        assert len(fList) == 1
        assert fList[0].Name == 'DHost'
        
        mList = t.MonitorList(parent, meta, ExecDriCmdFn)
        assert len(mList) == 1
        assert mList[0].DriCommand == 'CS_Set'
        
    def ConfigMakerTest(parent : GuiDeviceWidget, 
                        meta : GuiFieldWidgetFactory):
        t = GuiDeviceConfigType()
        BoardNameFn = lambda: 'name'
        HwNumberFn = lambda: 15
        
        m = ConfigMaker(t, BoardNameFn, HwNumberFn)
        assert m.BoardName() == 'name'
        assert m.HwNumber() == 15
        
        assert m.vn() and m.vi() and m.vh() and m.vb()
        f = m.Field('DHost', 'host', '')
        assert f
        m.Add(f)
        
        m.Close(version = 1, typeID = 2, typeName = 'tName')
        assert t.Version == 1
        assert t.DeviceTypeID == 2
        assert t.DeviceTypeName == 'tName'
        assert len(t.ConfigFields(parent, meta, ExecDriCmdFn)) == 1
    
    def MonitorMakerTest(parent : GuiDeviceWidget, 
                         meta : GuiFieldWidgetFactory):
        t = GuiDeviceConfigType()
        m = MonitorMaker(t)
        m.Add('host', 'CS_Set')
        assert len(t.MonitorList(parent, meta, ExecDriCmdFn)) == 1

    def Impl(p : TestParams):
        meta = GuiFieldWidgetFactory()
        module = TempFiles.UiPyModule('Test')
        parent = GuiDeviceWidget(None, module)
        
        GuiDeviceMonitorItemTest(parent, meta)
        GuiDeviceConfigTypeTest(parent, meta)
        ConfigMakerTest(parent, meta)
        MonitorMakerTest(parent, meta)
        
        p.Output("GuiDeviceConfigTypeTest: OK")
        p.Complete()
    
    GTestRunner.RunApp(Impl, traceTest = False)
 
# ------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()