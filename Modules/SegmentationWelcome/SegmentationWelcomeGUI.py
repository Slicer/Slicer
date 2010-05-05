from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer

vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000
vtkKWPushButton_InvokedEvent = 10000

class SegmentationWelcomeGUI(ScriptedModuleGUI):

    def __init__(self):
        ScriptedModuleGUI.__init__(self)
        self.vtkScriptedModuleGUI.SetCategory("Segmentation")
    
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
        self.GetUIPanel().AddPage("SegmentationWelcome","SegmentationWelcome","")
        pageWidget = self.GetUIPanel().GetPageWidget("SegmentationWelcome")
        helpText = "This module provides a quick guide for choosing the segmentation options most appropriate for your task. More detailed information can be found on Slicer's wiki: <a>http://www.slicer.org/slicerWiki/index.php/Modules:SegmentationOverview3.6</a>.\n\nThe following provides a brief overview of the main and auxiliary  modules Slicer has to offer for image segmentation. Selecting the best module will depend on your input data and the underlying questions you are asking. The spectrum of approaches ranges from fully automated to manual segmentation; most modules are generic and can handle any image content, but a few are designed specifically for brain images."
        aboutText = "This work is supported by NA-MIC, NAC, Harvard Catalyst, NCIGT, and the Slicer Community. See http://www.slicer.org for details.  Module implemented by Wendy Plesniak and Steve Pieper."
        self.BuildHelpAndAboutFrame(pageWidget,helpText,aboutText)
    
        moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        moduleFrame.SetParent(self.GetUIPanel().GetPageWidget("SegmentationWelcome"))
        moduleFrame.Create()
        moduleFrame.SetLabelText("SegmentationWelcome")
        moduleFrame.ExpandFrame()
        widgetName = moduleFrame.GetWidgetName()
        pageWidgetName = self.GetUIPanel().GetPageWidget("SegmentationWelcome").GetWidgetName()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (widgetName,pageWidgetName))

        text = slicer.vtkKWTextWithHyperlinksWithScrollbars()
        text.SetParent(moduleFrame.GetFrame())
        text.Create()
        text.SetHorizontalScrollbarVisibility(0)
        text.GetWidget().SetReliefToFlat()
        text.GetWidget().SetWrapToWord()
        text.GetWidget().QuickFormattingOn()
        text.GetWidget().SetHeight(35)
        text.SetText("This module provides a quick guide for choosing the segmentation options most appropriate for your task. More detailed information can be found on Slicer's wiki: <a>http://www.slicer.org/slicerWiki/index.php/Modules:SegmentationOverview3.6</a>.\n\nThe following provides a brief overview of the main and auxiliary  modules Slicer has to offer for image segmentation. Selecting the best module will depend on your input data and the underlying questions you are asking. The spectrum of approaches ranges from fully automated to manual segmentation; most modules are generic and can handle any image content, but a few are designed specifically for brain images.")
        # Important that Read only after SetText otherwise it doesn't work
        text.GetWidget().ReadOnlyOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % text.GetWidgetName())

    def TearDownGUI(self):
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget("SegmentationWelcome")
            self.GetUIPanel().RemovePage("SegmentationWelcome")

