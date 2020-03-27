import os
import vtk, qt, ctk, slicer, logging

#
# Abstract class of python scripted segment editor effects
#

class AbstractScriptedSegmentEditorEffect(object):
  """ Abstract scripted segment editor effects for effects implemented in python

      USAGE:
      1. Instantiation and registration
        Instantiate segment editor effect adaptor class from
        module (e.g. from setup function), and set python source:
        > import qSlicerSegmentationsEditorEffectsPythonQt as effects
        > scriptedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
        > scriptedEffect.setPythonSource(MyEffect.filePath)
        > scriptedEffect.self().register()
        If effect name is added to slicer.modules.segmenteditorscriptedeffectnames
        list then the above instantiation and registration steps are not necessary,
        as the SegmentEditor module do all these.

      2. Call host C++ implementation using
        > self.scriptedEffect.functionName()

      2.a. Most frequently used such methods are:
        Parameter get/set: parameter, integerParameter, doubleParameter, setParameter
        Add options widget: addOptionsWidget
        Coordinate transforms: rasToXy, xyzToRas, xyToRas, xyzToIjk, xyToIjk
        Convenience getters: renderWindow, renderer, viewNode

      2.b. Always call API functions (the ones that are defined in the adaptor
        class qSlicerSegmentEditorScriptedEffect) using the adaptor accessor:
        > self.scriptedEffect.updateGUIFromMRML()

      3. To prevent deactivation of an effect by clicking place fiducial toolbar button,
         override interactionNodeModified(self, interactionNode)

      An example for a generic effect is the ThresholdEffect

  """

  def __init__(self, scriptedEffect):
    self.scriptedEffect = scriptedEffect

  def register(self):
    import qSlicerSegmentationsEditorEffectsPythonQt
    #TODO: For some reason the instance() function cannot be called as a class function although it's static
    factory = qSlicerSegmentationsEditorEffectsPythonQt.qSlicerSegmentEditorEffectFactory()
    effectFactorySingleton = factory.instance()
    effectFactorySingleton.registerEffect(self.scriptedEffect)

  #
  # Utility functions for convenient coordinate transformations
  #
  def rasToXy(self, ras, viewWidget):
    rasVector = qt.QVector3D(ras[0], ras[1], ras[2])
    xyPoint = self.scriptedEffect.rasToXy(rasVector, viewWidget)
    return [xyPoint.x(), xyPoint.y()]

  def xyzToRas(self, xyz, viewWidget):
    xyzVector = qt.QVector3D(xyz[0], xyz[1], xyz[2])
    rasVector = self.scriptedEffect.xyzToRas(xyzVector, viewWidget)
    return [rasVector.x(), rasVector.y(), rasVector.z()]

  def xyToRas(self, xy, viewWidget):
    xyPoint = qt.QPoint(xy[0], xy[1])
    rasVector = self.scriptedEffect.xyToRas(xyPoint, viewWidget)
    return [rasVector.x(), rasVector.y(), rasVector.z()]

  def xyzToIjk(self, xyz, viewWidget, image, parentTransformNode=None):
    import vtkSegmentationCorePython as vtkSegmentationCore
    xyzVector = qt.QVector3D(xyz[0], xyz[1], xyz[2])
    ijkVector = self.scriptedEffect.xyzToIjk(xyzVector, viewWidget, image, parentTransformNode)
    return [int(ijkVector.x()), int(ijkVector.y()), int(ijkVector.z())]

  def xyToIjk(self, xy, viewWidget, image, parentTransformNode=None):
    import vtkSegmentationCorePython as vtkSegmentationCore
    xyPoint = qt.QPoint(xy[0], xy[1])
    ijkVector = self.scriptedEffect.xyToIjk(xyPoint, viewWidget, image, parentTransformNode)
    return [int(ijkVector.x()), int(ijkVector.y()), int(ijkVector.z())]

  def setWidgetMinMaxStepFromImageSpacing(self, spinbox, imageData):
    # Set spinbox minimum, maximum, and step size from vtkImageData spacing:
    # Set widget minimum spacing and step size to be 1/10th or less than minimum spacing
    # Set widget minimum spacing to be 100x or more than minimum spacing
    if not imageData:
      return
    import math
    spinbox.unitAwareProperties &= ~(slicer.qMRMLSpinBox.MinimumValue | slicer.qMRMLSpinBox.MaximumValue | slicer.qMRMLSpinBox.Precision)
    stepSize = 10**(math.floor(math.log10(min(imageData.GetSpacing())/10.0)))
    spinbox.minimum = stepSize
    spinbox.maximum = 10**(math.ceil(math.log10(max(imageData.GetSpacing())*100.0)))
    spinbox.singleStep = stepSize
    # number of decimals is set to be able to show the step size (e.g., stepSize = 0.01 => decimals = 2)
    spinbox.decimals = max(int(-math.floor(math.log10(stepSize))),0)
