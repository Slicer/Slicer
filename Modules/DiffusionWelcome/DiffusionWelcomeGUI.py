from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer

vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000
vtkKWPushButton_InvokedEvent = 10000

class DiffusionWelcomeGUI(ScriptedModuleGUI):
    _category = "Diffusion"
    _title = "DiffusionWelcome"
    _helpText="""
  This module provides a brief description of every Diffusion Imaging module in 3D Slicer. The usual processing pipeline and more detailed information can be found on Slicer's wiki: <a>http://www.slicer.org/slicerWiki/index.php/Modules:DiffusionMRIWelcome-Documentation-3.6</a>."""
    _mainText = """
**DWI DENOSING**



Three techniques are provided for denoising DW images:

 ** Unbiased Non Local Means filter for DWI**: Is the one providing the most visually appealing results. However, it is very time consuming and may mix information from remote areas of the image.
 
 ** Rician LMMSE Image Filter**: Estimates Rician noise and uses this estimation and spatial coherence to perform the denoising. It processes each gradient direction individually.
 
 ** Joint Rician LMMSE Image Filter**: Estimates Rician noise and uses this estimation, spatial and orientational coherence to perform the denoising. It jointly processes several gradient directions.


**DIFFUSION TENSOR UTILITIES**



 ** Diffusion Tensor Estimation**: Produces a diffusion tensor image from a DW image.   
 
 ** Diffusion Tensor Scalar Measurements**: Calculates scalar invariants such as the fractional anisotropy (FA) or the linear measure (LM) from a diffusion tensor image.  
 
 ** Resample DTI Volume**: Increases or decreases the resolution of a diffusion tensor image.



**TRACTOGRAPHY**

 ** Label Seeding**: Deterministic tracing of the white matter fibers traversing a specified labeled region of the diffusion tensor image.

 **Fiducial Seeding**: Deterministic tracing of the white matter fibers traversing each fiducial from a fiducial list.

 **FiberBundles**: Tuning of the visualization options for the deterministic tractography results produced with the Label Seeding or Fiducial Seeding modules.

 **Stochastic Tractography**: Probabilistically traces the white matter fibers connecting two regions. The output of this module is an image quantifying the probability that white matter fiber connecting two selected regions traverses each point in space.

 **ROI Select**: Filters tracts produced by determinictractography passing through a region of interest expressed as a labeled region on the diffusion tensor image. 

    """
    _aboutText = """
    This work is supported by NA-MIC, NAC, Harvard Catalyst, NCIGT, and the Slicer Community. See http://www.slicer.org for details.  Module implemented by Demian Wassermann."
      """
    def __init__(self):
        ScriptedModuleGUI.__init__(self)
        self.vtkScriptedModuleGUI.SetCategory(self._category)
    
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
        self.GetUIPanel().AddPage(self._title,self._title,"")
        pageWidget = self.GetUIPanel().GetPageWidget("DiffusionWelcome")
        self.BuildHelpAndAboutFrame(pageWidget,self._helpText,self._aboutText)
    
        moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        moduleFrame.SetParent(self.GetUIPanel().GetPageWidget(self._title))
        moduleFrame.Create()
        moduleFrame.SetLabelText(self._title)
        moduleFrame.ExpandFrame()
        widgetName = moduleFrame.GetWidgetName()
        pageWidgetName = self.GetUIPanel().GetPageWidget("DiffusionWelcome").GetWidgetName()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (widgetName,pageWidgetName))

        text = slicer.vtkKWTextWithHyperlinksWithScrollbars()
        text.SetParent(moduleFrame.GetFrame())
        text.Create()
        text.SetHorizontalScrollbarVisibility(0)
        text.GetWidget().SetReliefToFlat()
        text.GetWidget().SetWrapToWord()
        text.GetWidget().QuickFormattingOn()
        text.GetWidget().SetHeight(35)
        text.SetText(self._helpText+"\n\n\n"+self._mainText)
        # Important that Read only after SetText otherwise it doesn't work
        text.GetWidget().ReadOnlyOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % text.GetWidgetName())

    def TearDownGUI(self):
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget(self._title)
            self.GetUIPanel().RemovePage(self._title)




