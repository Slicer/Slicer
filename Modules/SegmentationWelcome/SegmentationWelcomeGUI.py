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
        text.SetText("""This module provides a quick guide for choosing the segmentation options most appropriate for your task. More detailed information can be found on Slicer's wiki: <a>http://www.slicer.org/slicerWiki/index.php/Modules:SegmentationOverview3.6</a>.
        
The following provides a brief overview of the main modules Slicer has to offer for image segmentation. Selecting the best module will depend on your input data and the underlying questions you are asking. The spectrum of approaches ranges from fully automated to manual segmentation; most modules are generic and can handle any image content, but a few are designed specifically for brain images. Slicer modules are listed below:

**Interactive Editor**
Slicer's interactive editor is a module for manual segmentation of volumes. Some of the tools mimic a painting interface like photoshop or gimp, but work on 3D arrays of voxels rather than on 2D pixels. The overall goal is to allow users to efficiently and precisely define structures within their volumes as label map volumes. These label maps can be used for building models, or further processing (using, for example, the Label Statistics module).

**EM Segment Template Builder**
This module is designed for users who perform atlas based medical image segmentation and has been tested on brain MRI images. Familiarity with statistical modeling may help but is not required. It allows the user to configure the algorithm ---step-by-step--- to a variety of imaging protocols and anatomical structures, and then apply the algorithm to segment data. Configuration settings are stored in an EMSegment parameters node in the Slicer3 MRML tree. These settings can be saved and later applied to new data via any of the EMSegment interfaces within Slicer3 or the command-line EMSegment executable.

**EMSegment Command-Line**
This module is used to simplify the process of segmenting large collections of images by providing a command line interface to the EMSegment algorithm for script and batch processing. The primary function of the EMSegment Template Builder module is to step the user through the process of calibrating, via algorithm parameters, the segmentation algorithm to a particular set of input data. However, once a successful collection of parameters is established, the user will commonly want to bypass this detailed calibration process when segmenting new images by using those parameters collected from the calibration process in EMSegment Template Builder module. The command-line executable provides this batch processing capability.

**EMSegment Simple**
This module provide a simplified ~~one-click~~ GUI interface for the EMSegment Command-line Executable. The interface is simple because the number of required command-line parameters is minimized, and it is flexible because any EM algorithm parameter can be modified, within the MRML scene, via the EMSegment GUI interface. Similar to the EMSegment Command-line Executable, once a set of parameters for segmentation algorithm is deemed satisfactory through the process of calibrating in the Template Builder module, the user will commonly want to bypass this detailed calibration process when segmenting new images. This module provides a simple interface to specify a small number of required command-line parameters --- a MRML scene (containing algorithm parameters), a target image (or multiple target images, e.g., T1 and T2), and an output labelmap image --- and to allow any EM algorithm parameters to be modified, within the MRML scene, via the EMSegment GUI interface.

**Fast Marching Segmentation**
This module is most useful to segment image regions that have similar intensity. Initialization is accomlished by selecting points within the region to be segmented and expected volume of the segmented structure. The segmentation is completed relatively quickly for typical images, allowing experimentation with choosing optimal parameters. The resulting volume can be adjusted interactively by scrolling through the evolution of the label contour.

** Otsu Treshold Segmentation**
This filter creates a labeled image from a grayscale image. First, it calculates an optimal threshold that separates the image into foreground and background. This threshold separates those two classes so that their intra-class variance is minimal (see <a>http://en.wikipedia.org/wiki/Otsu%27s_method</a>). Then the filter runs a connected component algorithm to generate unique labels for each connected region of the foreground. Finally, the resulting image is relabeled to provide consecutive numbering.

** Simple Region Growing**
Simple Region Growing is a statistical region growing algorithm. The algorithm takes one or more seeds as input. It executes using the following steps:

    * A statistical model of the foreground (mean and standard deviation of intensity) is estimated over neighborhoods of the seed points. The statistical model is converted to a scalar threshold range using the mean intensity of a seed point plus or minus a multiplier or the standard deviation.

    * The algorithm then constructs a segmentation by labeling all voxels that are connected to the seed voxels and satisfy the scalar threshold range.

After this initial segmentation, the statistical model can be iteratively refined by re-calculating the mean and standard deviation of the intensity of the voxels in the initial segmentation. The refined statistical model in turn is converted to a new scalar threshold range as described in the preceding paragraph. This is followed by a new segmentation where the algorithm labels all voxels connected to the seed voxels and satisfy the new scalar threshold range. The number of repetitions for the segmentation process is specified using an iteration parameter to the algorithm.

Through this process, Simple Region Growing attempts to adapt to the statistical properties of the image. Initially, the statistical model is based strictly on the neighborhoods about the seeds. This statistical model is precise (being based on the user supplied seeds) but also uncertain (because the number of samples in the model can be rather small). After the initial segmentation, the statistics are recalculated which yields a more certain model (because the number of samples in the model can be rather large).

** Robust Statistics **
This module is a general purpose segmenter. The target object is initialized by a label map. An active contour model then evolves to extract the desired boundary of the object.

**BRAINSROIAuto**
BRAINSROIAuto automatically generates a Binary Image (or Mask) to encompass the region in an brain image volume occupied by the brain. In general, BRAINSROIAuto takes a brain image volume, and generates a mask without requiring parameter tweaking.
""")
        # Important that Read only after SetText otherwise it doesn't work
        text.GetWidget().ReadOnlyOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % text.GetWidgetName())

    def TearDownGUI(self):
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget("SegmentationWelcome")
            self.GetUIPanel().RemovePage("SegmentationWelcome")

