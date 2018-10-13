import os
import unittest
import string
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

class SurfaceToolbox(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Surface Toolbox"
    self.parent.categories = ["Surface Models"]
    self.parent.dependencies = []
    self.parent.contributors = ["Luca Antiga (Orobix), Ron Kikinis (Brigham and Women's Hospital)"] # replace with "Firstname Lastname (Org)"
    self.parent.helpText = """
This module supports various cleanup and optimization processes on surface models.
Select the input and output models, and then enable the stages of the pipeline by selecting the buttons.
Stages that include parameters will open up when they are enabled.
Click apply to activate the pipeline and then click the Toggle button to compare the model before and after the operation.
"""
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = """
This module was developed by Luca Antiga, Orobix Srl, with a little help from Steve Pieper, Isomics, Inc.
"""

def numericInputFrame(parent, label, tooltip, minimum, maximum, step, decimals):
  inputFrame = qt.QFrame(parent)
  inputFrame.setLayout(qt.QHBoxLayout())
  inputLabel = qt.QLabel(label, inputFrame)
  inputLabel.setToolTip(tooltip)
  inputFrame.layout().addWidget(inputLabel)
  inputSpinBox = qt.QDoubleSpinBox(inputFrame)
  inputSpinBox.setToolTip(tooltip)
  inputSpinBox.minimum = minimum
  inputSpinBox.maximum = maximum
  inputSpinBox.singleStep = step
  inputSpinBox.decimals = decimals
  inputFrame.layout().addWidget(inputSpinBox)
  inputSlider = ctk.ctkDoubleSlider(inputFrame)
  inputSlider.minimum = minimum
  inputSlider.maximum = maximum
  inputSlider.orientation = 1
  inputSlider.singleStep = step
  inputSlider.setToolTip(tooltip)
  inputFrame.layout().addWidget(inputSlider)
  return inputFrame, inputSlider, inputSpinBox


class SurfaceToolboxWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...

    inputModelSelectorFrame = qt.QFrame(self.parent)
    inputModelSelectorFrame.setLayout(qt.QHBoxLayout())
    self.parent.layout().addWidget(inputModelSelectorFrame)

    inputModelSelectorLabel = qt.QLabel("Input Model: ", inputModelSelectorFrame)
    inputModelSelectorLabel.setToolTip( "Select the input model")
    inputModelSelectorFrame.layout().addWidget(inputModelSelectorLabel)

    inputModelSelector = slicer.qMRMLNodeComboBox(inputModelSelectorFrame)
    inputModelSelector.nodeTypes = ["vtkMRMLModelNode"]
    inputModelSelector.selectNodeUponCreation = False
    inputModelSelector.addEnabled = False
    inputModelSelector.removeEnabled = False
    inputModelSelector.noneEnabled = True
    inputModelSelector.showHidden = False
    inputModelSelector.showChildNodeTypes = False
    inputModelSelector.setMRMLScene( slicer.mrmlScene )
    inputModelSelectorFrame.layout().addWidget(inputModelSelector)

    outputModelSelectorFrame = qt.QFrame(self.parent)
    outputModelSelectorFrame.setLayout(qt.QHBoxLayout())
    self.parent.layout().addWidget(outputModelSelectorFrame)

    outputModelSelectorLabel = qt.QLabel("Output Model: ", outputModelSelectorFrame)
    outputModelSelectorLabel.setToolTip( "Select the output model")
    outputModelSelectorFrame.layout().addWidget(outputModelSelectorLabel)

    outputModelSelector = slicer.qMRMLNodeComboBox(outputModelSelectorFrame)
    outputModelSelector.nodeTypes = ["vtkMRMLModelNode"]
    outputModelSelector.selectNodeUponCreation = False
    outputModelSelector.addEnabled = True
    outputModelSelector.renameEnabled = True
    outputModelSelector.removeEnabled = True
    outputModelSelector.noneEnabled = True
    outputModelSelector.showHidden = False
    outputModelSelector.showChildNodeTypes = False
    outputModelSelector.baseName = "Model"
    outputModelSelector.selectNodeUponCreation = True
    outputModelSelector.setMRMLScene( slicer.mrmlScene )
    outputModelSelectorFrame.layout().addWidget(outputModelSelector)

    decimationButton = qt.QPushButton("Decimation")
    decimationButton.checkable = True
    self.layout.addWidget(decimationButton)
    decimationFrame = qt.QFrame(self.parent)
    self.layout.addWidget(decimationFrame)
    decimationFormLayout = qt.QFormLayout(decimationFrame)

    reductionFrame, reductionSlider, reductionSpinBox = numericInputFrame(self.parent,"Reduction:",
      "Specifies the desired reduction in the total number of polygons (e.g., if Reduction is set"
      +" to 0.9, this filter will try to reduce the data set to 10% of its original size).", 0.0,1.0,0.05,2)
    decimationFormLayout.addWidget(reductionFrame)

    boundaryDeletionCheckBox = qt.QCheckBox("Boundary deletion")
    decimationFormLayout.addWidget(boundaryDeletionCheckBox)

    smoothingButton = qt.QPushButton("Smoothing")
    smoothingButton.checkable = True
    self.layout.addWidget(smoothingButton)
    smoothingFrame = qt.QFrame(self.parent)
    self.layout.addWidget(smoothingFrame)
    smoothingFormLayout = qt.QFormLayout(smoothingFrame)

    smoothingMethodCombo = qt.QComboBox(smoothingFrame)
    smoothingMethodCombo.addItem("Laplace")
    smoothingMethodCombo.addItem("Taubin")
    smoothingFormLayout.addWidget(smoothingMethodCombo)

    laplaceMethodFrame = qt.QFrame(self.parent)
    smoothingFormLayout.addWidget(laplaceMethodFrame)
    laplaceMethodFormLayout = qt.QFormLayout(laplaceMethodFrame)

    laplaceIterationsFrame, laplaceIterationsSlider, laplaceIterationsSpinBox = numericInputFrame(self.parent,"Iterations:",
      "Determines the maximum number of smoothing iterations. Higher value allows more smoothing."
      +" In general, small relaxation factors and large numbers of iterations are more stable than"
      +" larger relaxation factors and smaller numbers of iterations. ",0.0,500.0,1.0,0)
    laplaceMethodFormLayout.addWidget(laplaceIterationsFrame)

    laplaceRelaxationFrame, laplaceRelaxationSlider, laplaceRelaxationSpinBox = numericInputFrame(self.parent,"Relaxation:",
      "Specifies how much points may be displaced during each iteration. Higher value results in more smoothing.",0.0,1.0,0.1,1)
    laplaceMethodFormLayout.addWidget(laplaceRelaxationFrame)

    taubinMethodFrame = qt.QFrame(self.parent)
    smoothingFormLayout.addWidget(taubinMethodFrame)
    taubinMethodFormLayout = qt.QFormLayout(taubinMethodFrame)

    taubinIterationsFrame, taubinIterationsSlider, taubinIterationsSpinBox = numericInputFrame(self.parent,"Iterations:",
      "Determines the maximum number of smoothing iterations. Higher value allows more accurate smoothing."
      +" Typically 10-20 iterations are enough.",0.0,100.0,1.0,0)
    taubinMethodFormLayout.addWidget(taubinIterationsFrame)

    taubinPassBandFrame, taubinPassBandSlider, taubinPassBandSpinBox = numericInputFrame(self.parent,"Pass Band:",
      "Number between 0 and 2. Lower values produce more smoothing.",0.0,2.0,0.0001,4)
    taubinMethodFormLayout.addWidget(taubinPassBandFrame)

    boundarySmoothingCheckBox = qt.QCheckBox("Boundary Smoothing")
    smoothingFormLayout.addWidget(boundarySmoothingCheckBox)

    normalsButton = qt.QPushButton("Normals")
    normalsButton.checkable = True
    self.layout.addWidget(normalsButton)
    normalsFrame = qt.QFrame(self.parent)
    self.layout.addWidget(normalsFrame)
    normalsFormLayout = qt.QFormLayout(normalsFrame)

    autoOrientNormalsCheckBox = qt.QCheckBox("Auto-orient Normals")
    autoOrientNormalsCheckBox.setToolTip("Orient the normals outwards from closed surface")
    normalsFormLayout.addWidget(autoOrientNormalsCheckBox)

    flipNormalsCheckBox = qt.QCheckBox("Flip Normals")
    flipNormalsCheckBox.setToolTip("Flip normal direction from its current or auto-oriented state")
    normalsFormLayout.addWidget(flipNormalsCheckBox)

    splittingCheckBox = qt.QCheckBox("Splitting")
    normalsFormLayout.addWidget(splittingCheckBox)

    featureAngleFrame, featureAngleSlider, featureAngleSpinBox = numericInputFrame(self.parent,"Feature Angle:","Tooltip",0.0,180.0,1.0,0)
    normalsFormLayout.addWidget(featureAngleFrame)

    mirrorButton = qt.QPushButton("Mirror")
    mirrorButton.checkable = True
    self.layout.addWidget(mirrorButton)
    mirrorFrame = qt.QFrame(self.parent)
    self.layout.addWidget(mirrorFrame)
    mirrorFormLayout = qt.QFormLayout(mirrorFrame)

    mirrorXCheckBox = qt.QCheckBox("X-axis")
    mirrorXCheckBox.setToolTip("Flip model along its X axis")
    mirrorFormLayout.addWidget(mirrorXCheckBox)

    mirrorYCheckBox = qt.QCheckBox("Y-axis")
    mirrorYCheckBox.setToolTip("Flip model along its Y axis")
    mirrorFormLayout.addWidget(mirrorYCheckBox)

    mirrorZCheckBox = qt.QCheckBox("Z-axis")
    mirrorZCheckBox.setToolTip("Flip model along its Z axis")
    mirrorFormLayout.addWidget(mirrorZCheckBox)

    cleanerButton = qt.QPushButton("Cleaner")
    cleanerButton.checkable = True
    self.layout.addWidget(cleanerButton)

    fillHolesButton = qt.QPushButton("Fill holes")
    fillHolesButton.checkable = True
    self.layout.addWidget(fillHolesButton)

    fillHolesFrame = qt.QFrame(self.parent)
    self.layout.addWidget(fillHolesFrame)
    fillHolesFormLayout = qt.QFormLayout(fillHolesFrame)

    fillHolesSizeFrame, fillHolesSizeSlider, fillHolesSizeSpinBox = numericInputFrame(self.parent,"Maximum hole size:",
      "Specifies the maximum size of holes that will be filled. This is represented as a radius to the bounding"
      +" circumsphere containing the hole. Note that this is an approximate area; the actual area cannot be"
      +" computed without first triangulating the hole. "
      , 0.0, 1000, 0.1, 1)
    fillHolesFormLayout.addWidget(fillHolesSizeFrame)

    connectivityButton = qt.QPushButton("Connectivity")
    connectivityButton.checkable = True
    self.layout.addWidget(connectivityButton)
    #connectivityFrame = qt.QFrame(self.parent)
    #self.layout.addWidget(connectivityFrame)
    #connectivityFormLayout = qt.QFormLayout(connectivityFrame)

    # TODO: connectivity could be
    # - largest connected
    # - threshold connected (discard below point count)
    # - pick a region interactively
    # - turn a multiple connected surface into a model hierarchy

    buttonFrame = qt.QFrame(self.parent)
    buttonFrame.setLayout(qt.QHBoxLayout())
    self.layout.addWidget(buttonFrame)

    toggleModelsButton = qt.QPushButton("Toggle Models")
    toggleModelsButton.toolTip = "Show original model."
    buttonFrame.layout().addWidget(toggleModelsButton)

    applyButton = qt.QPushButton("Apply")
    applyButton.toolTip = "Filter surface."
    buttonFrame.layout().addWidget(applyButton)

    self.layout.addStretch(1)

    class state(object):
      inputModelNode = None
      outputModelNode = None
      decimation = False
      reduction = 0.8
      boundaryDeletion = False
      smoothing = False
      smoothingMethod = "Laplace"
      laplaceIterations = 100.0
      laplaceRelaxation = 0.5
      taubinIterations = 30.0
      taubinPassBand = 0.1
      boundarySmoothing = True
      normals = False
      flipNormals = False
      autoOrientNormals = False
      mirror = False
      mirrorX = False
      mirrorY = False
      mirrorZ = False
      splitting = False
      featureAngle = 30.0
      cleaner = False
      fillHoles = False
      fillHolesSize = 1000.0
      connectivity = False

    scope_locals = locals()
    def connect(obj, evt, cmd):
      def callback(*args):
        current_locals = scope_locals.copy()
        current_locals.update({'args':args})
        exec cmd in globals(), current_locals
        updateGUI()
      obj.connect(evt,callback)

    def updateGUI():

      def button_stylesheet(active):
        if active:
          return "background-color: green"
        else:
          return ""

      decimationButton.checked = state.decimation
      #decimationButton.setStyleSheet(button_stylesheet(state.decimation))
      decimationFrame.visible = state.decimation
      boundaryDeletionCheckBox.checked = state.boundaryDeletion
      reductionSlider.value = state.reduction
      reductionSpinBox.value = state.reduction

      smoothingButton.checked = state.smoothing
      smoothingFrame.visible = state.smoothing
      laplaceMethodFrame.visible = state.smoothingMethod == "Laplace"
      laplaceIterationsSlider.value = state.laplaceIterations
      laplaceIterationsSpinBox.value = state.laplaceIterations
      laplaceRelaxationSlider.value = state.laplaceRelaxation
      laplaceRelaxationSpinBox.value = state.laplaceRelaxation
      taubinMethodFrame.visible = state.smoothingMethod == "Taubin"
      taubinIterationsSlider.value = state.taubinIterations
      taubinIterationsSpinBox.value = state.taubinIterations
      taubinPassBandSlider.value = state.taubinPassBand
      taubinPassBandSpinBox.value = state.taubinPassBand
      boundarySmoothingCheckBox.checked = state.boundarySmoothing

      normalsButton.checked = state.normals
      normalsFrame.visible = state.normals
      autoOrientNormalsCheckBox.checked = state.autoOrientNormals
      flipNormalsCheckBox.checked = state.flipNormals
      splittingCheckBox.checked = state.splitting
      featureAngleFrame.visible = state.splitting
      featureAngleSlider.value = state.featureAngle
      featureAngleSpinBox.value = state.featureAngle
      featureAngleFrame.visible = state.splitting

      mirrorButton.checked = state.mirror
      mirrorFrame.visible = state.mirror
      mirrorXCheckBox.checked = state.mirrorX
      mirrorYCheckBox.checked = state.mirrorY
      mirrorZCheckBox.checked = state.mirrorZ

      cleanerButton.checked = state.cleaner

      fillHolesButton.checked = state.fillHoles
      fillHolesFrame.visible = state.fillHoles
      fillHolesSizeSlider.value = state.fillHolesSize
      fillHolesSizeSpinBox.value = state.fillHolesSize

      connectivityButton.checked = state.connectivity

      toggleModelsButton.enabled = state.inputModelNode is not None and state.outputModelNode is not None
      applyButton.enabled = state.inputModelNode is not None and state.outputModelNode is not None


    connect(inputModelSelector,'currentNodeChanged(vtkMRMLNode*)','state.inputModelNode = args[0]')
    connect(outputModelSelector,'currentNodeChanged(vtkMRMLNode*)','state.outputModelNode = args[0]')

    def initializeModelNode(node):
      displayNode = slicer.vtkMRMLModelDisplayNode()
      storageNode = slicer.vtkMRMLModelStorageNode()
      displayNode.SetScene(slicer.mrmlScene)
      storageNode.SetScene(slicer.mrmlScene)
      slicer.mrmlScene.AddNode(displayNode)
      slicer.mrmlScene.AddNode(storageNode)
      node.SetAndObserveDisplayNodeID(displayNode.GetID())
      node.SetAndObserveStorageNodeID(storageNode.GetID())

    outputModelSelector.connect('nodeAddedByUser(vtkMRMLNode*)',initializeModelNode)

    connect(decimationButton, 'clicked(bool)', 'state.decimation = args[0]')
    connect(reductionSlider, 'valueChanged(double)', 'state.reduction = args[0]')
    connect(reductionSpinBox, 'valueChanged(double)', 'state.reduction = args[0]')
    connect(boundaryDeletionCheckBox, 'stateChanged(int)', 'state.boundaryDeletion = bool(args[0])')

    connect(smoothingButton, 'clicked(bool)', 'state.smoothing = args[0]')
    connect(smoothingMethodCombo, 'currentIndexChanged(QString)', 'state.smoothingMethod = args[0]')

    connect(laplaceIterationsSlider, 'valueChanged(double)', 'state.laplaceIterations = int(args[0])')
    connect(laplaceIterationsSpinBox, 'valueChanged(double)', 'state.laplaceIterations = int(args[0])')
    connect(laplaceRelaxationSlider, 'valueChanged(double)', 'state.laplaceRelaxation = args[0]')
    connect(laplaceRelaxationSpinBox, 'valueChanged(double)', 'state.laplaceRelaxation = args[0]')

    connect(taubinIterationsSlider, 'valueChanged(double)', 'state.taubinIterations = int(args[0])')
    connect(taubinIterationsSpinBox, 'valueChanged(double)', 'state.taubinIterations = int(args[0])')
    connect(taubinPassBandSlider, 'valueChanged(double)', 'state.taubinPassBand = args[0]')
    connect(taubinPassBandSpinBox, 'valueChanged(double)', 'state.taubinPassBand = args[0]')

    connect(boundarySmoothingCheckBox, 'stateChanged(int)', 'state.boundarySmoothing = bool(args[0])')

    connect(normalsButton, 'clicked(bool)', 'state.normals = args[0]')
    connect(autoOrientNormalsCheckBox, 'toggled(bool)', 'state.autoOrientNormals = bool(args[0])')
    connect(flipNormalsCheckBox, 'toggled(bool)', 'state.flipNormals = bool(args[0])')
    connect(splittingCheckBox, 'stateChanged(int)', 'state.splitting = bool(args[0])')
    connect(featureAngleSlider, 'valueChanged(double)', 'state.featureAngle = args[0]')
    connect(featureAngleSpinBox, 'valueChanged(double)', 'state.featureAngle = args[0]')

    connect(mirrorButton, 'clicked(bool)', 'state.mirror = args[0]')
    connect(mirrorXCheckBox, 'toggled(bool)', 'state.mirrorX = bool(args[0])')
    connect(mirrorYCheckBox, 'toggled(bool)', 'state.mirrorY = bool(args[0])')
    connect(mirrorZCheckBox, 'toggled(bool)', 'state.mirrorZ = bool(args[0])')

    connect(cleanerButton, 'clicked(bool)', 'state.cleaner = args[0]')

    connect(fillHolesButton, 'clicked(bool)', 'state.fillHoles = args[0]')
    connect(fillHolesSizeSlider, 'valueChanged(double)', 'state.fillHolesSize = args[0]')
    connect(fillHolesSizeSpinBox, 'valueChanged(double)', 'state.fillHolesSize = args[0]')

    connect(connectivityButton, 'clicked(bool)', 'state.connectivity = args[0]')

    def onApply():
      updateGUI()
      applyButton.text = "Working..."
      applyButton.repaint()
      slicer.app.processEvents()
      logic = SurfaceToolboxLogic()
      result = logic.applyFilters(state)
      if result:
        state.inputModelNode.GetModelDisplayNode().VisibilityOff()
        state.outputModelNode.GetModelDisplayNode().VisibilityOn()
      else:
        state.inputModelNode.GetModelDisplayNode().VisibilityOn()
        state.outputModelNode.GetModelDisplayNode().VisibilityOff()
      applyButton.text = "Apply"

    applyButton.connect('clicked()', onApply)

    def onToggleModels():
      updateGUI()
      if state.inputModelNode.GetModelDisplayNode().GetVisibility():
        state.inputModelNode.GetModelDisplayNode().VisibilityOff()
        state.outputModelNode.GetModelDisplayNode().VisibilityOn()
        toggleModelsButton.text = "Toggle Models (Output)"
      else:
        state.inputModelNode.GetModelDisplayNode().VisibilityOn()
        state.outputModelNode.GetModelDisplayNode().VisibilityOff()
        toggleModelsButton.text = "Toggle Models (Input)"

    toggleModelsButton.connect('clicked()', onToggleModels)

    updateGUI()

    self.updateGUI = updateGUI


class SurfaceToolboxLogic(ScriptedLoadableModuleLogic):
  """Perform filtering
  """

  def applyFilters(self, state):

    surface = None
    surface = state.inputModelNode.GetPolyDataConnection()

    if state.decimation:
      triangle = vtk.vtkTriangleFilter()
      triangle.SetInputConnection(surface)
      decimation = vtk.vtkDecimatePro()
      decimation.SetTargetReduction(state.reduction)
      decimation.SetBoundaryVertexDeletion(state.boundaryDeletion)
      decimation.PreserveTopologyOn()
      decimation.SetInputConnection(triangle.GetOutputPort())
      surface = decimation.GetOutputPort()

    if state.smoothing:
      if state.smoothingMethod == "Laplace":
        smoothing = vtk.vtkSmoothPolyDataFilter()
        smoothing.SetBoundarySmoothing(state.boundarySmoothing)
        smoothing.SetNumberOfIterations(state.laplaceIterations)
        smoothing.SetRelaxationFactor(state.laplaceRelaxation)
        smoothing.SetInputConnection(surface)
        surface = smoothing.GetOutputPort()
      elif state.smoothingMethod == "Taubin":
        smoothing = vtk.vtkWindowedSincPolyDataFilter()
        smoothing.SetBoundarySmoothing(state.boundarySmoothing)
        smoothing.SetNumberOfIterations(state.taubinIterations)
        smoothing.SetPassBand(state.taubinPassBand)
        smoothing.SetInputConnection(surface)
        surface = smoothing.GetOutputPort()

    if state.normals:
      normals = vtk.vtkPolyDataNormals()
      normals.SetAutoOrientNormals(state.autoOrientNormals)
      normals.SetFlipNormals(state.flipNormals)
      normals.SetSplitting(state.splitting)
      normals.SetFeatureAngle(state.featureAngle)
      normals.ConsistencyOn()
      normals.SetInputConnection(surface)
      surface = normals.GetOutputPort()

    if state.mirror:
      mirrorTransformMatrix = vtk.vtkMatrix4x4()
      mirrorTransformMatrix.SetElement(0, 0, -1 if state.mirrorX else 1)
      mirrorTransformMatrix.SetElement(1, 1, -1 if state.mirrorY else 1)
      mirrorTransformMatrix.SetElement(2, 2, -1 if state.mirrorZ else 1)
      mirrorTransform = vtk.vtkTransform()
      mirrorTransform.SetMatrix(mirrorTransformMatrix)
      transformFilter = vtk.vtkTransformPolyDataFilter()
      transformFilter.SetInputConnection(surface)
      transformFilter.SetTransform(mirrorTransform)
      surface = transformFilter.GetOutputPort()
      if mirrorTransformMatrix.Determinant()<0:
        reverse = vtk.vtkReverseSense()
        reverse.SetInputConnection(surface)
        surface = reverse.GetOutputPort()

    if state.cleaner:
      cleaner = vtk.vtkCleanPolyData()
      cleaner.SetInputConnection(surface)
      surface = cleaner.GetOutputPort()

    if state.fillHoles:
      fillHoles = vtk.vtkFillHolesFilter()
      fillHoles.SetHoleSize(state.fillHolesSize)
      fillHoles.SetInputConnection(surface)
      # Need to auto-orient normals, otherwise holes
      # could appear to be unfilled when only front-facing elements
      # are chosen to be visible.
      normals = vtk.vtkPolyDataNormals()
      normals.AutoOrientNormalsOn()
      normals.ConsistencyOn()
      normals.SetInputConnection(fillHoles.GetOutputPort())
      surface = normals.GetOutputPort()

    if state.connectivity:
      connectivity = vtk.vtkPolyDataConnectivityFilter()
      connectivity.SetExtractionModeToLargestRegion()
      connectivity.SetInputConnection(surface)
      surface = connectivity.GetOutputPort()

    state.outputModelNode.SetPolyDataConnection(surface)
    return True



class SurfaceToolboxTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SurfaceToolbox1()

  def test_SurfaceToolbox1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests should exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = SurfaceToolboxLogic()
    self.assertIsNotNone( logic )
    self.delayDisplay('Test passed!')
