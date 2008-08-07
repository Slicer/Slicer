from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer

vtkKWScale_ScaleValueChangedEvent = 10001
vtkKWScale_ScaleValueStartChangingEvent = 10002
vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000
vtkKWPushButton_InvokedEvent = 10000

class PythonGADScriptedModuleGUI(ScriptedModuleGUI):

    def __init__(self):
        ScriptedModuleGUI.__init__(self)
    
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

    def Apply(self):
        if not self.GetScriptedModuleNode():
          slicer.Application.ErrorMessage("No input ScriptedModuleNode found")
          return
       
        slicer.Application.InformationMessage("Done applying GradientAnisotropicDiffusion.")

    def UpdateMRML(self):
        pass

    def UpdateGUI(self):
        pass

    def ProcessMRMLEvents(self,caller,event):
        pass

    def BuildGUI(self):
        return
        self.GetUIPanel().AddPage("GradientAnisotropicDiffusionFilter","GradientAnisotropicDiffusionFilter","")
        pageWidget = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter")
        helpText = "Flex, dude!"
        aboutText = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details."
        self.BuildHelpAndAboutFrame(pageWidget,helpText,aboutText)
    
        moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        moduleFrame.SetParent(self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter"))
        moduleFrame.Create()
        moduleFrame.SetLabelText("Gradient Anisotropic Diffusion Filter")
        moduleFrame.ExpandFrame()
        widgetName = moduleFrame.GetWidgetName()
        pageWidgetName = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter").GetWidgetName()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (widgetName,pageWidgetName))

    def TearDownGUI(self):
        return
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter")
            self.GetUIPanel().RemovePage("GradientAnisotropicDiffusionFilter")

