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
        text.GetWidget().SetHeight(35)
        text.SetText("""This module provides a quick guide for choosing the registration option most optimal for your task. More detailed information can be found on Slicer's wiki: <a>http://www.slicer.org/slicerWiki/index.php/Slicer3:Registration</a>.\n\nThe following is a brief overview of the main and auxiliary modules Slicer has to offer for image registration. Most modules are generic and can handle any image content, but a few are designed specifically for brain images. Slicer modules are listed below by category:\n\n

**FAST REGISTRATION**\n\n

**  Transforms:** The Transformations module creates and edits slicer Transform nodes. Transformation nodes are used in Slicer to define spacial relationships between different nodes (such as volumes, models, fiducials, ROI's, or other Transform nodes) or between the nodes and the global RAS space. You can establish these relations by dragging the nodes under the Transformation nodes in the Data module. \n

**  Fast Affine Registration:** This module implements a registration algorithm based on the Mattes mutual information registration metric and the affine transformation. Both the fixed and moving images may be optionally smoothed before registration. \n

**  Fast Rigid Registration:** This module implements a registration algorithm based on the Mattes mutual information registration metric. The transformation mapping the moving image to the fixed image consists of 3 translations and 3 rotations. Thus, only rigid body transformations are permitted. Both the fixed and moving images may be optionally smoothed before registration. The module optionally breaks the optimization into multiple stages, each with a different learning rate and number of iterations. \n

**  Fast Nonrigid Registration:** This module performs non-rigid alignment/registration of two volumes based on a B-spline interpolation scheme, driven by points on a (cubic) control grid. The number of points on the grid is expected to be low to obtain a fast and feasible solution. Output consists in a transform node and/or resampled target volume. For finer grids a scale-space approach of multiple runs with gradually increasing grid size is recommended. Note that non-rigid transforms as produced here are not available for instant view in the data module in the way linear transforms are, i.e. moving a new volume under a B-spline transform in the Data/MRML tree will not show an effect in the views. To see the result the volume must be resampled, using the ResampleScalarVectorDWIVolume module. \n

**ROBUST REGISTRATION**\n\n

**  Expert Automated Registration:** This module is an integrated framework providing access to ITK registration technologies. Algorithms can be run in single mode or pipelined. Depending on the size of the data sets, a significant amount of memory is needed. There is an option to trade off speed for memory. Most of the code is parallelized and will take advantage of multicore capabilities, if available. \n

**  Robust Multiresolution Affine Registration:** This module implements mutual information based affine registration using a multi-resolution optimization strategy. Several parts of the algorithm are based on a description of the FLIRT algorithm, see the Reference section. \n

**  BRAINSDemoWarp:** BRAINSDemonWarp is a command line program for image registration by using different methods including Thirion and diffeomorphic demons algorithms. The function takes in a template image and a target image along with other optional parameters and registers the template image onto the target image. The resultant deformation fields and metric values can be written to a file. The program uses the Insight Toolkit (www.ITK.org) for all the computations, and can operate on any of the image types supported by that library. \n

**  BRAINSFit:** \n

**  BRAINSResample:** \n

**BRAIN-ONLY REGISTRATION**\n\n

**  ACPC Transform:** The ACPC Transform Module is used to orient brain images along predefined anatomical landmarks: (manually defined) fiducials for the inter-hemispheral midline, anterior- and posterior commissure are used to align an image such that these landmarks become vertical and horizontal, respectively. This transformation can then be applied to a volume using the Resample Scalar/Vector/DWI Volume module. **Note: renamed from RealignVolume**. \n

**NON-RASTER-IMAGE DATA REGISTRATION**\n\n

**  Fiducial Registration:** The Fiducial Alignment Module can align images based on pairs of manually selected fiducial points (rigid and affine). Two sets of fiducials (fiducial lists) are required, forming matching pairs to be aligned. The transform can either be translation only, rigid transform, or similarity transform. \n

**  Surface Registration: ** The ICP Surface Registration Module performs automated registration of surfaces (not images) using the Iterative Closest Point algorithm using rigid, similarity and affine transforms. This is useful if image data directly is unreliable, but surfaces can be produced from segmentations that provide good information about desired alignment. \n """)
        
        # Important that Read only after SetText otherwise it doesn't work
        text.GetWidget().ReadOnlyOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % text.GetWidgetName())

    def TearDownGUI(self):
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget("RegistrationWelcome")
            self.GetUIPanel().RemovePage("RegistrationWelcome")

