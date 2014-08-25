import os, glob, sys
import numpy as np
from __main__ import qt
from __main__ import vtk
from __main__ import ctk
from __main__ import slicer

import DataProbeLib
import DataProbeUtil

class SliceAnnotations(object):
  """Implement the Qt window showing settings for Slice View Annotations
  """
  def __init__(self):

    self.dataProbeUtil = DataProbeUtil.DataProbeUtil()

    self.sliceViewNames = []
    self.popupGeometry = qt.QRect()
    self.cornerTexts =[]
    # Bottom Left Corner Text
    self.cornerTexts.append({
      '1-Label':{'text':'','category':'A'},
      '2-Foreground':{'text':'','category':'A'},
      '3-Background':{'text':'','category':'A'}
      })
    # Bottom Right Corner Text
    # Not used - orientation figure will be draw there
    self.cornerTexts.append({
      '1-TR':{'text':'','category':'A'},
      '2-TE':{'text':'','category':'A'}
      })
    # Top Left Corner Text
    self.cornerTexts.append({
      '1-PatientName':{'text':'','category':'B'},
      '2-PatientID':{'text':'','category':'A'},
      '3-PatientInfo':{'text':'','category':'B'},
      '4-Bg-StudyDate':{'text':'','category':'B'},
      '5-Fg-StudyDate':{'text':'','category':'B'},
      '6-Bg-StudyTime':{'text':'','category':'C'},
      '7-Bg-StudyTime':{'text':'','category':'C'},
      '8-Bg-SeriesDescription':{'text':'','category':'C'},
      '9-Fg-SeriesDescription':{'text':'','category':'C'}
      })
    # Top Rihgt Corner Text
    self.cornerTexts.append({
      '1-Institution-Name':{'text':'','category':'B'},
      '2-Referring-Phisycian':{'text':'','category':'B'},
      '3-Manufacturer':{'text':'','category':'C'},
      '4-Model':{'text':'','category':'C'},
      '5-Patient-Position':{'text':'','category':'A'},
      '6-TR':{'text':'','category':'A'},
      '7-TE':{'text':'','category':'A'}
      })

    self.layoutManager = slicer.app.layoutManager()
    self.sliceCornerAnnotations = {}

    self.annotationsDisplayAmount = 0

    # If there is no user settings load defaults
    settings = qt.QSettings()
    if settings.contains('DataProbe/sliceViewAnnotations.topLeft'):
      self.topLeftAnnotationDisplay = int(settings.value(
          'DataProbe/sliceViewAnnotations.topLeft'))
    else:
      self.topLeftAnnotationDisplay = 1

    if settings.contains('DataProbe/sliceViewAnnotations.topRight'):
      self.topRightAnnotationDisplay = int(settings.value(
          'DataProbe/sliceViewAnnotations.topRight'))
    else:
      self.topRightAnnotationDisplay = 1

    if settings.contains('DataProbe/sliceViewAnnotations.bottomLeft'):
      self.bottomLeftAnnotationDisplay = int(settings.value(
          'DataProbe/sliceViewAnnotations.bottomLeft'))
    else:
      self.bottomLeftAnnotationDisplay = 1

    if settings.contains('DataProbe/sliceViewAnnotations.show'):
      self.showSliceViewAnnotations= int(settings.value(
          'DataProbe/sliceViewAnnotations.show'))
    else:
      self.showSliceViewAnnotations = 1

    if settings.contains('DataProbe/sliceViewAnnotations.showScalingRuler'):
      self.showScalingRuler= int(settings.value(
          'DataProbe/sliceViewAnnotations.showScalingRuler'))
    else:
      self.showScalingRuler = 1

    if settings.contains('DataProbe/sliceViewAnnotations.showColorScalarBar'):
      self.showColorScalarBar= int(settings.value(
          'DataProbe/sliceViewAnnotations.showColorScalarBar'))
    else:
      self.showColorScalarBar = 0

    if settings.contains('DataProbe/sliceViewAnnotations.fontFamily'):
      self.fontFamily = settings.value('DataProbe/sliceViewAnnotations.fontFamily')
    else:
      self.fontFamily = 'Times'

    if settings.contains('DataProbe/sliceViewAnnotations.fontSize'):
      self.fontSize = int(settings.value('DataProbe/sliceViewAnnotations.fontSize'))
    else:
      self.fontSize = 14
    self.maximumTextLength= 35

    if settings.contains('DataProbe/sliceViewAnnotations.bgDicomAnnotationsPersistence'):
      self.backgroundDicomAnnotationsPersistence = int(settings.value(
          'DataProbe/sliceViewAnnotations.bgDicomAnnotationsPersistence'))
    else:
      self.backgroundDicomAnnotationsPersistence = 0

    self.parameter = 'showSliceViewAnnotations'
    self.parameterNode = self.dataProbeUtil.getParameterNode()
    self.parameterNodeTag = self.parameterNode.AddObserver(
        vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

    self.colorbarSelectedLayer = 'background'
    self.create()

    if self.showSliceViewAnnotations:
      self.updateSliceViewFromGUI()

  def create(self):
    # Instantiate and connect widgets ...

    loader = qt.QUiLoader()
    path = os.path.join(os.path.dirname(__file__), 'Resources', 'UI','settings.ui')
    qfile = qt.QFile(path)
    qfile.open(qt.QFile.ReadOnly)
    self.window = loader.load(qfile)

    self.cornerTextParametersCollapsibleButton = slicer.util.findChildren(self.window,
       'cornerTextParametersCollapsibleButton')[0]
    self.sliceViewAnnotationsCheckBox = slicer.util.findChildren(self.window,
       'sliceViewAnnotationsCheckBox')[0]
    self.sliceViewAnnotationsCheckBox.checked = self.showSliceViewAnnotations

    self.activateCornersGroupBox = slicer.util.findChildren(self.window,
       'activateCornersGroupBox')[0]
    self.topLeftActivationCheckbox = slicer.util.findChildren(self.window,
       'topLeftActivationCheckbox')[0]
    self.topLeftActivationCheckbox.checked = self.topLeftAnnotationDisplay
    self.topRightActivationCheckbox = slicer.util.findChildren(self.window,
       'topRightActivationCheckbox')[0]
    self.topRightActivationCheckbox.checked = self.topRightAnnotationDisplay
    self.bottomLeftActivationCheckbox = slicer.util.findChildren(self.window,
       'bottomLeftActivationCheckbox')[0]
    self.bottomLeftActivationCheckbox.checked = self.bottomLeftAnnotationDisplay

    self.level1RadioButton = slicer.util.findChildren(self.window,
       'level1RadioButton')[0]
    self.level2RadioButton = slicer.util.findChildren(self.window,
       'level2RadioButton')[0]
    self.level3RadioButton = slicer.util.findChildren(self.window,
       'level3RadioButton')[0]

    self.fontPropertiesGroupBox = slicer.util.findChildren(self.window,
       'fontPropertiesGroupBox')[0]
    self.timesFontRadioButton = slicer.util.findChildren(self.window,
       'timesFontRadioButton')[0]
    self.arialFontRadioButton = slicer.util.findChildren(self.window,
       'arialFontRadioButton')[0]
    if self.fontFamily == 'Times':
      self.timesFontRadioButton.checked = True
    else:
      self.arialFontRadioButton.checked = True

    self.fontSizeSpinBox = slicer.util.findChildren(self.window,
       'fontSizeSpinBox')[0]
    self.fontSizeSpinBox.value = self.fontSize

    self.backgroundLayerPersistenceCheckbox = slicer.util.findChildren(self.window,
       'backgroundLayerPersistenceCheckbox')[0]
    self.backgroundLayerPersistenceCheckbox.checked = self.backgroundDicomAnnotationsPersistence

    self.annotationsAmountGroupBox = slicer.util.findChildren(self.window,
       'annotationsAmountGroupBox')[0]
    self.scalingRulerCollapsibleButton = slicer.util.findChildren(self.window,
       'scalingRulerCollapsibleButton')[0]
    self.showScalingRulerCheckBox = slicer.util.findChildren(self.window,
       'showScalingRulerCheckBox')[0]
    self.showScalingRulerCheckBox.checked = self.showScalingRuler

    self.colorScalarBarCollapsibleButton = slicer.util.findChildren(self.window,
       'colorScalarBarCollapsibleButton')[0]
    self.showColorScalarBarCheckBox = slicer.util.findChildren(self.window,
        'showColorScalarBarCheckBox')[0]
    self.showColorScalarBarCheckBox.checked = self.showColorScalarBar
    self.colorBarLayerSelectionGroupBox = slicer.util.findChildren(self.window,
       'colorBarLayerSelectionGroupBox')[0]

    self.backgroundRadioButton = slicer.util.findChildren(self.window,
       'backgroundRadioButton')[0]
    self.backgroundRadioButton.checked = True
    self.foregroundRadioButton = slicer.util.findChildren(self.window,
       'foregroundRadioButton')[0]

    self.colorScalarBarMaxWidthSlider = slicer.util.findChildren(self.window,
       'colorScalarBarMaxWidthSlider')[0]

    restorDefaultsButton = slicer.util.findChildren(self.window,
        'restoreDefaultsButton')[0]

    # connections
    self.sliceViewAnnotationsCheckBox.connect('clicked()', self.onSliceViewAnnotationsCheckbox)
    self.topLeftActivationCheckbox.connect('clicked()', self.onCornerTextsActivationCheckbox)
    self.topRightActivationCheckbox.connect('clicked()', self.onCornerTextsActivationCheckbox)
    self.bottomLeftActivationCheckbox.connect('clicked()', self.onCornerTextsActivationCheckbox)
    self.timesFontRadioButton.connect('clicked()', self.onFontFamilyRadioButton)
    self.arialFontRadioButton.connect('clicked()', self.onFontFamilyRadioButton)
    self.fontSizeSpinBox.connect('valueChanged(int)', self.onFontSizeSpinBox)

    self.level1RadioButton.connect('clicked()', self.updateSliceViewFromGUI)
    self.level2RadioButton.connect('clicked()', self.updateSliceViewFromGUI)
    self.level3RadioButton.connect('clicked()', self.updateSliceViewFromGUI)

    self.backgroundLayerPersistenceCheckbox.connect('clicked()', self.onBackgroundLayerPersistenceCheckbox)

    self.showScalingRulerCheckBox.connect('clicked()', self.onShowScalingRulerCheckbox)

    self.showColorScalarBarCheckBox.connect('clicked()', self.onShowColorScalarBarCheckbox)
    self.backgroundRadioButton.connect('clicked()',self.onLayerSelectionRadioButton)
    self.foregroundRadioButton.connect('clicked()',self.onLayerSelectionRadioButton)
    self.colorScalarBarMaxWidthSlider.connect('valueChanged(double)', self.updateSliceViewFromGUI)

    restorDefaultsButton.connect('clicked()', self.restoreDefaultValues)

  def onSliceViewAnnotationsCheckbox(self):
    if self.sliceViewAnnotationsCheckBox.checked:
      self.showColorScalarBarCheckBox.checked = True
      self.showScalingRulerCheckBox.checked = True
      self.showSliceViewAnnotations = 1
      self.showScalingRuler = 1
      self.showColorScalarBar = 1
    else:
      self.showColorScalarBarCheckBox.checked = False
      self.showScalingRulerCheckBox.checked = False
      self.showSliceViewAnnotations = 0
      self.showScalingRuler = 0
      self.showColorScalarBar = 0
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.show',
        self.showSliceViewAnnotations)
    settings.setValue('DataProbe/sliceViewAnnotations.showScalingRuler',
        self.showScalingRuler)
    settings.setValue('DataProbe/sliceViewAnnotations.showColorScalarBar',
        self.showColorScalarBar)
    self.updateSliceViewFromGUI()

  def onBackgroundLayerPersistenceCheckbox(self):
    if self.backgroundLayerPersistenceCheckbox.checked:
      self.backgroundDicomAnnotationsPersistence = 1
    else:
      self.backgroundDicomAnnotationsPersistence = 0
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.bgDicomAnnotationsPersistence',
        self.backgroundDicomAnnotationsPersistence)
    self.updateSliceViewFromGUI()

  def onShowScalingRulerCheckbox(self):
    if self.showScalingRulerCheckBox.checked:
      self.showScalingRuler = 1
    else:
      self.showScalingRuler = 0
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.showScalingRuler',
        self.showScalingRuler)
    self.updateSliceViewFromGUI()

  def onLayerSelectionRadioButton(self):
    if self.backgroundRadioButton.checked:
      self.colorbarSelectedLayer = 'background'
    else:
      self.colorbarSelectedLayer = 'foreground'
    self.updateSliceViewFromGUI()

  def onShowColorScalarBarCheckbox(self):
    if self.showColorScalarBarCheckBox.checked:
      self.showColorScalarBar = 1
    else:
      self.showColorScalarBar = 0
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.showColorScalarBar',
        self.showColorScalarBar)
    self.updateSliceViewFromGUI()

  def onCornerTextsActivationCheckbox(self):
    if self.topLeftActivationCheckbox.checked:
      self.topLeftAnnotationDisplay = 1
    else:
      self.topLeftAnnotationDisplay = 0

    if self.topRightActivationCheckbox.checked:
      self.topRightAnnotationDisplay = 1
    else:
      self.topRightAnnotationDisplay = 0

    if self.bottomLeftActivationCheckbox.checked:
      self.bottomLeftAnnotationDisplay = 1
    else:
      self.bottomLeftAnnotationDisplay = 0
    self.updateSliceViewFromGUI()
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.topLeft',
        self.topLeftAnnotationDisplay)
    settings.setValue('DataProbe/sliceViewAnnotations.topRight',
        self.topRightAnnotationDisplay)
    settings.setValue('DataProbe/sliceViewAnnotations.bottomLeft',
        self.bottomLeftAnnotationDisplay)

  def onFontFamilyRadioButton(self):
    # Updating font size and family
    if self.timesFontRadioButton.checked:
      self.fontFamily = 'Times'
    else:
      self.fontFamily = 'Arial'
    self.updateSliceViewFromGUI()
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.fontFamily',
        self.fontFamily)
    self.updateSliceViewFromGUI()

  def onFontSizeSpinBox(self):
    self.fontSize = self.fontSizeSpinBox.value
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.fontSize',
        self.fontSize)
    self.updateSliceViewFromGUI()

  def onTextLengthSpinBox(self):
    self.maximumTextLength = self.textLengthSpinBox.value
    self.updateSliceViewFromGUI()

  def restoreDefaultValues(self):
    self.topLeftActivationCheckbox.checked = True
    self.topLeftAnnotationDisplay = 1
    self.topRightActivationCheckbox.checked = True
    self.topRightAnnotationDisplay = 1
    self.bottomLeftActivationCheckbox.checked = True
    self.bottomLeftAnnotationDisplay = 1
    self.fontSizeSpinBox.value = 14
    self.timesFontRadioButton.checked = True
    self.fontFamily = 'Times'
    self.backgroundDicomAnnotationsPersistence = 0
    self.backgroundLayerPersistenceCheckbox.checked = False
    self.showScalingRulerCheckBox.checked = True
    self.showScalingRuler = 1
    self.showColorScalarBarCheckBox.checked = False
    self.showColorScalarBar = 0
    self.colorScalarBarMaxWidthSlider.value = 50

    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.show',
        self.showSliceViewAnnotations)
    settings.setValue('DataProbe/sliceViewAnnotations.topLeft',
        self.topLeftAnnotationDisplay)
    settings.setValue('DataProbe/sliceViewAnnotations.topRight',
        self.topRightAnnotationDisplay)
    settings.setValue('DataProbe/sliceViewAnnotations.bottomLeft',
        self.bottomLeftAnnotationDisplay)
    settings.setValue('DataProbe/sliceViewAnnotations.fontFamily',
        self.fontFamily)
    settings.setValue('DataProbe/sliceViewAnnotations.fontSize',
        self.fontSize)
    settings.setValue('DataProbe/sliceViewAnnotations.bgDicomAnnotationsPersistence',
        self.backgroundDicomAnnotationsPersistence)
    settings.setValue('DataProbe/sliceViewAnnotations.showScalingRuler',
        self.showScalingRuler)
    settings.setValue('DataProbe/sliceViewAnnotations.showColorScalarBar',
        self.showColorScalarBar)
    self.updateSliceViewFromGUI()

  def updateMRMLFromGUI(self):
    self.parameterNode.SetParameter(self.parameter, str(showSliceViewAnnotations))

  def updateGUIFromMRML(self,caller,event):
    if self.parameterNode.GetParameter(self.parameter) == '':
      # parameter does not exist - probably intializing
      return
    showStatus = int(self.parameterNode.GetParameter(self.parameter))
    self.showSliceViewAnnotations = showStatus
    self.showScalingRuler = showStatus
    self.showColorScalarBar = showStatus
    self.updateSliceViewFromGUI()

  def openSettingsPopup(self):
    if not self.window.isVisible():
      self.window.show()
    self.window.raise_()

  def close(self):
    self.window.hide()

  def updateSliceViewFromGUI(self):
    # Create corner annotations if have not created already
    if len(self.sliceCornerAnnotations.items()) == 0:
      self.createCornerAnnotations()

    for sliceViewName in self.sliceViewNames:
      cornerAnnotation = self.sliceCornerAnnotations[sliceViewName]
      cornerAnnotation.SetMaximumFontSize(self.fontSize)
      cornerAnnotation.SetMinimumFontSize(self.fontSize)
      cornerAnnotation.SetNonlinearFontScaleFactor(1)
      textProperty = cornerAnnotation.GetTextProperty()
      scalarBar = self.colorScalarBars[sliceViewName]
      scalarBarTextProperty = scalarBar.GetLabelTextProperty()
      scalarBarTextProperty.ItalicOff()
      scalarBarTextProperty.BoldOff()
      if self.fontFamily == 'Times':
        textProperty.SetFontFamilyToTimes()
        scalarBarTextProperty.SetFontFamilyToTimes()
      else:
        textProperty.SetFontFamilyToArial()
        scalarBarTextProperty.SetFontFamilyToArial()

    # Updating Annotations Amount
    if self.level1RadioButton.checked:
      self.annotationsDisplayAmount = 0
    elif self.level2RadioButton.checked:
      self.annotationsDisplayAmount = 1
    elif self.level3RadioButton.checked:
      self.annotationsDisplayAmount = 2

    if self.showSliceViewAnnotations:
      self.cornerTextParametersCollapsibleButton.enabled = True
      self.activateCornersGroupBox.enabled = True
      self.fontPropertiesGroupBox.enabled = True
      self.colorBarLayerSelectionGroupBox.enabled = True
      self.annotationsAmountGroupBox.enabled = True
      self.scalingRulerCollapsibleButton.enabled = True
      self.colorScalarBarCollapsibleButton.enabled = True

      for sliceViewName in self.sliceViewNames:
        sliceWidget = self.layoutManager.sliceWidget(sliceViewName)
        sl = sliceWidget.sliceLogic()
        self.makeAnnotationText(sl)
        self.makeScalingRuler(sl)
        self.makeColorScalarBar(sl)
    else:
      self.cornerTextParametersCollapsibleButton.enabled = False
      self.activateCornersGroupBox.enabled = False
      self.colorBarLayerSelectionGroupBox.enabled = False
      self.fontPropertiesGroupBox.enabled = False
      self.annotationsAmountGroupBox.enabled = False
      self.scalingRulerCollapsibleButton.enabled = False
      self.colorScalarBarCollapsibleButton.enabled = False
      # Remove Observers
      for sliceViewName in self.sliceViewNames:
        sliceWidget = self.layoutManager.sliceWidget(sliceViewName)
        sl = sliceWidget.sliceLogic()
        self.makeScalingRuler(sl)
        self.makeColorScalarBar(sl)

      # Clear Annotations
      for sliceViewName in self.sliceViewNames:
        self.sliceCornerAnnotations[sliceViewName].SetText(0, "")
        self.sliceCornerAnnotations[sliceViewName].SetText(1, "")
        self.sliceCornerAnnotations[sliceViewName].SetText(2, "")
        self.sliceCornerAnnotations[sliceViewName].SetText(3, "")
        self.sliceViews[sliceViewName].scheduleRender()

      # reset golobal variables
      self.sliceCornerAnnotations = {}

  def createGlobalVariables(self):
    self.sliceViewNames = []
    self.sliceWidgets = {}
    self.sliceViews = {}
    self.cameras = {}
    self.blNodeObserverTag = {}
    self.sliceLogicObserverTag = {}
    self.sliceCornerAnnotations = {}
    self.renderers = {}
    self.scalingRulerActors = {}
    self.points = {}
    self.scalingRulerTextActors = {}
    self.colorScalarBars = {}
    self.colorScalarBarWidgets = {}

  def createCornerAnnotations(self):
    self.createGlobalVariables()
    sliceViewNames = self.layoutManager.sliceViewNames()

    for sliceViewName in sliceViewNames:
      self.sliceViewNames.append(sliceViewName)
    for sliceViewName in self.sliceViewNames:
      self.addObserver(sliceViewName)
      self.createActors(sliceViewName)

  def addObserver(self, sliceViewName):
    sliceWidget = self.layoutManager.sliceWidget(sliceViewName)
    self.sliceWidgets[sliceViewName] = sliceWidget
    sliceView = sliceWidget.sliceView()

    renderWindow = sliceView.renderWindow()
    renderWindow.GetRenderers()
    renderer = renderWindow.GetRenderers().GetItemAsObject(0)
    self.renderers[sliceViewName] = renderer

    self.sliceViews[sliceViewName] = sliceView
    self.sliceCornerAnnotations[sliceViewName] = sliceView.cornerAnnotation()
    sliceLogic = sliceWidget.sliceLogic()
    self.sliceLogicObserverTag[sliceViewName] = sliceLogic.AddObserver(vtk.vtkCommand.ModifiedEvent,
                                             self.updateCornerAnnotations)

  def createActors(self, sliceViewName):
    sliceWidget = self.layoutManager.sliceWidget(sliceViewName)
    self.sliceWidgets[sliceViewName] = sliceWidget
    sliceView = sliceWidget.sliceView()

    self.scalingRulerTextActors[sliceViewName] = vtk.vtkTextActor()
    textActor = self.scalingRulerTextActors[sliceViewName]
    textProperty = textActor.GetTextProperty()
    textProperty.SetShadow(1)
    self.scalingRulerActors[sliceViewName] = vtk.vtkActor2D()
    # Create Scaling Ruler
    self.createScalingRuler(sliceViewName)
    # Create Color Scalar Bar
    self.colorScalarBars[sliceViewName] = self.createColorScalarBar(sliceViewName)

  def createScalingRuler(self, sliceViewName):
    #
    # Create the Scaling Ruler
    #
    self.points[sliceViewName] = vtk.vtkPoints()
    self.points[sliceViewName].SetNumberOfPoints(22)

    lines = []
    for i in xrange(0,21):
      line = vtk.vtkLine()
      lines.append(line)

    # setting the points to lines
    for i in xrange(0,21):
      if (i%2 == 0):
        lines[i].GetPointIds().SetId(0,i)
        lines[i].GetPointIds().SetId(1,i+1)
      else:
        lines[i].GetPointIds().SetId(0,i-1)
        lines[i].GetPointIds().SetId(1,i+1)

    # Create a cell array to store the lines in and add the lines to it
    linesArray = vtk.vtkCellArray()
    for i in xrange(0,21):
      linesArray.InsertNextCell(lines[i])

    # Create a polydata to store everything in
    linesPolyData = vtk.vtkPolyData()

    # Add the points to the dataset
    linesPolyData.SetPoints(self.points[sliceViewName])

    # Add the lines to the dataset
    linesPolyData.SetLines(linesArray)

    # mapper
    mapper = vtk.vtkPolyDataMapper2D()
    if vtk.VTK_MAJOR_VERSION <= 5:
      mapper.SetInput(linesPolyData)
    else:
      mapper.SetInputData(linesPolyData)
    # actor
    actor = self.scalingRulerActors[sliceViewName]
    actor.SetMapper(mapper)
    # color actor
    actor.GetProperty().SetColor(1,1,1)
    actor.GetProperty().SetLineWidth(1)
    textActor = self.scalingRulerTextActors[sliceViewName]
    textProperty = textActor.GetTextProperty()

  def createColorScalarBar(self, sliceViewName):
    scalarBar = vtk.vtkScalarBarActor()
    scalarBar.SetTitle("")
    # adjust text property
    lookupTable = vtk.vtkLookupTable()
    scalarBar.SetLookupTable(lookupTable)
    scalarBarWidget = vtk.vtkScalarBarWidget()
    scalarBarWidget.SetScalarBarActor(scalarBar)
    scalarBarWidget.RepositionableOff()
    self.colorScalarBarWidgets[sliceViewName] = scalarBarWidget
    return scalarBar

  def updateCornerAnnotations(self,caller,event):
    sliceViewNames = self.layoutManager.sliceViewNames()
    for sliceViewName in sliceViewNames:
      if sliceViewName not in self.sliceViewNames:
        self.sliceViewNames.append(sliceViewName)
        self.addObserver(sliceViewName)
        self.createActors(sliceViewName)
    self.makeAnnotationText(caller)
    self.makeScalingRuler(caller)
    self.makeColorScalarBar(caller)

  def sliceLogicModifiedEvent(self, caller,event):
    self.updateLayersAnnotation(caller)

  def makeScalingRuler(self, sliceLogic):
    sliceCompositeNode = sliceLogic.GetSliceCompositeNode()

    # Get the layers
    backgroundLayer = sliceLogic.GetBackgroundLayer()
    foregroundLayer = sliceLogic.GetForegroundLayer()
    labelLayer = sliceLogic.GetLabelLayer()

    # Get the volumes
    backgroundVolume = backgroundLayer.GetVolumeNode()
    foregroundVolume = foregroundLayer.GetVolumeNode()
    labelVolume = labelLayer.GetVolumeNode()

    # Get slice view name
    sliceNode = backgroundLayer.GetSliceNode()
    sliceViewName = sliceNode.GetLayoutName()
    self.currentSliceViewName = sliceViewName

    renderer = self.renderers[sliceViewName]

    if self.sliceViews[sliceViewName]:

      #
      # update scaling ruler
      #
      self.minimumWidthForScalingRuler = 200
      viewWidth = self.sliceViews[sliceViewName].width
      viewHeight = self.sliceViews[sliceViewName].height

      rasToXY = vtk.vtkMatrix4x4()
      m = sliceNode.GetXYToRAS()
      rasToXY.DeepCopy(m)
      rasToXY.Invert()
      scalingFactorString = " mm"

      # TODO: The current logic only supports rulers from 1mm to 10cm
      # add support for other ranges.
      import math
      scalingFactor = math.sqrt( rasToXY.GetElement(0,0)**2 +
          rasToXY.GetElement(0,1)**2 +rasToXY.GetElement(0,2) **2 )

      rulerArea = viewWidth/scalingFactor/4

      if self.showScalingRuler and \
          viewWidth > self.minimumWidthForScalingRuler and\
         rulerArea>0.5 and rulerArea<500 :
        rulerSizesArray = np.array([1,5,10,50,100])
        index = np.argmin(np.abs(rulerSizesArray- rulerArea))

        if rulerSizesArray[index]/10 > 1:
          scalingFactorString = str(int(rulerSizesArray[index]/10))+" cm"
        else:
          scalingFactorString = str(rulerSizesArray[index])+" mm"

        RASRulerSize = rulerSizesArray[index]

        pts = self.points[sliceViewName]

        pts.SetPoint(0,[(viewWidth/2-RASRulerSize*scalingFactor/10*5),5, 0])
        pts.SetPoint(1,[(viewWidth/2-RASRulerSize*scalingFactor/10*5),15, 0])
        pts.SetPoint(2,[(viewWidth/2-RASRulerSize*scalingFactor/10*4),5, 0])
        pts.SetPoint(3,[(viewWidth/2-RASRulerSize*scalingFactor/10*4),10, 0])
        pts.SetPoint(4,[(viewWidth/2-RASRulerSize*scalingFactor/10*3),5, 0])
        pts.SetPoint(5,[(viewWidth/2-RASRulerSize*scalingFactor/10*3),15, 0])
        pts.SetPoint(6,[(viewWidth/2-RASRulerSize*scalingFactor/10*2),5, 0])
        pts.SetPoint(7,[(viewWidth/2-RASRulerSize*scalingFactor/10*2),10, 0])
        pts.SetPoint(8,[(viewWidth/2-RASRulerSize*scalingFactor/10),5, 0])
        pts.SetPoint(9,[(viewWidth/2-RASRulerSize*scalingFactor/10),15, 0])
        pts.SetPoint(10,[viewWidth/2,5, 0])
        pts.SetPoint(11,[viewWidth/2,10, 0])
        pts.SetPoint(12,[(viewWidth/2+RASRulerSize*scalingFactor/10),5, 0])
        pts.SetPoint(13,[(viewWidth/2+RASRulerSize*scalingFactor/10),15, 0])
        pts.SetPoint(14,[(viewWidth/2+RASRulerSize*scalingFactor/10*2),5, 0])
        pts.SetPoint(15,[(viewWidth/2+RASRulerSize*scalingFactor/10*2),10, 0])
        pts.SetPoint(16,[(viewWidth/2+RASRulerSize*scalingFactor/10*3),5, 0])
        pts.SetPoint(17,[(viewWidth/2+RASRulerSize*scalingFactor/10*3),15, 0])
        pts.SetPoint(18,[(viewWidth/2+RASRulerSize*scalingFactor/10*4),5, 0])
        pts.SetPoint(19,[(viewWidth/2+RASRulerSize*scalingFactor/10*4),10, 0])
        pts.SetPoint(20,[(viewWidth/2+RASRulerSize*scalingFactor/10*5),5, 0])
        pts.SetPoint(21,[(viewWidth/2+RASRulerSize*scalingFactor/10*5),15, 0])

        textActor = self.scalingRulerTextActors[sliceViewName]
        textActor.SetInput(scalingFactorString)
        textActor.SetDisplayPosition(int((viewWidth+RASRulerSize*scalingFactor)/2)+10,5)

        renderer.AddActor2D(self.scalingRulerActors[sliceViewName])
        renderer.AddActor2D(textActor)

      else:
        renderer.RemoveActor2D(self.scalingRulerActors[sliceViewName])
        renderer.RemoveActor2D(self.scalingRulerTextActors[sliceViewName])

  def makeColorScalarBar(self, sliceLogic):
    sliceCompositeNode = sliceLogic.GetSliceCompositeNode()

    # Get the layers
    backgroundLayer = sliceLogic.GetBackgroundLayer()
    foregroundLayer = sliceLogic.GetForegroundLayer()

    # Get the volumes
    backgroundVolume = backgroundLayer.GetVolumeNode()
    foregroundVolume = foregroundLayer.GetVolumeNode()

    # Get slice view name
    sliceNode = backgroundLayer.GetSliceNode()
    sliceViewName = sliceNode.GetLayoutName()
    self.currentSliceViewName = sliceViewName

    renderer = self.renderers[sliceViewName]

    if self.sliceViews[sliceViewName]:

      scalarBar = self.colorScalarBars[sliceViewName]
      scalarBar.SetTextPositionToPrecedeScalarBar()

      scalarBar.SetMaximumWidthInPixels(int(self.colorScalarBarMaxWidthSlider.value))

      renderWindow = renderer.GetRenderWindow()
      interactor = renderWindow.GetInteractor()

      # create the scalarBarWidget
      scalarBarWidget = self.colorScalarBarWidgets[sliceViewName]
      scalarBarWidget.SetInteractor(interactor)

      # Auto-Adjust
      ## Adjusting the maximum height of the scalar color bar based on view height
      viewHeight = self.sliceViews[sliceViewName].height
      if viewHeight > 200:
        if self.topRightAnnotationDisplay:
          scalarBar.SetMaximumHeightInPixels(int(viewHeight/2 - 30))
        else:
          scalarBar.SetMaximumHeightInPixels(int(viewHeight-30))
      else:
        scalarBar.SetMaximumHeightInPixels(int(viewHeight-30))

      if self.showColorScalarBar:
        #if (backgroundVolume != None and foregroundVolume == None):
        if (backgroundVolume != None and self.colorbarSelectedLayer == 'background'):
          self.updateScalarBarRange(sliceLogic, backgroundVolume, scalarBar, self.colorbarSelectedLayer)
          scalarBarWidget.On()
        elif (foregroundVolume != None and self.colorbarSelectedLayer == 'foreground'):
          self.updateScalarBarRange(sliceLogic, foregroundVolume, scalarBar, self.colorbarSelectedLayer)
          scalarBarWidget.On()
        else:
          scalarBarWidget.Off()
      else:
          scalarBarWidget.Off()

  def makeAnnotationText(self, sliceLogic):
    self.resetTexts()
    sliceCompositeNode = sliceLogic.GetSliceCompositeNode()

    # Get the layers
    backgroundLayer = sliceLogic.GetBackgroundLayer()
    foregroundLayer = sliceLogic.GetForegroundLayer()
    labelLayer = sliceLogic.GetLabelLayer()

    # Get the volumes
    backgroundVolume = backgroundLayer.GetVolumeNode()
    foregroundVolume = foregroundLayer.GetVolumeNode()
    labelVolume = labelLayer.GetVolumeNode()

    # Get slice view name
    sliceNode = backgroundLayer.GetSliceNode()
    sliceViewName = sliceNode.GetLayoutName()
    self.currentSliceViewName = sliceViewName

    renderer = self.renderers[sliceViewName]

    if self.sliceViews[sliceViewName]:
      #
      # Update slice corner annotations
      #
      # Case I: Both background and foregraound
      if ( backgroundVolume != None and foregroundVolume != None):
        foregroundOpacity = sliceCompositeNode.GetForegroundOpacity()
        backgroundVolumeName = backgroundVolume.GetName()
        foregroundVolumeName = foregroundVolume.GetName()
        self.cornerTexts[0]['3-Background']['text'] = 'B: ' + backgroundVolumeName
        self.cornerTexts[0]['2-Foreground']['text'] = 'F: ' + foregroundVolumeName +  ' (' + str(
                      "%.1f"%foregroundOpacity) + ')'

        bgUids = backgroundVolume.GetAttribute('DICOM.instanceUIDs')
        fgUids = foregroundVolume.GetAttribute('DICOM.instanceUIDs')
        if (bgUids and fgUids):
          bgUid = bgUids.partition(' ')[0]
          fgUid = fgUids.partition(' ')[0]
          self.makeDicomAnnotation(bgUid,fgUid)
        elif (bgUids and self.backgroundDicomAnnotationsPersistence):
          uid = bgUids.partition(' ')[0]
          self.makeDicomAnnotation(uid,None)
        else:
          for key in self.cornerTexts[2]:
            self.cornerTexts[2][key]['text'] = ''

      # Case II: Only background
      elif (backgroundVolume != None):
        backgroundVolumeName = backgroundVolume.GetName()
        if self.bottomLeftAnnotationDisplay:
          self.cornerTexts[0]['3-Background']['text'] = 'B: ' + backgroundVolumeName

        uids = backgroundVolume.GetAttribute('DICOM.instanceUIDs')
        if uids:
          uid = uids.partition(' ')[0]
          self.makeDicomAnnotation(uid,None)

      # Case III: Only foreground
      elif (foregroundVolume != None):
        if self.bottomLeftAnnotationDisplay:
          foregroundVolumeName = foregroundVolume.GetName()
          self.cornerTexts[0]['2-Foreground']['text'] = 'F: ' + foregroundVolumeName

        uids = foregroundVolume.GetAttribute('DICOM.instanceUIDs')
        if uids:
          uid = uids.partition(' ')[0]
          # passed UID as bg
          self.makeDicomAnnotation(uid,None)

      if (labelVolume != None):
        labelOpacity = sliceCompositeNode.GetLabelOpacity()
        labelVolumeName = labelVolume.GetName()
        self.cornerTexts[0]['1-Label']['text'] = 'L: ' + labelVolumeName + ' (' + str(
                      "%.1f"%labelOpacity) + ')'

      self.drawCornerAnnotations()

  def updateScalarBarRange(self, sliceLogic, volumeNode, scalarBar, selectedLayer):
    vdn = volumeNode.GetDisplayNode()
    if vdn:
      vcn = vdn.GetColorNode()
      lut = vcn.GetLookupTable()
      lut2 = vtk.vtkLookupTable()
      lut2.DeepCopy(lut)
      width = vtk.mutable(0)
      level = vtk.mutable(0)
      rangeLow = vtk.mutable(0)
      rangeHigh = vtk.mutable(0)
      if selectedLayer == 'background':
        sliceLogic.GetBackgroundWindowLevelAndRange(width,level,rangeLow,rangeHigh)
      else:
        sliceLogic.GetForegroundWindowLevelAndRange(width,level,rangeLow,rangeHigh)
      lut2.SetRange(int(level-width/2),int(level+width/2))
      scalarBar.SetLookupTable(lut2)
      if width < 5 and np.abs(level)<10 :
        scalarBar.SetLabelFormat("%4.2f")
      else:
        scalarBar.SetLabelFormat("%4.0f")

  def makeDicomAnnotation(self,bgUid,fgUid):
    viewHeight = self.sliceViews[self.currentSliceViewName].height
    if fgUid != None and bgUid != None:
      backgroundDicomDic = self.extractDICOMValues(bgUid)
      foregroundDicomDic = self.extractDICOMValues(fgUid)
      # check if background and foreground are from different patients
      # and remove the annotations

      if self.topLeftAnnotationDisplay and viewHeight > 150:
        if backgroundDicomDic['Patient Name'] != foregroundDicomDic['Patient Name'
            ] or backgroundDicomDic['Patient ID'] != foregroundDicomDic['Patient ID'
              ] or backgroundDicomDic['Patient Birth Date'] != foregroundDicomDic['Patient Birth Date']:
              for key in self.cornerTexts[2]:
                self.cornerTexts[2][key]['text'] = ''
        else:
          self.cornerTexts[2]['1-PatientName']['text'] = backgroundDicomDic['Patient Name'].replace('^',', ')
          self.cornerTexts[2]['2-PatientID']['text'] = 'ID: ' + backgroundDicomDic['Patient ID']
          backgroundDicomDic['Patient Birth Date'] = self.formatDICOMDate(backgroundDicomDic['Patient Birth Date'])
          self.cornerTexts[2]['3-PatientInfo']['text'] = self.makePatientInfo(backgroundDicomDic)

          if (backgroundDicomDic['Study Date'] != foregroundDicomDic['Study Date']):
            self.cornerTexts[2]['4-Bg-StudyDate']['text'] = 'B: ' + self.formatDICOMDate(backgroundDicomDic['Study Date'])
            self.cornerTexts[2]['5-Fg-StudyDate']['text'] = 'F: ' + self.formatDICOMDate(foregroundDicomDic['Study Date'])
          else:
            self.cornerTexts[2]['4-Bg-StudyDate']['text'] =  self.formatDICOMDate(backgroundDicomDic['Study Date'])

          if (backgroundDicomDic['Study Time'] != foregroundDicomDic['Study Time']):
            self.cornerTexts[2]['6-Bg-StudyTime']['text'] = 'B: ' + self.formatDICOMTime(backgroundDicomDic['Study Time'])
            self.cornerTexts[2]['7-Fg-StudyTime']['text'] = 'F: ' + self.formatDICOMTime(foregroundDicomDic['Study Time'])
          else:
            self.cornerTexts[2]['6-Bg-StudyTime']['text'] = self.formatDICOMTime(backgroundDicomDic['Study Time'])

          if (backgroundDicomDic['Series Description'] != foregroundDicomDic['Series Description']):
            self.cornerTexts[2]['8-Bg-SeriesDescription']['text'] = 'B: ' + backgroundDicomDic['Series Description']
            self.cornerTexts[2]['9-Fg-SeriesDescription']['text'] = 'F: ' + foregroundDicomDic['Series Description']
          else:
            self.cornerTexts[2]['8-Bg-SeriesDescription']['text'] = backgroundDicomDic['Series Description']

    # Only Background or Only Foreground
    else:
      uid = bgUid
      dicomDic = self.extractDICOMValues(uid)

      if self.topLeftAnnotationDisplay and viewHeight > 150:
        self.cornerTexts[2]['1-PatientName']['text'] = dicomDic['Patient Name'].replace('^',', ')
        self.cornerTexts[2]['2-PatientID']['text'] = 'ID: ' + dicomDic ['Patient ID']
        dicomDic['Patient Birth Date'] = self.formatDICOMDate(dicomDic['Patient Birth Date'])
        self.cornerTexts[2]['3-PatientInfo']['text'] = self.makePatientInfo(dicomDic)
        self.cornerTexts[2]['4-Bg-StudyDate']['text'] = self.formatDICOMDate(dicomDic['Study Date'])
        self.cornerTexts[2]['6-Bg-StudyTime']['text'] = self.formatDICOMTime(dicomDic['Study Time'])
        self.cornerTexts[2]['8-Bg-SeriesDescription']['text'] = dicomDic['Series Description']

      # top right corner annotation would be hidden if colorscalarbar is on and
      # view height is less than 260 pixels
      if (self.topRightAnnotationDisplay and ((not self.showColorScalarBar) or viewHeight > 260)):
        self.cornerTexts[3]['1-Institution-Name']['text'] = dicomDic['Institution Name']
        self.cornerTexts[3]['2-Referring-Phisycian']['text'] = dicomDic['Referring Physician Name'].replace('^',', ')
        self.cornerTexts[3]['3-Manufacturer']['text'] = dicomDic['Manufacturer']
        self.cornerTexts[3]['4-Model']['text'] = dicomDic['Model']
        self.cornerTexts[3]['5-Patient-Position']['text'] = dicomDic['Patient Position']
        modality = dicomDic['Modality']
        if modality == 'MR':
         self.cornerTexts[3]['6-TR']['text']  = 'TR ' + dicomDic['Repetition Time']
         self.cornerTexts[3]['7-TE']['text'] = 'TE ' + dicomDic['Echo Time']

  def makePatientInfo(self,dicomDic):
    # This will give an string of patient's birth date,
    # patient's age and sex
    patientInfo = dicomDic['Patient Birth Date'
          ] + ', ' + dicomDic['Patient Age'
              ] + ', ' + dicomDic['Patient Sex']
    return patientInfo

  def formatDICOMDate(self, date):
    standardDate = ''
    if date != '':
      date = date.rstrip()
      # convert to ISO 8601 Date format
      standardDate = date[:4] + '-' + date[4:6]+ '-' + date[6:]
    return standardDate


  def formatDICOMTime(self, time):
    studyH = time[:2]
    if int(studyH) > 12 :
      studyH = str (int(studyH) - 12)
      clockTime = ' PM'
    else:
      studyH = studyH
      clockTime = ' AM'
    studyM = time[2:4]
    studyS = time[4:6]
    return studyH + ':' + studyM  + ':' + studyS +clockTime

  def fitText(self,text,textSize):
    if len(text) > textSize:
      preSize = textSize/ 2
      postSize = preSize - 3
      text = text[:preSize] + "..." + text[-postSize:]
    return text

  def drawCornerAnnotations(self):
    # Auto-Adjust
    # adjust maximum text length based on fontsize and view width
    viewWidth = self.sliceViews[self.currentSliceViewName].width
    self.maximumTextLength = (viewWidth - 40) / self.fontSize
    cornerAnnotation = ''

    for i, cornerText in enumerate(self.cornerTexts):
      keys = sorted(cornerText.keys())
      cornerAnnotation = ''
      for key in keys:
        text = cornerText[key]['text']
        if ( text != ''):
          text = self.fitText(text, self.maximumTextLength)
          # level 1: All categories will be displayed
          if self.annotationsDisplayAmount == 0:
            cornerAnnotation = cornerAnnotation+ text + '\n'
          # level 2: Category A and B will be displayed
          elif self.annotationsDisplayAmount == 1:
            if (cornerText[key]['category'] != 'C'):
              cornerAnnotation = cornerAnnotation+ text + '\n'
          # level 3 only Category A will be displayed
          elif self.annotationsDisplayAmount == 2:
            if (cornerText[key]['category'] == 'A'):
              cornerAnnotation = cornerAnnotation+ text + '\n'
      sliceCornerAnnotation = self.sliceCornerAnnotations[self.currentSliceViewName]
      sliceCornerAnnotation.SetText(i, cornerAnnotation)
      textProperty = sliceCornerAnnotation.GetTextProperty()
      textProperty.SetShadow(1)
    self.sliceViews[self.currentSliceViewName].scheduleRender()

  def resetTexts(self):
    for i, cornerText in enumerate(self.cornerTexts):
      for key in cornerText.keys():
        self.cornerTexts[i][key]['text'] = ''

  def extractDICOMValues(self,uid):
    p ={}
    tags = {
    "0008,0020": "Study Date",
    "0008,0030": "Study Time",
    "0008,0060": "Modality",
    "0008,0070": "Manufacturer",
    "0008,0080": "Institution Name",
    "0008,0090": "Referring Physician Name",
    "0008,103e": "Series Description",
    "0008,1090": "Model",
    "0010,0010": "Patient Name",
    "0010,0020": "Patient ID",
    "0010,0030": "Patient Birth Date",
    "0010,0040": "Patient Sex",
    "0010,1010": "Patient Age",
    "0018,5100": "Patient Position",
    "0018,0080": "Repetition Time",
    "0018,0081": "Echo Time"
    }
    for tag in tags.keys():
      value = slicer.dicomDatabase.instanceValue(uid,tag)
      p[tags[tag]] = value
    return p
