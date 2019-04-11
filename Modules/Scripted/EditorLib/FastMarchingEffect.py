from __future__ import print_function
import os
import vtk, qt, ctk, slicer
from . import HelpButton
from . import EditUtil
from . import EffectOptions, EffectTool, EffectLogic, Effect

__all__ = [
  'FastMarchingEffectOptions',
  'FastMarchingEffectTool',
  'FastMarchingEffectLogic',
  'FastMarchingEffect'
  ]

#
# The Editor Extension itself.
#
# This needs to define the hooks to become an editor effect.
#

#
# FastMarchingEffectOptions - see EditOptions and Effect for superclasses
#

class FastMarchingEffectOptions(EffectOptions):
  """ FastMarchingEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(FastMarchingEffectOptions,self).__init__(parent)

    # self.attributes should be tuple of options:
    # 'MouseTool' - grabs the cursor
    # 'Nonmodal' - can be applied while another is active
    # 'Disabled' - not available
    # self.attributes = ('MouseTool')
    self.displayName = 'FastMarchingEffect Effect'

    self.logic = FastMarchingEffectLogic(EditUtil.getSliceLogic())

  def __del__(self):
    super(FastMarchingEffectOptions,self).__del__()

  def create(self):
    super(FastMarchingEffectOptions,self).create()

    self.defaultMaxPercent = 30

    self.percentLabel = qt.QLabel('Expected structure volume as % of image volume:',self.frame)
    self.percentLabel.setToolTip('Segmentation will grow from the seed label until this value is reached')
    self.frame.layout().addWidget(self.percentLabel)
    self.widgets.append(self.percentLabel)

    self.percentMax = ctk.ctkSliderWidget(self.frame)
    self.percentMax.minimum = 0
    self.percentMax.maximum = 100
    self.percentMax.singleStep = 1
    self.percentMax.value = self.defaultMaxPercent
    self.percentMax.setToolTip('Approximate volume of the structure to be segmented relative to the total volume of the image')
    self.frame.layout().addWidget(self.percentMax)
    self.widgets.append(self.percentMax)
    self.percentMax.connect('valueChanged(double)', self.percentMaxChanged)

    self.march = qt.QPushButton("March", self.frame)
    self.march.setToolTip("Perform the Marching operation into the current label map")
    self.frame.layout().addWidget(self.march)
    self.widgets.append(self.march)

    self.percentVolume = qt.QLabel('Maximum volume of the structure: ')
    self.percentVolume.setToolTip('Total maximum volume')
    self.frame.layout().addWidget(self.percentVolume)
    self.widgets.append(self.percentVolume)

    self.marcher = ctk.ctkSliderWidget(self.frame)
    self.marcher.minimum = 0
    self.marcher.maximum = 1
    self.marcher.singleStep = 0.01
    self.marcher.enabled = False
    self.frame.layout().addWidget(self.marcher)
    self.widgets.append(self.marcher)
    self.marcher.connect('valueChanged(double)',self.onMarcherChanged)

    HelpButton(self.frame, "To use FastMarching effect, first mark the areas that belong to the structure of interest to initialize the algorithm. Define the expected volume of the structure you are trying to segment, and hit March.\nAfter computation is complete, use the Marcher slider to go over the segmentation history.")

    self.march.connect('clicked()', self.onMarch)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

    self.percentMaxChanged(self.percentMax.value)

  def destroy(self):
    super(FastMarchingEffectOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type
  def updateParameterNode(self, caller, event):
    node = EditUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(FastMarchingEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(FastMarchingEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    # TODO: get the march parameter from the node
    # march = float(self.parameterNode.GetParameter
    self.connectWidgets()

  def onMarch(self):
    try:
      slicer.util.showStatusMessage('Running FastMarching...', 2000)
      self.logic.undoRedo = self.undoRedo
      npoints = self.logic.fastMarching(self.percentMax.value)
      slicer.util.showStatusMessage('FastMarching finished', 2000)
      if npoints:
        self.marcher.minimum = 0
        self.marcher.maximum = npoints
        self.marcher.value = npoints
        self.marcher.singleStep = 1
        self.marcher.enabled = True
    except IndexError:
      print('No tools available!')
      pass

  def onMarcherChanged(self,value):
    self.logic.updateLabel(value/self.marcher.maximum)

  def percentMaxChanged(self, val):
    labelNode = self.logic.getLabelNode()
    labelImage = EditUtil.getLabelImage()
    spacing = labelNode.GetSpacing()
    dim = labelImage.GetDimensions()
    print(dim)
    totalVolume = spacing[0]*dim[0]+spacing[1]*dim[1]+spacing[2]*dim[2]

    percentVolumeStr = "%.5f" % (totalVolume*val/100.)
    self.percentVolume.text = '(maximum total volume: '+percentVolumeStr+' mL)'

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(FastMarchingEffectOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#
# FastMarchingEffectTool
#

class FastMarchingEffectTool(EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(FastMarchingEffectTool,self).__init__(sliceWidget)


  def cleanup(self):
    super(FastMarchingEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """
    return

  def getVolumeNode(self):
    return self.sliceWidget.sliceLogic().GetLabelLayer().GetVolumeNode()
