from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer

vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000
vtkKWPushButton_InvokedEvent = 10000

class RegistrationWelcomeGUI(ScriptedModuleGUI):

    def __init__(self):
        ScriptedModuleGUI.__init__(self)
        self.vtkScriptedModuleGUI.SetCategory("Registration")
    
    def Destructor(self):
        pass
    
    def RemoveMRMLNodeObservers(self):
        pass
    
    def RemoveLogicObservers(self):
        pass

    def AddGUIObservers(self):
        pass

    def RemoveGUIObservers(self):
        pass

    def ProcessGUIEvents(self,caller,event):
        pass

    def BuildGUI(self):
        self.GetUIPanel().AddPage("RegistrationWelcome","RegistrationWelcome","")
        pageWidget = self.GetUIPanel().GetPageWidget("RegistrationWelcome")
        helpText = "This module provides a quick guide for choosing the registration option most optimal for your task. More detailed information can be found on Slicer's wiki: <a>http://www.slicer.org/slicerWiki/index.php/Slicer3:Registration</a>.\n\nThe following is a brief overview of the main and auxiliary modules Slicer has to offer for image registration. Most modules are generic and can handle any image content, but a few are designed specifically for brain images."
        aboutText = "This work is supported by NA-MIC, NAC, Harvard Catalyst, NCIGT, and the Slicer Community. See http://www.slicer.org for details.  Module implemented by Wendy Plesniak and Steve Pieper."
        self.BuildHelpAndAboutFrame(pageWidget,helpText,aboutText)
    
        moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        moduleFrame.SetParent(self.GetUIPanel().GetPageWidget("RegistrationWelcome"))
        moduleFrame.Create()
        moduleFrame.SetLabelText("RegistrationWelcome")
        moduleFrame.ExpandFrame()
        widgetName = moduleFrame.GetWidgetName()
        pageWidgetName = self.GetUIPanel().GetPageWidget("RegistrationWelcome").GetWidgetName()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (widgetName,pageWidgetName))

        text = slicer.vtkKWTextWithHyperlinksWithScrollbars()
        text.SetParent(moduleFrame.GetFrame())
        text.Create()
        text.SetHorizontalScrollbarVisibility(0)
        text.GetWidget().SetReliefToFlat()
        text.GetWidget().SetWrapToWord()
        text.GetWidget().QuickFormattingOn()
        text.GetWidget().SetHeight(5)
        text.SetText("This module provides a quick guide for choosing the registration option most optimal for your task. More detailed information can be found on Slicer's wiki: <a>http://www.slicer.org/slicerWiki/index.php/Slicer3:Registration</a>.\n\nThe following is a brief overview of the main and auxiliary modules Slicer has to offer for image registration. Most modules are generic and can handle any image content, but a few are designed specifically for brain images.")
        # Important that Read only after SetText otherwise it doesn't work
        text.GetWidget().ReadOnlyOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % text.GetWidgetName())

    def TearDownGUI(self):
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget("RegistrationWelcome")
            self.GetUIPanel().RemovePage("RegistrationWelcome")

