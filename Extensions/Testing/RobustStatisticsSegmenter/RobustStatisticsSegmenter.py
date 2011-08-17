from __main__ import vtk, qt, ctk, slicer
import EditorLib
from EditorLib.EditOptions import HelpButton

#
# The Editor Extenion itself.
# 
# This needs to define the hooks to be come an editor effect.
#

class RobustStatisticsSegmenterOptions(EditorLib.EditOptions):
  """ RobustStatisticsSegmenter-specfic gui
  """

  def __init__(self, parent=0):
    self.CLINode = None
    super(RobustStatisticsSegmenterOptions,self).__init__(parent)
    # self.attributes should be tuple of options:
    # 'MouseTool' - grabs the cursor
    # 'Nonmodal' - can be applied while another is active
    # 'Disabled' - not available    
    self.attributes = ('MouseTool', 'Nonmodal')
    self.displayName = 'Editor Extension Template'

  def __del__(self):
    super(RobustStatisticsSegmenterOptions,self).__del__()

  def create(self):
    
    super(RobustStatisticsSegmenterOptions,self).create()
    self.goToRobustStatisticsSegmenter = qt.QPushButton("Go To RSS Segmenter",self.frame)
    self.goToRobustStatisticsSegmenter.setToolTip( "Robust Statistics Segmentation - allows fast segmentation of 2D/3D data given an initial seed." )
    self.frame.layout().addWidget(self.goToRobustStatisticsSegmenter)
    self.widgets.append(self.goToRobustStatisticsSegmenter)
    
    # gui widgets for exposing some parameter control
    self.smooth_boundary = qt.QCheckBox("Smooth Boundary?", self.frame)
    self.good_contrast = qt.QCheckBox("Good Contrast?", self.frame)
    self.smooth_boundary.checked = True
    self.good_contrast.checked = True
    self.smooth_boundary.setToolTip("Is the boundary of the region smooth (not jagged)?.")
    self.good_contrast.setToolTip("Is the boundary of the region visually very clear?.")
    self.frame.layout().addWidget(self.smooth_boundary)
    self.frame.layout().addWidget(self.good_contrast)
    self.widgets.append(self.smooth_boundary)
    self.widgets.append(self.good_contrast)
  
    # create button group for selecting size   
    self.region_small = qt.QRadioButton("Small ~10mL", self.frame)
    self.region_small.setToolTip("about the size of a grape")
    self.region_medium = qt.QRadioButton("Medium ~100mL", self.frame)
    self.region_medium.setToolTip("about the size of a golf ball")
    self.region_large = qt.QRadioButton("Large ~500mL", self.frame)
    self.region_large.setToolTip("about the size of your fist")
    self.region_medium.checked = True
    self.frame.layout().addWidget(self.region_small)
    self.widgets.append(self.region_small)
    self.frame.layout().addWidget(self.region_medium)
    self.widgets.append(self.region_medium)
    self.frame.layout().addWidget(self.region_large)
    self.widgets.append(self.region_large)   
    
    #
    # Setup parameters [minimalist & intuitive]
    #
    
    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Segment the region of interest from the seed provided - using the Robust Statistical Segmenter" )
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to segment 3D regions.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

    self.apply.connect('clicked()', self.onApply)
    self.goToRobustStatisticsSegmenter.connect('clicked()', self.onGoToRobustStatisticsSegmenter)
    
    # disable segmentation unless seed is selected
    self.confirmSeedSelection()

  def confirmSeedSelection(self):
    
    labelVolume = self.editUtil.getLabelVolume()
    backgroundVolume = self.editUtil.getBackgroundVolume()

    if (not labelVolume) or (not backgroundVolume):      
      self.goToRobustStatisticsSegmenter.enabled = False
      self.smooth_boundary.enabled = False
      self.good_contrast.enabled = False
      self.region_small.enabled = False
      self.region_medium.enabled = False
      self.region_large.enabled = False
      self.apply.enabled = False
      #dialog = qt.QErrorMessage(self.parent)
      #dialog.showMessage("Need a background volume and label volume (seed) to perform segmentation!")
    else:
      labelID = self.editUtil.getLabelID()
      labelNode = slicer.mrmlScene.GetNodeByID(labelID)
      labelImage = labelNode.GetImageData()

      backgroundID  = self.editUtil.getBackgroundID()
      backgroundNode = slicer.mrmlScene.GetNodeByID(backgroundID)
      backgroundImage = backgroundNode.GetImageData()

      labelDim = labelImage.GetDimensions()
      backgroundDim = backgroundImage.GetDimensions()
    
      if labelDim != backgroundDim: 
        dialog = qt.QErrorMessage(self.parent)
        dialog.showMessage("Label Image and Background Image Dimensions don't match!")
      else:
        self.goToRobustStatisticsSegmenter.enabled = True
        self.smooth_boundary.enabled = True
        self.good_contrast.enabled = True
        self.region_small.enabled = True
        self.region_medium.enabled = True
        self.region_large.enabled = True
        self.apply.enabled = True   
    
    
  def onGoToRobustStatisticsSegmenter(self):
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModuleByTitle('Robust Statistics Segmentation')
  
  def destroy(self):
    super(RobustStatisticsSegmenterOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver("ModifiedEvent", self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(RobustStatisticsSegmenterOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(RobustStatisticsSegmenterOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def onApply(self):
    labelVolume = self.editUtil.getLabelVolume()
    if not labelVolume:
      return

    backgroundVolume = self.editUtil.getBackgroundVolume()
    if not backgroundVolume:
      return
    
    #
    # set up the segmenter node
    #
    
    parameters = {}
    # segmentation parameters
    parameters['expectedVolume'] = 100
    if self.region_small.isChecked()==True:
      parameters['expectedVolume'] = 10
    if self.region_medium.isChecked()==True:
      parameters['expectedVolume'] = 100
    if self.region_large.isChecked()==True:
      parameters['expectedVolume'] = 500
    
    print "expectedVolume = "+str(parameters['expectedVolume'])
    
    # auxiliary parameters
    
    good_contrast = 0.7
    bad_contrast = 0.3
    smooth_boundary = 0.7
    rough_boundary = 0.3
    
    parameters['intensityHomogeneity'] = good_contrast
    if self.good_contrast.isChecked()==True:
      parameters['intensityHomogeneity'] = good_contrast
    else:
      parameters['intensityHomogeneity'] = bad_contrast
    
    print "intensityHomogeneity = "+str(parameters['intensityHomogeneity'])
    
    parameters['curvatureWeight'] = smooth_boundary
    if self.smooth_boundary.isChecked()==True:
      parameters['curvatureWeight'] = smooth_boundary
    else:
      parameters['curvatureWeight'] = rough_boundary
      
    print "curvatureWeight = "+str(parameters['curvatureWeight'])
    
    #parameters['labelValue'] = 1
    parameters['labelValue'] = self.getPaintLabel()
    parameters['maxRunningTime'] = 10
    
    # IO parameters
    
      
    parameters['originalImageFileName'] = backgroundVolume.GetID()
    parameters['labelImageFileName'] = labelVolume.GetID()
    parameters['segmentedImageFileName'] = labelVolume.GetID()

    RobustStatisticsSegmenterCLImodule = slicer.modules.robuststatsegmenter

    # 
    # run the task (in the background)
    # - use the GUI to provide progress feedback
    # - use the GUI's Logic to invoke the task
    # - segmentation will show up when the processing is finished
    #
    self.CLINode = slicer.cli.run(RobustStatisticsSegmenterCLImodule, self.CLINode, parameters)

    self.statusText( "RSS Segmentation Started..." )

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(RobustStatisticsSegmenterOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# RobustStatisticsSegmenter
#

class RobustStatisticsSegmenter:
  """
  This class is the 'hook' for slicer to detect and recognize the extension
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "Robust Statistics Segmenter"
    parent.category = "Editor Extensions"
    parent.contributor = "Kedar Patwardhan"
    parent.helpText = "Simple GUI tool to instantiate Robust Statistics Segmenter using intuitive parameter settings."
    parent.acknowledgementText = """
    This editor extension was developed by 
    <Kedar Patwardhan>, <GE Global Research>
    based on work by:
    Steve Pieper, Isomics, Inc.
    based on work by:
    Jean-Christophe Fillion-Robin, Kitware Inc.
    and was partially funded by NIH grant 3P41RR013218.
    """

    # TODO:
    # don't show this module - it only appears in the Editor module
    #parent.hidden = True

    # Add this extension to the editor's list for discovery when the module
    # is created.  Since this module may be discovered before the Editor itself,
    # create the list if it doesn't already exist.
    try:
      slicer.modules.editorExtensions
    except AttributeError:
      slicer.modules.editorExtensions = {}
    slicer.modules.editorExtensions['RobustStatisticsSegmenterOptions'] = RobustStatisticsSegmenterOptions

#
# RobustStatisticsSegmenter
#

class RobustStatisticsSegmenterWidget:
  def __init__(self, parent = None):
    self.parent = parent
    
  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass
    
  def exit(self):
    pass


