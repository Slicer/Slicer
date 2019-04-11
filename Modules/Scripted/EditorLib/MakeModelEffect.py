import os
import vtk
import qt
import ctk
import slicer
from . import HelpButton
from . import EditUtil
from . import EffectOptions, EffectTool, EffectLogic, Effect

__all__ = [
  'MakeModelEffectOptions',
  'MakeModelEffectTool',
  'MakeModelEffectLogic',
  'MakeModelEffect'
  ]

#########################################################
#
#
comment = """

  MakeModelEffect is a subclass of Effect (for tools that plug into the
  slicer Editor module) for making model nodes from the
  current paint label

# TODO :
"""
#
#########################################################

#
# MakeModelEffectOptions - see EditOptions and Effect for superclasses
#

class MakeModelEffectOptions(EffectOptions):
  """ MakeModelEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(MakeModelEffectOptions,self).__init__(parent)
    # create a logic instance to do the non-gui work
    # (since this is created from the option gui it has no slice logic)
    self.logic = MakeModelEffectLogic(None)

  def __del__(self):
    super(MakeModelEffectOptions,self).__del__()

  def create(self):
    super(MakeModelEffectOptions,self).create()

    self.goToModelMaker = qt.QPushButton("Go To Model Maker", self.frame)
    self.goToModelMaker.setToolTip( "The Model Maker interface contains a whole range of options for building sets of models and controlling the parameters." )
    self.frame.layout().addWidget(self.goToModelMaker)
    self.widgets.append(self.goToModelMaker)

    self.smooth = qt.QCheckBox("Smooth Model", self.frame)
    self.smooth.checked = True
    self.smooth.setToolTip("When smoothed, the model will look better, but some details of the label map will not be visible on the model.  When not smoothed you will see individual voxel boundaries in the model.  Smoothing here corresponds to Decimation of 0.25 and Smoothing iterations of 10.")
    self.frame.layout().addWidget(self.smooth)
    self.widgets.append(self.smooth)

    #
    # model name
    #
    self.nameFrame = qt.QFrame(self.frame)
    self.nameFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.nameFrame)
    self.widgets.append(self.nameFrame)

    self.modelNameLabel = qt.QLabel("Model Name: ", self.nameFrame)
    self.modelNameLabel.setToolTip( "Select the name for the newly created model." )
    self.nameFrame.layout().addWidget(self.modelNameLabel)
    self.widgets.append(self.modelNameLabel)

    self.modelName = qt.QLineEdit(self.nameFrame)
    self.modelName.setText( self.getUniqueModelName( EditUtil.getLabelName() ) )
    self.nameFrame.layout().addWidget(self.modelName)
    self.widgets.append(self.modelName)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.objectName = self.__class__.__name__ + 'Apply'
    self.apply.setToolTip("Build a model for the current label value of the label map being edited in the Red slice window.  Model will be created in the background." )
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool build a model.  A subset of model building options is provided here.  Go to the Model Maker module to expose a range of parameters.  Use Merge and Build button in the Advanced... tab to quickly make a model of all defined structures in the merge label map.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

    self.connections.append( (self.apply, 'clicked()', self.onApply) )
    self.connections.append( (self.goToModelMaker, 'clicked()', self.onGoToModelMaker) )

  def destroy(self):
    super(MakeModelEffectOptions,self).destroy()

  def updateParameterNode(self, caller, event):
    """
    note: this method needs to be implemented exactly as
    defined in the leaf classes in EditOptions.py
    in each leaf subclass so that "self" in the observer
    is of the correct type """
    node = EditUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def checkForModelMakerModule(self):
    try:
      modelMaker = slicer.modules.modelmaker
      return True
    except AttributeError:
      slicer.util.errorDisplay('The ModelMaker module is not available<p>Perhaps it was disabled in the application '
                               'settings or did not load correctly.', windowTitle='Editor')
      return False

  def onGoToModelMaker(self):
    if self.checkForModelMakerModule():
      m = slicer.util.mainWindow()
      m.moduleSelector().selectModule('ModelMaker')

  def onApply(self):
    if self.checkForModelMakerModule():
      #
      # run the task (in the background)
      # - use the GUI to provide progress feedback
      # - use the GUI's Logic to invoke the task
      # - model will show up when the processing is finished
      #
      slicer.util.showStatusMessage( "Model Making Started...", 2000 )
      self.logic.makeModel(self.modelName.text,self.smooth.checked)

  def setMRMLDefaults(self):
    super(MakeModelEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(MakeModelEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(MakeModelOptions,self).updateMRMLFromGUI()
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

  def getUniqueModelName(self, baseName):
      names = slicer.util.getNodes().keys()
      name = baseName
      index = 0
      while names.__contains__(name):
        index += 1
        name = "%s %d" % (baseName, index)
      return name


#
# MakeModelEffectTool
#

class MakeModelEffectTool(EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self,sliceWidget):
    super(MakeModelEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    super(MakeModelEffectTool,self).cleanup()

#
# MakeModelEffectLogic
#

class MakeModelEffectLogic(EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an MakeModelEffectTool
  or MakeModelEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the MakeModelEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(MakeModelEffectLogic,self).__init__(sliceLogic)

  def makeModel(self,modelName='EditorModel',smooth=True):
    #
    # create a model using the command line module
    # based on the current editor parameters
    #

    volumeNode = EditUtil.getLabelVolume()
    if not volumeNode:
      return

    #
    # set up the model maker node
    #

    parameters = {}
    parameters['Name'] = modelName
    parameters["InputVolume"] = volumeNode.GetID()
    parameters['FilterType'] = "Sinc"

    # build only the currently selected model.
    parameters['Labels'] = EditUtil.getLabel()
    parameters["StartLabel"] = -1
    parameters["EndLabel"] = -1

    parameters['GenerateAll'] = False
    parameters["JointSmoothing"] = False
    parameters["SplitNormals"] = True
    parameters["PointNormals"] = True
    parameters["SkipUnNamed"] = True

    if smooth:
      parameters["Decimate"] = 0.25
      parameters["Smooth"] = 10
    else:
      parameters["Decimate"] = 0
      parameters["Smooth"] = 0

    #
    # output
    # - make a new hierarchy node if needed
    #
    numNodes = slicer.mrmlScene.GetNumberOfNodesByClass( "vtkMRMLModelHierarchyNode" )
    outHierarchy = None
    for n in range(numNodes):
      node = slicer.mrmlScene.GetNthNodeByClass( n, "vtkMRMLModelHierarchyNode" )
      if node.GetName() == "Editor Models":
        outHierarchy = node
        break

    if not outHierarchy:
      outHierarchy = slicer.vtkMRMLModelHierarchyNode()
      outHierarchy.SetScene( slicer.mrmlScene )
      outHierarchy.SetName( "Editor Models" )
      slicer.mrmlScene.AddNode( outHierarchy )

    parameters["ModelSceneFile"] = outHierarchy

    modelMaker = slicer.modules.modelmaker

    #
    # run the task (in the background)
    # - use the GUI to provide progress feedback
    # - use the GUI's Logic to invoke the task
    # - model will show up when the processing is finished
    #
    slicer.cli.run(modelMaker, None, parameters)

    slicer.util.showStatusMessage( "Model Making Started...", 2000 )

#
# The MakeModelEffect class definition
#

class MakeModelEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. MakeModelEffect.png)
    self.name = "MakeModelEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "MakeModelEffect: Generic abstract labeling effect - not meant to be instanced"

    self.options = MakeModelEffectOptions
    self.tool = MakeModelEffectTool
    self.logic = MakeModelEffectLogic
