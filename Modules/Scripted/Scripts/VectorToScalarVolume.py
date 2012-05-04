from __main__ import vtk, qt, ctk, slicer

#
# VectorToScalarVolume
#

class VectorToScalarVolume:
  def __init__(self, parent):
    parent.title = "Vector to Scalar Volume"
    parent.categories = ["Converters"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)",]
    parent.helpText = """
    Make a scalar (1 component) volume from a vector volume
    """
    parent.acknowledgementText = """
Developed by Steve Pieper, Isomics, Inc., 
partially funded by NIH grant 3P41RR013218-12S1 (NAC) and is part of the National Alliance 
for Medical Image Computing (NA-MIC), funded by the National Institutes of Health through the 
NIH Roadmap for Medical Research, Grant U54 EB005149."""
    self.parent = parent

#
# VectorToScalarVolumeWidget
#

class VectorToScalarVolumeWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Collapsible button
    self.selectionCollapsibleButton = ctk.ctkCollapsibleButton()
    self.selectionCollapsibleButton.text = "Selection"
    self.layout.addWidget(self.selectionCollapsibleButton)

    # Layout within the collapsible button
    self.formLayout = qt.QFormLayout(self.selectionCollapsibleButton)

    #
    # the volume selectors
    #
    self.inputFrame = qt.QFrame(self.selectionCollapsibleButton)
    self.inputFrame.setLayout(qt.QHBoxLayout())
    self.formLayout.addWidget(self.inputFrame)
    self.inputSelector = qt.QLabel("Input Vector Volume: ", self.inputFrame)
    self.inputFrame.layout().addWidget(self.inputSelector)
    self.inputSelector = slicer.qMRMLNodeComboBox(self.inputFrame)
    self.inputSelector.nodeTypes = ( ("vtkMRMLVectorVolumeNode"), "" )
    self.inputSelector.addEnabled = False
    self.inputSelector.removeEnabled = False
    self.inputSelector.setMRMLScene( slicer.mrmlScene )
    self.inputFrame.layout().addWidget(self.inputSelector)

    self.outputFrame = qt.QFrame(self.selectionCollapsibleButton)
    self.outputFrame.setLayout(qt.QHBoxLayout())
    self.formLayout.addWidget(self.outputFrame)
    self.outputSelector = qt.QLabel("Output Scalar Volume: ", self.outputFrame)
    self.outputFrame.layout().addWidget(self.outputSelector)
    self.outputSelector = slicer.qMRMLNodeComboBox(self.outputFrame)
    self.outputSelector.nodeTypes = ( ("vtkMRMLScalarVolumeNode"), "" )
    self.outputSelector.setMRMLScene( slicer.mrmlScene )
    self.outputFrame.layout().addWidget(self.outputSelector)

    # Apply button
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run Convert the vector to scalar."
    self.formLayout.addWidget(self.applyButton)
    self.applyButton.connect('clicked(bool)', self.onApply)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onApply(self):
    inputVolume = self.inputSelector.currentNode()
    outputVolume = self.outputSelector.currentNode()
    if not (inputVolume and outputVolume):
      qt.QMessageBox.critical(
          slicer.util.mainWindow(),
          'Luminance', 'Input and output volumes are required for conversion')
      return
    # run the filter
    luminance = vtk.vtkImageLuminance()
    luminance.SetInput(inputVolume.GetImageData())
    luminance.GetOutput().Update()
    ijkToRAS = vtk.vtkMatrix4x4()
    inputVolume.GetIJKToRASMatrix(ijkToRAS)
    outputVolume.SetIJKToRASMatrix(ijkToRAS)
    outputVolume.SetAndObserveImageData(luminance.GetOutput())

    # make the output volume appear in all the slice views
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(outputVolume.GetID())
    slicer.app.applicationLogic().PropagateVolumeSelection(0)