#
# FastMarchingEffectLogic
#

class FastMarchingEffectLogic(EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an FastMarchingEffectTool
  or FastMarchingEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the FastMarchingEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(FastMarchingEffectLogic,self).__init__(sliceLogic)

  def fastMarching(self,percentMax):

    self.fm = None
    # allocate a new filter each time March is hit
    bgImage = EditUtil.getBackgroundImage()
    labelImage = EditUtil.getLabelImage()

    # collect seeds
    dim = bgImage.GetDimensions()
    print(dim)
    # initialize the filter
    self.fm = slicer.vtkPichonFastMarching()
    scalarRange = bgImage.GetScalarRange()
    depth = scalarRange[1]-scalarRange[0]

    # this is more or less arbitrary; large depth values will bring the
    # algorithm to the knees
    scaleValue = 0
    shiftValue = 0

    if depth>300:
      scaleValue = 300./depth
    if scalarRange[0] < 0:
      shiftValue = scalarRange[0]*-1

    if scaleValue or shiftValue:
      rescale = vtk.vtkImageShiftScale()
      rescale.SetInputData(bgImage)
      rescale.SetScale(scaleValue)
      rescale.SetShift(shiftValue)
      rescale.Update()
      bgImage = rescale.GetOutput()
      scalarRange = bgImage.GetScalarRange()
      depth = scalarRange[1]-scalarRange[0]

    print('Input scalar range: '+str(depth))
    self.fm.init(dim[0], dim[1], dim[2], depth, 1, 1, 1)

    caster = vtk.vtkImageCast()
    caster.SetOutputScalarTypeToShort()
    caster.SetInputData(bgImage)
    self.fm.SetInputConnection(caster.GetOutputPort())

    # self.fm.SetOutput(labelImage)

    npoints = int(dim[0]*dim[1]*dim[2]*percentMax/100.)

    self.fm.setNPointsEvolution(npoints)
    print('Setting active label to '+str(EditUtil.getLabel()))
    self.fm.setActiveLabel(EditUtil.getLabel())

    nSeeds = self.fm.addSeedsFromImage(labelImage)
    if nSeeds == 0:
      return 0

    self.fm.Modified()
    self.fm.Update()

    # TODO: need to call show() twice for data to be updated
    self.fm.show(1)
    self.fm.Modified()
    self.fm.Update()

    self.fm.show(1)
    self.fm.Modified()
    self.fm.Update()

    self.undoRedo.saveState()

    EditUtil.getLabelImage().DeepCopy(self.fm.GetOutput())
    EditUtil.markVolumeNodeAsModified(self.sliceLogic.GetLabelLayer().GetVolumeNode())
    # print('FastMarching output image: '+str(output))
    print('FastMarching march update completed')

    return npoints

  def updateLabel(self,value):
    if not self.fm:
      return
    self.fm.show(value)
    self.fm.Modified()
    self.fm.Update()

    EditUtil.getLabelImage().DeepCopy(self.fm.GetOutput())
    EditUtil.getLabelImage().Modified()

    EditUtil.markVolumeNodeAsModified(self.sliceLogic.GetLabelLayer().GetVolumeNode())

  def getLabelNode(self):
    return self.sliceLogic.GetLabelLayer().GetVolumeNode()


#
# The FastMarchingEffectExtension class definition
#

class FastMarchingEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. FastMarchingEffect.png)
    self.name = "FastMarchingEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "FastMarchingEffect - a similarity based 3D region growing"

    self.options = FastMarchingEffectOptions
    self.tool = FastMarchingEffectTool
    self.logic = FastMarchingEffectLogic
