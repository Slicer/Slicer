from __future__ import print_function
import os
import vtk
import vtkITK
import ctk
import qt
import slicer

from . import EditUtil
from . import HelpButton
from . import IslandEffectOptions, IslandEffectTool, IslandEffectLogic, IslandEffect

__all__ = [
  'RemoveIslandsEffectOptions',
  'RemoveIslandsEffectTool',
  'RemoveIslandsEffectLogic',
  'RemoveIslandsEffect'
  ]

#########################################################
#
#
comment = """

  RemoveIslandsEffect is a subclass of IslandEffect
  to remove small islands that might, for example, be
  cause by noise after thresholding.

# TODO :
"""
#
#########################################################

#
# RemoveIslandsEffectOptions - see Effect for superclasses
#

class RemoveIslandsEffectOptions(IslandEffectOptions):
  """ RemoveIslandsEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(RemoveIslandsEffectOptions,self).__init__(parent)
    # create a logic instance to do the non-gui work
    # (since this is created from the option gui it has no slice logic)
    self.logic = RemoveIslandsEffectLogic(None)

  def __del__(self):
    super(RemoveIslandsEffectOptions,self).__del__()

  def create(self):
    super(RemoveIslandsEffectOptions,self).create()

    self.applyConnectivity = qt.QPushButton("Apply Connectivity Method", self.frame)
    self.applyConnectivity.setToolTip("Remove islands that are not connected at all to the surface.  Islands with thin connections will not be removed.")
    self.frame.layout().addWidget(self.applyConnectivity)
    self.widgets.append(self.applyConnectivity)

    self.applyMorphology = qt.QPushButton("Apply Morphology Method", self.frame)
    self.applyMorphology.setToolTip("Remove islands by erosion and dilation so that small islands are removed even if they are connected to the outside or if they exist on the outside of the larger segmented regions.")
    self.frame.layout().addWidget(self.applyMorphology)
    self.widgets.append(self.applyMorphology)

    self.connections.append( (self.applyConnectivity, 'clicked()', self.onApplyConnectivity) )
    self.connections.append( (self.applyMorphology, 'clicked()', self.onApplyMorphology) )

    HelpButton(self.frame, "Remove connected regions (islands) that are fully enclosed by the current label color and are smaller than the given minimum size.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def onApplyConnectivity(self):
    self.logic.undoRedo = self.undoRedo
    self.logic.removeIslandsConnectivity()

  def onApplyMorphology(self):
    self.logic.undoRedo = self.undoRedo
    self.logic.removeIslandsMorphology()

  def destroy(self):
    super(RemoveIslandsEffectOptions,self).destroy()

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
    super(RemoveIslandsEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(RemoveIslandsEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    super(RemoveIslandsEffectOptions,self).updateMRMLFromGUI()

#
# RemoveIslandsEffectTool
#

class RemoveIslandsEffectTool(IslandEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(RemoveIslandsEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    """
    call superclass to clean up actors
    """
    super(RemoveIslandsEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    super(RemoveIslandsEffectTool,self).processEvent()

#
# RemoveIslandsEffectLogic
#

class RemoveIslandsEffectLogic(IslandEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an RemoveIslandsEffectTool
  or RemoveIslandsEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the RemoveIslandsEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(RemoveIslandsEffectLogic,self).__init__(sliceLogic)


  def findNonZeroBorderPixel(self, imageData):
    """ search the border of the image data looking for the first
    - usually whole border will be nonzero, but in some cases
      it may not be.  So check corners first, and then
      if can't find it, give up and use 1 (to avoid exhaustive search)
    """
    w,h,d = [x-1 for x in imageData.GetDimensions()]

    corners = [ [0, 0, 0], [w, 0, 0], [0, h, 0], [w, h, 0],
                  [0, 0, d], [w, 0, d], [0, h, d], [w, h, d] ]

    for corner in corners:
        p = imageData.GetScalarComponentAsDouble(*(corner+[0,]))
        if p != 0:
          return p
    return 1

  def removeIslandsConnectivity(self):
    #
    # change the label values based on the parameter node
    #
    if not self.sliceLogic:
      self.sliceLogic = EditUtil.getSliceLogic()
    parameterNode = EditUtil.getParameterNode()
    minimumSize = int(parameterNode.GetParameter("IslandEffect,minimumSize"))
    fullyConnected = bool(parameterNode.GetParameter("IslandEffect,fullyConnected"))
    label = EditUtil.getLabel()

    # first, create an inverse binary version of the image
    # so that islands inside segemented object will be detected, along
    # with a big island of the background
    preThresh = vtk.vtkImageThreshold()
    preThresh.SetInValue( 0 )
    preThresh.SetOutValue( 1 )
    preThresh.ReplaceInOn()
    preThresh.ReplaceOutOn()
    preThresh.ThresholdBetween( label,label )
    preThresh.SetInputData( self.getScopedLabelInput() )
    preThresh.SetOutputScalarTypeToUnsignedLong()

    # now identify the islands in the inverted volume
    # and find the pixel that corresponds to the background
    islandMath = vtkITK.vtkITKIslandMath()
    islandMath.SetInputConnection( preThresh.GetOutputPort() )
    islandMath.SetFullyConnected( fullyConnected )
    islandMath.SetMinimumSize( minimumSize )
    # TODO: $this setProgressFilter $islandMath "Calculating Islands..."
    islandMath.Update()
    islandCount = islandMath.GetNumberOfIslands()
    islandOrigCount = islandMath.GetOriginalNumberOfIslands()
    ignoredIslands = islandOrigCount - islandCount
    print( "%d islands created (%d ignored)" % (islandCount, ignoredIslands) )
    if islandCount == 0:
      return

    bgPixel = self.findNonZeroBorderPixel(islandMath.GetOutput())

    # now rethreshold so that everything which is not background becomes the label
    postThresh = vtk.vtkImageThreshold()
    postThresh.SetInValue( 0 )
    postThresh.SetOutValue( label )
    postThresh.ReplaceInOn()
    postThresh.ReplaceOutOn()
    postThresh.ThresholdBetween( bgPixel, bgPixel )
    postThresh.SetOutputScalarTypeToShort()
    postThresh.SetInputConnection( islandMath.GetOutputPort() )
    postThresh.SetOutput( self.getScopedLabelOutput() )
    # TODO $this setProgressFilter $postThresh "Applying to Label Map..."
    postThresh.Update()

    self.applyScopedLabel()
    postThresh.SetOutput( None )


    if False:
      # some code for debugging - leave it in
      layerLogic = self.sliceLogic.GetLabelLayer()
      labelNode = layerLogic.GetVolumeNode()
      volumesLogic = slicer.modules.volumes.logic()
      thresh1 = volumesLogic.CloneVolume(slicer.mrmlScene, labelNode, 'thres1')
      islands = volumesLogic.CloneVolume(slicer.mrmlScene, labelNode, 'islands')
      thresh2 = volumesLogic.CloneVolume(slicer.mrmlScene, labelNode, 'thres2')
      cast = vtk.vtkImageCast()
      cast.SetOutputScalarTypeToShort()
      cast.SetInputConnection(preThresh.GetOutputPort())
      thresh1.SetImageDataConnection(cast.GetOutputPort())
      cast2 = vtk.vtkImageCast()
      cast2.SetOutputScalarTypeToShort()
      cast2.SetInputConnection(islandMath.GetOutputPort())
      islands.SetImageDataConnection(cast2.GetOutputPort())
      thresh2.SetImageDataConnection(postThresh.GetOutputPort())

  def removeIslandsMorphology(self):
    """
    Remove cruft from image by eroding away by iterations number of layers of surface
    pixels and then saving only islands that are bigger than the minimumSize.
    Then dilate back and save only the pixels that are in both the original and
    result image.  Result is that small islands outside the foreground and small features
    on the foreground are removed.

    By calling the decrufter twice with fg and bg reversed, you can clean up small features in
    a label map while preserving the original boundary in other places.
    """
    if not self.sliceLogic:
      self.sliceLogic = EditUtil.getSliceLogic()
    parameterNode = EditUtil.getParameterNode()
    self.minimumSize = int(parameterNode.GetParameter("IslandEffect,minimumSize"))
    self.fullyConnected = bool(parameterNode.GetParameter("IslandEffect,fullyConnected"))

    labelImage = vtk.vtkImageData()
    labelImage.DeepCopy( self.getScopedLabelInput() )
    label = EditUtil.getLabel()

    self.undoRedo.saveState()

    self.removeIslandsMorphologyDecruft(labelImage,0,label)
    self.getScopedLabelOutput().DeepCopy(labelImage)
    self.applyScopedLabel()
    slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)

    self.removeIslandsMorphologyDecruft(labelImage,label,0)
    self.getScopedLabelOutput().DeepCopy(labelImage)
    self.applyScopedLabel()

  def removeIslandsMorphologyDecruft(self,image,foregroundLabel,backgroundLabel,iterations=1):
    #
    # make binary mask foregroundLabel->1, backgroundLabel->0
    #
    binThresh = vtk.vtkImageThreshold()
    binThresh.SetInputData( image )
    binThresh.ThresholdBetween(foregroundLabel,foregroundLabel)
    binThresh.SetInValue( 1 )
    binThresh.SetOutValue( 0 )
    binThresh.Update()

    #
    # first, erode iterations number of times
    #
    eroder = slicer.vtkImageErode()
    eroderImage = vtk.vtkImageData()
    eroderImage.DeepCopy(binThresh.GetOutput())
    eroder.SetInputData(eroderImage)
    for iteration in range(iterations):
      eroder.SetForeground( 1 )
      eroder.SetBackground( 0 )
      eroder.SetNeighborTo8()
      eroder.Update()
      eroderImage.DeepCopy(eroder.GetOutput())


    #
    # now save only islands bigger than a specified size
    #

    # note that island operation happens in unsigned long space
    # but the slicer editor works in Short
    castIn = vtk.vtkImageCast()
    castIn.SetInputConnection( eroder.GetInputConnection(0,0) )
    castIn.SetOutputScalarTypeToUnsignedLong()

    # now identify the islands in the inverted volume
    # and find the pixel that corresponds to the background
    islandMath = vtkITK.vtkITKIslandMath()
    islandMath.SetInputConnection( castIn.GetOutputPort() )
    islandMath.SetFullyConnected( self.fullyConnected )
    islandMath.SetMinimumSize( self.minimumSize )

    # note that island operation happens in unsigned long space
    # but the slicer editor works in Short
    castOut = vtk.vtkImageCast()
    castOut.SetInputConnection( islandMath.GetOutputPort() )
    castOut.SetOutputScalarTypeToShort()

    castOut.Update()
    islandCount = islandMath.GetNumberOfIslands()
    islandOrigCount = islandMath.GetOriginalNumberOfIslands()
    ignoredIslands = islandOrigCount - islandCount
    print( "%d islands created (%d ignored)" % (islandCount, ignoredIslands) )

    #
    # now map everything back to 0 and 1
    #

    thresh = vtk.vtkImageThreshold()
    thresh.SetInputConnection( castOut.GetOutputPort() )
    thresh.ThresholdByUpper(1)
    thresh.SetInValue( 1 )
    thresh.SetOutValue( 0 )
    thresh.Update()

    #
    # now, dilate back (erode background) iterations_plus_one number of times
    #
    dilater = slicer.vtkImageErode()
    dilaterImage = vtk.vtkImageData()
    dilaterImage.DeepCopy(thresh.GetOutput())
    dilater.SetInputData(dilaterImage)
    for iteration in range(1+iterations):
      dilater.SetForeground( 0 )
      dilater.SetBackground( 1 )
      dilater.SetNeighborTo8()
      dilater.Update()
      dilaterImage.DeepCopy(dilater.GetOutput())

    #
    # only keep pixels in both original and dilated result
    #

    logic = vtk.vtkImageLogic()
    logic.SetInputConnection(0, dilater.GetInputConnection(0,0))
    logic.SetInputConnection(1, binThresh.GetOutputPort())
    #if foregroundLabel == 0:
    #  logic.SetOperationToNand()
    #else:
    logic.SetOperationToAnd()
    logic.SetOutputTrueValue(1)
    logic.Update()

    #
    # convert from binary mask to 1->foregroundLabel, 0->backgroundLabel
    #
    unbinThresh = vtk.vtkImageThreshold()
    unbinThresh.SetInputConnection( logic.GetOutputPort() )
    unbinThresh.ThresholdBetween( 1,1 )
    unbinThresh.SetInValue( foregroundLabel )
    unbinThresh.SetOutValue( backgroundLabel )
    unbinThresh.Update()

    image.DeepCopy(unbinThresh.GetOutput())



#
# The RemoveIslandsEffect class definition
#

class RemoveIslandsEffect(IslandEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. RemoveIslandsEffect.png)
    self.name = "RemoveIslandsEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "RemoveIslands: remove any enclosed islands larger than minimum size"

    self.options = RemoveIslandsEffectOptions
    self.tool = RemoveIslandsEffectTool
    self.logic = RemoveIslandsEffectLogic
