import sys, os
from Slicer import slicer
from Slicer import SlicerWrapper

class ScriptedModuleGUI(object):

    def __init__(self):
        self.vtkScriptedModuleGUI = slicer.vtkScriptedModuleGUI()
    
    def __getattr__(self,name):
        method = getattr(self.vtkScriptedModuleGUI,name,None)
        if not method:
            raise AttributeError, "'%s' object has no attribute '%s'" % (self.__class__.__name__,name)
        return method

class SlicerScriptedModuleImporter(object):

    def __init__(self, moduleNames):
        self.ModuleNames = moduleNames
        self.Modules = {}
    
    def ScanAndInitModules(self):
        self.Modules = {}
        for moduleName in self.ModuleNames:
            try:
                importedModule = __import__(moduleName)
            except ImportError, e:
                print>>sys.stderr, e
                continue
            if importedModule == None:
                continue
            ScriptedModuleGUIClass = None
            for aname in importedModule.__dict__:
                a = importedModule.__dict__[aname]
                if type(a) == type:
                    if issubclass(a,ScriptedModuleGUI) and a != ScriptedModuleGUI:
                        ScriptedModuleGUIClass = a
                        break
            if ScriptedModuleGUIClass == None:
                continue
            try:
                gui = ScriptedModuleGUIClass()
                gui.SlicerWrapper = SlicerWrapper(gui.__dict__['vtkScriptedModuleGUI'])
                gui.OwnWrapper = False
            except Exception, e:
                print>>sys.stderr, "Cannot instantiate ScriptedModuleGUIClass: error ", e
                continue
            displayName = moduleName
            logic = slicer.vtkScriptedModuleLogic()
            logic.SetModuleName(moduleName)
            logic.SetAndObserveMRMLScene(slicer.MRMLScene)
            logic.SetApplicationLogic(slicer.ApplicationLogic)
            gui.SetModuleName(moduleName)
            gui.SetLanguageToPython()
            gui.SetLogic(logic)
#LEAK
            gui.SetApplicationGUI(slicer.ApplicationGUI)
            gui.SetApplication(slicer.Application)
            gui.SetGUIName(displayName)
            uiPanel = gui.GetUIPanel()
            uiPanel.SetName(displayName)
#LEAK
            uiPanel.SetUserInterfaceManager(slicer.ApplicationGUI.GetMainSlicerWindow().GetMainUserInterfaceManager())
            uiPanel.Create()
            slicer.Application.AddModuleGUI(gui)
            logic.Initialize()
            self.Modules[moduleName] = {
                'logic': logic,
                'gui': gui,
                'module': importedModule
                }
            gui.BuildGUI()
            gui.AddGUIObservers()
        
    def TearDownAllGUI(self):
        for moduleName in self.Modules:
            module = self.Modules[moduleName]
            gui = module['gui']
            if gui != None:
#                slicer.Application.RemoveModuleGUI(gui)
                gui.RemoveGUIObservers()
                gui.TearDownGUI()
#        slicer.ApplicationGUI.PopulateModuleChooseList()

    def DeleteAllGUI(self):
        for moduleName in self.Modules:
            module = self.Modules[moduleName]
            gui = module['gui']
            if gui != None:
                gui.RemoveMRMLNodeObservers()
                gui.RemoveLogicObservers()
                gui.SetLogic(None)
                gui.SetScriptedModuleNode(None)
#                gui.SetApplicationGUI(None)
#                gui.GetUIPanel().SetUserInterfaceManager(None)
                module['gui'] = None
        
    def DeleteAllLogic(self):
        for moduleName in self.Modules:
            module = self.Modules[moduleName]
            logic = module['logic']
            if logic != None:
                logic.SetAndObserveMRMLScene(None)
                module['logic'] = None

