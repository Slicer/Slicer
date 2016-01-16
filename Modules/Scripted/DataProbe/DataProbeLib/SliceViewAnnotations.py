from __future__ import division
import os
import qt
import vtk
import slicer

from slicer.util import settingsValue
from slicer.util import VTKObservationMixin

try:
  import numpy as np
  NUMPY_AVAILABLE = True
except ImportError:
  NUMPY_AVAILABLE = False

import DataProbeUtil

class SliceAnnotations(VTKObservationMixin):
  """Implement the Qt window showing settings for Slice View Annotations
  """
  def __init__(self, layoutManager=None):
    VTKObservationMixin.__init__(self)
    self.hasVTKPVScalarBarActor = hasattr(slicer, 'vtkPVScalarBarActor')
    if not self.hasVTKPVScalarBarActor:
      slicer.logging.warning("SliceAnnotations: Disable features relying on vtkPVScalarBarActor")

    self.layoutManager = layoutManager
    if self.layoutManager is None:
      self.layoutManager = slicer.app.layoutManager()
    self.layoutManager.connect("destroyed()", self.onLayoutManagerDestroyed)

    self.dataProbeUtil = DataProbeUtil.DataProbeUtil()

    self.dicomVolumeNode = 0
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
      '4-Bg-SeriesDate':{'text':'','category':'B'},
      '5-Fg-SeriesDate':{'text':'','category':'B'},
      '6-Bg-SeriesTime':{'text':'','category':'C'},
      '7-Bg-SeriesTime':{'text':'','category':'C'},
      '8-Bg-SeriesDescription':{'text':'','category':'C'},
      '9-Fg-SeriesDescription':{'text':'','category':'C'}
      })
    # Top Right Corner Text
    self.cornerTexts.append({
      '1-Institution-Name':{'text':'','category':'B'},
      '2-Referring-Phisycian':{'text':'','category':'B'},
      '3-Manufacturer':{'text':'','category':'C'},
      '4-Model':{'text':'','category':'C'},
      '5-Patient-Position':{'text':'','category':'A'},
      '6-TR':{'text':'','category':'A'},
      '7-TE':{'text':'','category':'A'}
      })

    self.annotationsDisplayAmount = 0

    #
    self.scene = slicer.mrmlScene
    self.sliceViews = {}

    self.viewPortFinishHeight = 0.3
    self.viewPortStartWidth = 0.8

    self.cube = None
    self.axes = None
    self.humanActor = None

    # If there are no user settings load defaults
    self.sliceViewAnnotationsEnabled = settingsValue('DataProbe/sliceViewAnnotations.enabled', 1, converter=int)

    self.bottomLeft = settingsValue('DataProbe/sliceViewAnnotations.bottomLeft', 1, converter=int)
    self.topLeft = settingsValue('DataProbe/sliceViewAnnotations.topLeft', 0, converter=int)
    self.topRight = settingsValue('DataProbe/sliceViewAnnotations.topRight', 0, converter=int)
    self.fontFamily = settingsValue('DataProbe/sliceViewAnnotations.fontFamily', 'Times')
    self.fontSize = settingsValue('DataProbe/sliceViewAnnotations.fontSize', 14, converter=int)
    self.backgroundDICOMAnnotationsPersistence = settingsValue(
        'DataProbe/sliceViewAnnotations.bgDICOMAnnotationsPersistence', 0, converter=int)

    self.rulerEnabled = settingsValue('DataProbe/sliceViewAnnotations.rulerEnabled', 1, converter=int)
    self.rulerEnabledLastStatus = self.rulerEnabled

    self.scalarBarEnabled = settingsValue('DataProbe/sliceViewAnnotations.scalarBarEnabled', 0, converter=int)
    self.scalarBarEnabledLastStatus = self.scalarBarEnabled
    self.scalarBarSelectedLayer = 'background'
    self.rangeLabelFormat = settingsValue('DataProbe/sliceViewAnnotations.rangeLabelFormat', '%G')

    self.orientationMarkerEnabled = settingsValue('DataProbe/sliceViewAnnotations.orientationMarkerEnabled', 0, converter=int)
    self.orientationMarkerType = settingsValue('DataProbe/sliceViewAnnotations.orientationMarkerType', 'Cube')
    self.orientationMarkerEnabledLastStatus = self.orientationMarkerEnabled
    self.zoomValue = settingsValue('DataProbe/sliceViewAnnotations.zoomValue', 20 , converter=int)

    self.parameter = 'sliceViewAnnotationsEnabled'
    self.parameterNode = self.dataProbeUtil.getParameterNode()
    self.addObserver(self.parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

    self.maximumTextLength= 35

    self.create()

    if self.sliceViewAnnotationsEnabled:
      self.updateSliceViewFromGUI()

  def create(self):
    # Instantiate and connect widgets ...
    loader = qt.QUiLoader()
    path = os.path.join(os.path.dirname(__file__), 'Resources', 'UI','settings.ui')
    qfile = qt.QFile(path)
    qfile.open(qt.QFile.ReadOnly)
    self.window = loader.load(qfile)
    window = self.window

    find = slicer.util.findChildren
    self.cornerTextParametersCollapsibleButton = find(window, 'cornerTextParametersCollapsibleButton')[0]
    self.sliceViewAnnotationsCheckBox = find(window,'sliceViewAnnotationsCheckBox')[0]
    self.sliceViewAnnotationsCheckBox.checked = self.sliceViewAnnotationsEnabled

    self.activateCornersGroupBox = find(window,'activateCornersGroupBox')[0]
    self.topLeftCheckBox = find(window,'topLeftCheckBox')[0]
    self.topLeftCheckBox.checked = self.topLeft
    self.topRightCheckBox = find(window,'topRightCheckBox')[0]
    self.topRightCheckBox.toolTip = "Top right corner annotation is only enabled when the scalarBar is off."
    self.topRightCheckBox.checked = self.topRight
    self.topRightCheckBox.enabled = not(self.scalarBarEnabled)

    self.bottomLeftCheckBox = find(window, 'bottomLeftCheckBox')[0]
    self.bottomLeftCheckBox.checked = self.bottomLeft

    self.level1RadioButton = find(window,'level1RadioButton')[0]
    self.level2RadioButton = find(window,'level2RadioButton')[0]
    self.level3RadioButton = find(window,'level3RadioButton')[0]

    self.fontPropertiesGroupBox = find(window,'fontPropertiesGroupBox')[0]
    self.timesFontRadioButton = find(window,'timesFontRadioButton')[0]
    self.arialFontRadioButton = find(window,'arialFontRadioButton')[0]
    if self.fontFamily == 'Times':
      self.timesFontRadioButton.checked = True
    else:
      self.arialFontRadioButton.checked = True

    self.fontSizeSpinBox = find(window,'fontSizeSpinBox')[0]
    self.fontSizeSpinBox.value = self.fontSize

    self.backgroundPersistenceCheckBox = find(window,'backgroundPersistenceCheckBox')[0]
    self.backgroundPersistenceCheckBox.checked = self.backgroundDICOMAnnotationsPersistence

    self.annotationsAmountGroupBox = find(window,'annotationsAmountGroupBox')[0]
    self.rulerCollapsibleButton = find(window,'rulerCollapsibleButton')[0]
    self.rulerEnableCheckBox = find(window, 'rulerEnableCheckBox')[0]
    self.rulerEnableCheckBox.checked = self.rulerEnabled

    self.scalarBarCollapsibleButton = find(window,'scalarBarCollapsibleButton')[0]
    self.scalarBarEnalbeCheckBox = find(window,'scalarBarEnableCheckBox')[0]
    self.scalarBarEnalbeCheckBox.checked = self.scalarBarEnabled
    self.scalarBarLayerSelectionGroupBox = find(window,'scalarBarLayerSelectionGroupBox')[0]

    self.backgroundRadioButton = find(window, 'backgroundRadioButton')[0]
    self.backgroundRadioButton.checked = True
    self.foregroundRadioButton = find(window, 'foregroundRadioButton')[0]
    self.rangeLabelFormatLineEdit = find(window,'rangeLabelFormatLineEdit')[0]
    self.rangeLabelFormatLineEdit.text = self.rangeLabelFormat

    # Orientation Marker Controllers
    self.orientationMarkerEnableCheckBox =  find(window,'orientationMarkerEnableCheckBox')[0]
    self.orientationMarkerEnableCheckBox.checked = self.orientationMarkerEnabled
    self.zoomSlider =  find(window,'zoomSlider')[0]
    self.widthSlider =  find(window,'widthSlider')[0]
    self.heightSlider =  find(window,'heightSlider')[0]
    self.cubeRadioButton = find(window,'cubeRadioButton')[0]
    self.humanRadioButton = find(window,'humanRadioButton')[0]
    self.axesRadioButton = find(window,'axesRadioButton')[0]
    if self.orientationMarkerType == 'Cube':
      self.cubeRadioButton.checked = True
    elif self.orientationMarkerType == 'Human':
      self.humanRadioButton.checked = True
    else:
      self.axesRadioButton.checked = True

    self.restoreDefaultsButton = find(window, 'restoreDefaultsButton')[0]

    # connections
    self.sliceViewAnnotationsCheckBox.connect('clicked()', self.onSliceViewAnnotationsCheckBox)

    self.topLeftCheckBox.connect('clicked()', self.onCornerTextsActivationCheckBox)
    self.topRightCheckBox.connect('clicked()', self.onCornerTextsActivationCheckBox)
    self.bottomLeftCheckBox.connect('clicked()', self.onCornerTextsActivationCheckBox)
    self.timesFontRadioButton.connect('clicked()', self.onFontFamilyRadioButton)
    self.arialFontRadioButton.connect('clicked()', self.onFontFamilyRadioButton)
    self.fontSizeSpinBox.connect('valueChanged(int)', self.onFontSizeSpinBox)

    self.level1RadioButton.connect('clicked()', self.updateSliceViewFromGUI)
    self.level2RadioButton.connect('clicked()', self.updateSliceViewFromGUI)
    self.level3RadioButton.connect('clicked()', self.updateSliceViewFromGUI)

    self.backgroundPersistenceCheckBox.connect('clicked()', self.onBackgroundLayerPersistenceCheckBox)

    self.rulerEnableCheckBox.connect('clicked()', self.onShowRulerCheckBox)

    self.scalarBarEnalbeCheckBox.connect('clicked()', self.onScalarBarCheckBox)
    self.backgroundRadioButton.connect('clicked()',self.onLayerSelectionRadioButton)
    self.foregroundRadioButton.connect('clicked()',self.onLayerSelectionRadioButton)
    self.rangeLabelFormatLineEdit.connect('editingFinished()',self.onRangeLabelFormatLineEdit)
    self.rangeLabelFormatLineEdit.connect('returnPressed()',self.onRangeLabelFormatLineEdit)

    self.orientationMarkerEnableCheckBox.connect('clicked()', self.onOrientationMarkerCheckBox)
    self.zoomSlider.connect('valueChanged(double)', self.onZoomSlider)
    self.zoomSlider.value = self.zoomValue
    self.widthSlider.connect('valueChanged(double)', self.widthValueChanged)
    self.heightSlider.connect('valueChanged(double)', self.heightValueChanged)
    for radioButton in [self.cubeRadioButton, self.axesRadioButton, self.humanRadioButton]:
      radioButton.connect('clicked()', self.onOrientationMarkerTypeRadioButton)

    self.restoreDefaultsButton.connect('clicked()', self.restoreDefaultValues)

  def onLayoutManagerDestroyed(self):
    self.layoutManager = slicer.app.layoutManager()
    if self.layoutManager:
      self.layoutManager.connect("destroyed()", self.onLayoutManagerDestroyed)

  def onZoomSlider(self):
    self.zoomValue = self.zoomSlider.value
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.zoomValue',self.zoomValue)
    self.updateSliceViewFromGUI()

  def widthValueChanged(self, value):
    self.viewPortStartWidth = 1- value/100
    self.updateSliceViewFromGUI()

  def heightValueChanged(self, value):
    self.viewPortFinishHeight= value/100
    self.updateSliceViewFromGUI()

  def onSliceViewAnnotationsCheckBox(self):
    if self.sliceViewAnnotationsCheckBox.checked:
      self.sliceViewAnnotationsEnabled = 1
      self.rulerEnableCheckBox.checked = self.rulerEnabledLastStatus
      self.rulerEnabled = self.rulerEnabledLastStatus
      self.scalarBarEnalbeCheckBox.checked = self.scalarBarEnabledLastStatus
      self.scalarBarEnabled = self.scalarBarEnabledLastStatus
      self.orientationMarkerEnabled = self.orientationMarkerEnabledLastStatus
    else:
      self.rulerEnabledLastStatus = self.rulerEnabled
      self.scalarBarEnabledLastStatus = self.scalarBarEnabled
      self.orientationMarkerEnabledLastStatus = self.orientationMarkerEnabled
      self.scalarBarEnalbeCheckBox.checked = False
      self.rulerEnableCheckBox.checked = False
      self.sliceViewAnnotationsEnabled = 0
      self.rulerEnabled = 0
      self.scalarBarEnabled = 0
      self.orientationMarkerEnabled = 0
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.enabled', self.sliceViewAnnotationsEnabled)
    settings.setValue('DataProbe/sliceViewAnnotations.rulerEnabled',self.rulerEnabled)
    settings.setValue('DataProbe/sliceViewAnnotations.scalarBarEnabled', self.scalarBarEnabled)
    settings.setValue('DataProbe/sliceViewAnnotations.orientationMarkerEnabled', self.orientationMarkerEnabled)
    self.updateSliceViewFromGUI()

  def onBackgroundLayerPersistenceCheckBox(self):
    if self.backgroundPersistenceCheckBox.checked:
      self.backgroundDICOMAnnotationsPersistence = 1
    else:
      self.backgroundDICOMAnnotationsPersistence = 0
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.bgDICOMAnnotationsPersistence',
        self.backgroundDICOMAnnotationsPersistence)
    self.updateSliceViewFromGUI()

  def onShowRulerCheckBox(self):
    if self.rulerEnableCheckBox.checked:
      self.rulerEnabled = 1
    else:
      self.rulerEnabled = 0
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.rulerEnabled', self.rulerEnabled)
    self.updateSliceViewFromGUI()

  def onLayerSelectionRadioButton(self):
    if self.backgroundRadioButton.checked:
      self.scalarBarSelectedLayer = 'background'
    else:
      self.scalarBarSelectedLayer = 'foreground'
    self.updateSliceViewFromGUI()

  def onScalarBarCheckBox(self):
    if self.scalarBarEnalbeCheckBox.checked:
      self.topRightCheckBox.enabled = False
    else:
      self.topRightCheckBox.enabled = True
    self.scalarBarEnabled = int(self.scalarBarEnalbeCheckBox.checked)
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.scalarBarEnabled',
        self.scalarBarEnabled)
    self.updateSliceViewFromGUI()

  def onOrientationMarkerCheckBox(self):
    self.orientationMarkerEnabled = int(self.orientationMarkerEnableCheckBox.checked)
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.orientationMarkerEnabled',
        self.orientationMarkerEnabled)
    self.updateSliceViewFromGUI()

  def onCornerTextsActivationCheckBox(self):
    self.topLeft = int(self.topLeftCheckBox.checked)
    self.topRight = int(self.topRightCheckBox.checked)
    self.bottomLeft = int(self.bottomLeftCheckBox.checked)

    if self.topRight:
      self.scalarBarEnalbeCheckBox.checked = False

    self.updateSliceViewFromGUI()

    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.topLeft',
        self.topLeft)
    settings.setValue('DataProbe/sliceViewAnnotations.topRight',
        self.topRight)
    settings.setValue('DataProbe/sliceViewAnnotations.bottomLeft',
        self.bottomLeft)

  def onFontFamilyRadioButton(self):
    # Updating font size and family
    if self.timesFontRadioButton.checked:
      self.fontFamily = 'Times'
    else:
      self.fontFamily = 'Arial'
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

  def onOrientationMarkerTypeRadioButton(self):
    if self.cubeRadioButton.checked:
      self.orientationMarkerType = 'Cube'
    elif self.humanRadioButton.checked:
      self.orientationMarkerType = 'Human'
    else:
      self.orientationMarkerType = 'Axes'
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.orientationMarkerType',
        self.orientationMarkerType)
    self.updateSliceViewFromGUI()

  def onRangeLabelFormatLineEdit(self):
    # Updating font size and family
    self.rangeLabelFormat =  self.rangeLabelFormatLineEdit.text
    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.rangeLabelFormat',
        self.rangeLabelFormat)
    self.updateSliceViewFromGUI()

  def restoreDefaultValues(self):
    self.topLeftCheckBox.checked = True
    self.topLeft = 1
    self.topRightCheckBox.checked = True
    self.topRight = 1
    self.bottomLeftCheckBox.checked = True
    self.bottomLeft = 1
    self.fontSizeSpinBox.value = 14
    self.timesFontRadioButton.checked = True
    self.fontFamily = 'Times'
    self.backgroundDICOMAnnotationsPersistence = 0
    self.backgroundPersistenceCheckBox.checked = False
    self.rulerEnableCheckBox.checked = True
    self.rulerEnabled = 1
    self.scalarBarEnalbeCheckBox.checked = False
    self.scalarBarEnabled = 0
    self.orientationMarkerEnabled = 0
    self.orientationMarkerEnableCheckBox.checked = False
    self.orientationMarkerType = 'Cube'
    self.zoomSlider.value = 20
    self.rangeLabelFormat = '%G'
    self.rangeLabelFormatLineEdit.text = '%G'

    settings = qt.QSettings()
    settings.setValue('DataProbe/sliceViewAnnotations.enabled', self.sliceViewAnnotationsEnabled)
    settings.setValue('DataProbe/sliceViewAnnotations.topLeft', self.topLeft)
    settings.setValue('DataProbe/sliceViewAnnotations.topRight', self.topRight)
    settings.setValue('DataProbe/sliceViewAnnotations.bottomLeft', self.bottomLeft)
    settings.setValue('DataProbe/sliceViewAnnotations.fontFamily',self.fontFamily)
    settings.setValue('DataProbe/sliceViewAnnotations.fontSize',self.fontSize)
    settings.setValue('DataProbe/sliceViewAnnotations.bgDICOMAnnotationsPersistence',
        self.backgroundDICOMAnnotationsPersistence)
    settings.setValue('DataProbe/sliceViewAnnotations.rulerEnabled', self.rulerEnabled)
    settings.setValue('DataProbe/sliceViewAnnotations.scalarBarEnabled', self.scalarBarEnabled)
    settings.setValue('DataProbe/sliceViewAnnotations.orientationMarkerType', self.orientationMarkerType)
    settings.setValue('DataProbe/sliceViewAnnotations.orientationMarkerEnabled', self.orientationMarkerEnabled)
    settings.setValue('DataProbe/sliceViewAnnotations.zoomValue', self.zoomValue)
    settings.setValue('DataProbe/sliceViewAnnotations.rangeLabelFormat', self.rangeLabelFormat)
    self.updateSliceViewFromGUI()

  def updateGUIFromMRML(self,caller,event):
    if self.parameterNode.GetParameter(self.parameter) == '':
      # parameter does not exist - probably intializing
      return
    self.sliceViewAnnotationsEnabled = int(self.parameterNode.GetParameter(self.parameter))
    self.updateSliceViewFromGUI()

  def updateSliceViewFromGUI(self):
    # Create corner annotations if have not created already
    if len(self.sliceViewNames) == 0:
      self.createCornerAnnotations()

    for slider in [self.zoomSlider,self.widthSlider, self.heightSlider]:
      slider.enabled = self.orientationMarkerEnableCheckBox.checked

    # Updating Annotations Amount
    if self.level1RadioButton.checked:
      self.annotationsDisplayAmount = 0
    elif self.level2RadioButton.checked:
      self.annotationsDisplayAmount = 1
    elif self.level3RadioButton.checked:
      self.annotationsDisplayAmount = 2

    enabled = self.sliceViewAnnotationsEnabled

    self.cornerTextParametersCollapsibleButton.enabled = enabled
    self.activateCornersGroupBox.enabled = enabled
    self.fontPropertiesGroupBox.enabled = enabled
    self.scalarBarLayerSelectionGroupBox.enabled = enabled
    self.annotationsAmountGroupBox.enabled = enabled
    self.rulerCollapsibleButton.enabled = enabled
    self.scalarBarCollapsibleButton.enabled = enabled
    self.restoreDefaultsButton.enabled = enabled

    for sliceViewName in self.sliceViewNames:
      sliceWidget = self.layoutManager.sliceWidget(sliceViewName)
      if sliceWidget:
        sl = sliceWidget.sliceLogic()
        self.updateRuler(sl)
        self.updateScalarBar(sl)
        self.updateOrientationMarker(sl)
        self.updateCornerAnnotation(sl)

  def createGlobalVariables(self):
    self.sliceViewNames = []
    self.sliceWidgets = {}
    self.sliceViews = {}
    self.cameras = {}
    self.renderers = {}
    self.rulerActors = {}
    self.points = {}
    self.rulerTextActors = {}
    self.scalarBars = {}
    self.scalarBarWidgets = {}
    self.orientationMarkerRenderers = {}

  def createCornerAnnotations(self):
    self.createGlobalVariables()
    self.sliceViewNames = list(self.layoutManager.sliceViewNames())
    for sliceViewName in self.sliceViewNames:
      self.addSliceViewObserver(sliceViewName)
      self.createActors(sliceViewName)

  def addSliceViewObserver(self, sliceViewName):
    sliceWidget = self.layoutManager.sliceWidget(sliceViewName)
    self.sliceWidgets[sliceViewName] = sliceWidget
    sliceView = sliceWidget.sliceView()

    renderWindow = sliceView.renderWindow()
    renderer = renderWindow.GetRenderers().GetItemAsObject(0)
    self.renderers[sliceViewName] = renderer

    self.sliceViews[sliceViewName] = sliceView
    sliceLogic = sliceWidget.sliceLogic()
    self.addObserver(sliceLogic, vtk.vtkCommand.ModifiedEvent, self.updateViewAnnotations)
    self.orientationMarkerRenderers[sliceViewName] = vtk.vtkRenderer()


  def createActors(self, sliceViewName):
    sliceWidget = self.layoutManager.sliceWidget(sliceViewName)
    self.sliceWidgets[sliceViewName] = sliceWidget

    self.rulerTextActors[sliceViewName] = vtk.vtkTextActor()
    textActor = self.rulerTextActors[sliceViewName]
    textProperty = textActor.GetTextProperty()
    textProperty.SetShadow(1)
    self.rulerActors[sliceViewName] = vtk.vtkActor2D()
    # Create Ruler
    self.createRuler(sliceViewName)
    # Create scalarBar
    self.scalarBars[sliceViewName] = self.createScalarBar(sliceViewName)

  def createRuler(self, sliceViewName):
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
    mapper.SetInputData(linesPolyData)
    # actor
    actor = self.rulerActors[sliceViewName]
    actor.SetMapper(mapper)
    # color actor
    actor.GetProperty().SetLineWidth(1)

  def createScalarBar(self, sliceViewName):
    if self.hasVTKPVScalarBarActor:
      scalarBar = slicer.vtkPVScalarBarActor()
    else:
      scalarBar = vtk.vtkScalarBarActor()
    scalarBar.SetTitle(" ")
    # adjust text property
    if self.hasVTKPVScalarBarActor:
      scalarBar.SetRangeLabelFormat(self.rangeLabelFormat)
    lookupTable = vtk.vtkLookupTable()
    scalarBar.SetLookupTable(lookupTable)
    '''
    scalarBarWidget = vtk.vtkScalarBarWidget()
    scalarBarWidget.SetScalarBarActor(scalarBar)
    self.scalarBarWidgets[sliceViewName] = scalarBarWidget
    '''
    return scalarBar

  def updateViewAnnotations(self,caller,event):
    layoutManager = self.layoutManager
    if layoutManager is None:
      return
    sliceViewNames = layoutManager.sliceViewNames()
    for sliceViewName in sliceViewNames:
      if sliceViewName not in self.sliceViewNames:
        self.sliceViewNames.append(sliceViewName)
        self.addSliceViewObserver(sliceViewName)
        self.createActors(sliceViewName)
        self.updateSliceViewFromGUI()
    self.makeAnnotationText(caller)
    self.updateRuler(caller)
    self.updateScalarBar(caller)
    self.updateOrientationMarker(caller)

  def updateCornerAnnotation(self, sliceLogic):

    sliceNode = sliceLogic.GetBackgroundLayer().GetSliceNode()
    sliceViewName = sliceNode.GetLayoutName()

    enabled = self.sliceViewAnnotationsEnabled

    cornerAnnotation = self.sliceViews[sliceViewName].cornerAnnotation()

    if enabled:
      # Font
      cornerAnnotation.SetMaximumFontSize(self.fontSize)
      cornerAnnotation.SetMinimumFontSize(self.fontSize)
      cornerAnnotation.SetNonlinearFontScaleFactor(1)
      textProperty = cornerAnnotation.GetTextProperty()
      if self.fontFamily == 'Times':
        textProperty.SetFontFamilyToTimes()
      else:
        textProperty.SetFontFamilyToArial()
      # Text
      self.makeAnnotationText(sliceLogic)
    else:
      # Clear Annotations
      for position in range(3):
        cornerAnnotation.SetText(position, "")

  def updateOrientationMarker(self, sliceLogic):
    sliceNode = sliceLogic.GetBackgroundLayer().GetSliceNode()
    if sliceNode is None:
      return
    sliceViewName = sliceNode.GetLayoutName()

    if self.sliceViews[sliceViewName]:
      ren = self.orientationMarkerRenderers[sliceViewName]
      rw = self.sliceViews[sliceViewName].renderWindow()
      ren.SetViewport(self.viewPortStartWidth,0,1,self.viewPortFinishHeight)
      ren.SetLayer(1)

      if self.orientationMarkerEnabled:
        # Add actors to renderer
        if self.orientationMarkerType == 'Cube':
          if self.cube is None:
            self.createCubeOrientationModel()
          ren.AddActor(self.cube)
          if self.humanActor is not None:
            ren.RemoveActor(self.humanActor)
            ren.RemoveActor(self.shortsActor)
            ren.RemoveActor(self.leftShoeActor)
            ren.RemoveActor(self.rightShoeActor)
          if self.axes is not None:
            ren.RemoveActor(self.axes)

        elif self.orientationMarkerType == 'Human':
          if self.humanActor is None:
            self.createHumanOrientationModel()
          ren.AddActor(self.humanActor)
          ren.AddActor(self.shortsActor)
          ren.AddActor(self.leftShoeActor)
          ren.AddActor(self.rightShoeActor)
          if self.cube is not None:
            ren.RemoveActor(self.cube)
          if self.axes is not None:
            ren.RemoveActor(self.axes)

        elif self.orientationMarkerType == 'Axes':
          if self.axes is None:
            self.createAxesOrientationModel()
          ren.AddActor(self.axes)
          if self.humanActor is not None:
            ren.RemoveActor(self.humanActor)
            ren.RemoveActor(self.shortsActor)
            ren.RemoveActor(self.leftShoeActor)
            ren.RemoveActor(self.rightShoeActor)
          if self.cube is not None:
            ren.RemoveActor(self.cube)

        # Calculate the camera position and viewup based on XYToRAS matrix
        camera = vtk.vtkCamera()
        m = sliceNode.GetSliceToRAS()
        v = np.array([[m.GetElement(0,0),m.GetElement(0,1),m.GetElement(0,2)],
            [m.GetElement(1,0),m.GetElement(1,1),m.GetElement(1,2)],
            [m.GetElement(2,0),m.GetElement(2,1),m.GetElement(2,2)]])
        det = np.linalg.det(v)
        cameraPositionMultiplier = 100/self.zoomValue
        y = np.array([0,0,-cameraPositionMultiplier]) # right hand
        if det < 0: # left hand
          y = np.array([0,0,cameraPositionMultiplier])

        x = np.matrix([[m.GetElement(0,0),m.GetElement(0,1),m.GetElement(0,2)],
            [m.GetElement(1,0),m.GetElement(1,1),m.GetElement(1,2)],
            [m.GetElement(2,0),m.GetElement(2,1),m.GetElement(2,2)]])

        # Calculating position
        position = np.inner(x,y)
        camera.SetPosition(-position[0,0],-position[0,1],-position[0,2])
        # Calculating viewUp
        n = np.array([0,1,0])
        viewUp = np.inner(x,n)
        camera.SetViewUp(viewUp[0,0],viewUp[0,1],viewUp[0,2])

        #ren.PreserveDepthBufferOff()
        ren.SetActiveCamera(camera)
        rw.AddRenderer(ren)

      else:
        if self.humanActor is not None:
          ren.RemoveActor(self.humanActor)
          ren.RemoveActor(self.shortsActor)
          ren.RemoveActor(self.leftShoeActor)
          ren.RemoveActor(self.rightShoeActor)
        if self.axes is not None:
          ren.RemoveActor(self.axes)
        if self.cube is not None:
          ren.RemoveActor(self.cube)
        rw.RemoveRenderer(ren)

      # Refresh view
      self.sliceViews[sliceViewName].scheduleRender()

  def createCubeOrientationModel(self):
    self.cube = vtk.vtkAnnotatedCubeActor()
    self.cube.SetXPlusFaceText('R')
    self.cube.SetXMinusFaceText('L')
    self.cube.SetYMinusFaceText('P')
    self.cube.SetYPlusFaceText('A')
    self.cube.SetZMinusFaceText('I')
    self.cube.SetZPlusFaceText('S')
    self.cube.SetZFaceTextRotation(90)
    self.cube.GetTextEdgesProperty().SetColor(0.95,0.95,0.95)
    self.cube.GetTextEdgesProperty().SetLineWidth(2)
    self.cube.GetCubeProperty().SetColor(0.15,0.15,0.15)

  def createHumanOrientationModel(self):
    # Module's Path
    modulePath= slicer.modules.dataprobe.path.replace("DataProbe.py","")

    # Test whether the model is already loaded into the scene or not.
    nodes = self.scene.GetNodesByName('leftShoeOrientationMarker')
    if nodes.GetNumberOfItems() == 0 :
      modelFiles = [ "humanOrientationMarker.stl","shortsOrientationMarker.stl",
      "leftShoeOrientationMarker.stl", "rightShoeOrientationMarker.stl"]
      modelPaths = [modulePath + "DataProbeLib/Resources/Models/"+ modelFile for modelFile in modelFiles]
      for modelPath in modelPaths:
        successfulLoad = slicer.util.loadModel(modelPath)
        if not successfulLoad:
          print 'Warning: Could not load model %s' %modelPath
    nodes.UnRegister(slicer.mrmlScene)

    modelNodes = []
    # Human node
    self.humanNode = slicer.util.getNode('humanOrientationMarker')
    modelNodes.append(self.humanNode)
    # Shorts node
    self.shortsNode = slicer.util.getNode('shortsOrientationMarker')
    modelNodes.append(self.shortsNode)
    # Left shoe node
    self.leftShoeNode = slicer.util.getNode('leftShoeOrientationMarker')
    modelNodes.append(self.leftShoeNode)
    # Right shoe node
    self.rightShoeNode = slicer.util.getNode('rightShoeOrientationMarker')
    modelNodes.append(self.rightShoeNode)

    for node in modelNodes:
      node.HideFromEditorsOn()
      node.SetDisplayVisibility(False)

    # Mappers
    humanMapper = vtk.vtkPolyDataMapper()
    humanMapper.SetInputData(self.humanNode.GetPolyData())

    shortsMapper = vtk.vtkPolyDataMapper()
    shortsMapper.SetInputData(self.shortsNode.GetPolyData())

    leftShoeMapper = vtk.vtkPolyDataMapper()
    leftShoeMapper.SetInputData(self.leftShoeNode.GetPolyData())

    rightShoeMapper = vtk.vtkPolyDataMapper()
    rightShoeMapper.SetInputData(self.rightShoeNode.GetPolyData())

    # Actors
    scale = 0.01
    self.humanActor = vtk.vtkActor()
    self.humanActor.SetMapper(humanMapper)

    '''
    The human skin tone (color) was chosen based on the unicode.org recommendations:
    It was chosen as non-realistic skin tone with RGG: #FFCC22
    Source: http://unicode.org/reports/tr51/
    '''
    self.humanActor.GetProperty().SetColor(255/256,204/256,34/256)
    self.humanActor.SetScale(scale,scale,scale)

    self.shortsActor = vtk.vtkActor()
    self.shortsActor.SetMapper(shortsMapper)
    self.shortsActor.GetProperty().SetColor(0,0,1)
    self.shortsActor.SetScale(scale,scale,scale)

    self.leftShoeActor = vtk.vtkActor()
    self.leftShoeActor.SetMapper(leftShoeMapper)
    self.leftShoeActor.GetProperty().SetColor(1,0,0)
    self.leftShoeActor.SetScale(scale,scale,scale)

    self.rightShoeActor = vtk.vtkActor()
    self.rightShoeActor.SetMapper(rightShoeMapper)
    self.rightShoeActor.GetProperty().SetColor(0,1,0)
    self.rightShoeActor.SetScale(scale,scale,scale)

  def createAxesOrientationModel(self):
    self.axes = vtk.vtkAxesActor()
    self.axes.SetXAxisLabelText('R')
    self.axes.SetYAxisLabelText('A')
    self.axes.SetZAxisLabelText('S')
    transform = vtk.vtkTransform()
    transform.Translate(0,0,0)
    self.axes.SetUserTransform(transform)


  def updateRuler(self, sliceLogic):
    sliceCompositeNode = sliceLogic.GetSliceCompositeNode()
    if not sliceCompositeNode:
      return

    # Get the layers
    sliceNode = sliceLogic.GetBackgroundLayer().GetSliceNode()
    if not sliceNode:
      return
    sliceViewName = sliceNode.GetLayoutName()
    renderer = self.renderers[sliceViewName]
    if self.rulerEnabled:
      self.makeRuler(sliceNode)
    else:
      renderer.RemoveActor2D(self.rulerActors[sliceViewName])
      renderer.RemoveActor2D(self.rulerTextActors[sliceViewName])

  def updateScalarBar(self, sliceLogic):
    sliceCompositeNode = sliceLogic.GetSliceCompositeNode()
    if not sliceCompositeNode:
      return

    # Get the layers
    backgroundLayer = sliceLogic.GetBackgroundLayer()
    # Get slice view name
    sliceNode = backgroundLayer.GetSliceNode()
    if not sliceNode:
      return
    sliceViewName = sliceNode.GetLayoutName()
    scalarBar = self.scalarBars[sliceViewName]
    renderer = self.renderers[sliceViewName]
    #scalarBarWidget = self.scalarBarWidgets[sliceViewName]
    if self.scalarBarEnabled:
      self.modifyScalarBar(sliceLogic)
    else:
      renderer.RemoveActor(scalarBar)

  def makeRuler(self, sliceNode):
    sliceViewName = sliceNode.GetLayoutName()
    renderer = self.renderers[sliceViewName]
    if self.sliceViews[sliceViewName]:
      #
      # update scaling ruler
      #
      self.minimumWidthForRuler = 200
      viewWidth = self.sliceViews[sliceViewName].width

      rasToXY = vtk.vtkMatrix4x4()
      m = sliceNode.GetXYToRAS()
      rasToXY.DeepCopy(m)
      rasToXY.Invert()

      # TODO: The current logic only supports rulers from 1mm to 10cm
      # add support for other ranges.
      import math
      scalingFactor = math.sqrt( rasToXY.GetElement(0,0)**2 +
          rasToXY.GetElement(0,1)**2 +rasToXY.GetElement(0,2) **2 )

      if scalingFactor != 0:
        rulerArea = viewWidth/scalingFactor/4
      else:
        rulerArea = viewWidth/4

      #if self.rulerEnabled and \
      if viewWidth > self.minimumWidthForRuler and 0.5 < rulerArea < 500 and NUMPY_AVAILABLE:
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

        textActor = self.rulerTextActors[sliceViewName]
        textActor.SetInput(scalingFactorString)
        textProperty = textActor.GetTextProperty()
        # set font size
        textProperty.SetFontSize(self.fontSize)
        # set font family
        if self.fontFamily == 'Times':
          textProperty.SetFontFamilyToTimes()
        else:
          textProperty.SetFontFamilyToArial()
        # set ruler text actor position
        textActor.SetDisplayPosition(int((viewWidth+RASRulerSize*scalingFactor)/2)+10,5)

        renderer.AddActor2D(self.rulerActors[sliceViewName])
        renderer.RemoveActor2D(textActor)
        renderer.AddActor2D(textActor)

  def modifyScalarBar(self, sliceLogic):
    sliceCompositeNode = sliceLogic.GetSliceCompositeNode()
    if not sliceCompositeNode:
      return

    # Get the layers
    backgroundLayer = sliceLogic.GetBackgroundLayer()
    foregroundLayer = sliceLogic.GetForegroundLayer()

    # Get slice view name
    sliceViewName = backgroundLayer.GetSliceNode().GetLayoutName()

    renderer = self.renderers[sliceViewName]
    orientationRenderer = self.orientationMarkerRenderers[sliceViewName]
    if self.sliceViews[sliceViewName]:
      scalarBar = self.scalarBars[sliceViewName]
      # Font
      scalarBarTextProperty = scalarBar.GetLabelTextProperty()
      scalarBarTextProperty.ItalicOff()
      scalarBarTextProperty.BoldOff()
      if self.fontFamily == 'Times':
        scalarBarTextProperty.SetFontFamilyToTimes()
      else:
        scalarBarTextProperty.SetFontFamilyToArial()

      scalarBar.SetTextPositionToPrecedeScalarBar()
      if self.hasVTKPVScalarBarActor:
        scalarBar.SetRangeLabelFormat(self.rangeLabelFormat)
        scalarBar.SetAddRangeAnnotations(0)
      else:
        scalarBar.SetMaximumWidthInPixels(50)

      renderer.SetViewport(0,0,1,1)
      renderer.SetLayer(0)
      if orientationRenderer is not None:
        orientationRenderer.SetLayer(1)
      #renderWindow = renderer.GetRenderWindow()
      #interactor = renderWindow.GetInteractor()

      # create the scalarBarWidget
      #scalarBarWidget = self.scalarBarWidgets[sliceViewName]
      #scalarBarWidget.SetInteractor(interactor)
      renderer.AddActor(self.scalarBars[sliceViewName])

      ## Adjusting the positions
      if self.orientationMarkerEnabled:
        scalarBar.SetPosition(0.8,0.3)
        scalarBar.SetPosition2(0.17,0.7)
      else:
        scalarBar.SetPosition(0.8,0.1)
        scalarBar.SetPosition2(0.17,0.8)

      # Get the volumes
      backgroundVolume = backgroundLayer.GetVolumeNode()
      foregroundVolume = foregroundLayer.GetVolumeNode()

      if (backgroundVolume is not None and self.scalarBarSelectedLayer == 'background'):
        self.updateScalarBarRange(sliceLogic, backgroundVolume, scalarBar, self.scalarBarSelectedLayer)
        renderer.AddActor(scalarBar)
        #scalarBarWidget.On()
      elif (foregroundVolume is not None and self.scalarBarSelectedLayer == 'foreground'):
        self.updateScalarBarRange(sliceLogic, foregroundVolume, scalarBar, self.scalarBarSelectedLayer)
        renderer.AddActor(scalarBar)
        #scalarBarWidget.On()
      else:
        renderer.RemoveActor(scalarBar)
        #scalarBarWidget.Off()

  def makeAnnotationText(self, sliceLogic):
    self.resetTexts()
    sliceCompositeNode = sliceLogic.GetSliceCompositeNode()
    if not sliceCompositeNode:
      return

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
    if not sliceNode:
      return
    sliceViewName = sliceNode.GetLayoutName()

    if self.sliceViews[sliceViewName]:
      #
      # Update slice corner annotations
      #
      # Case I: Both background and foregraound
      if ( backgroundVolume is not None and foregroundVolume is not None):
        foregroundOpacity = sliceCompositeNode.GetForegroundOpacity()
        backgroundVolumeName = backgroundVolume.GetName()
        foregroundVolumeName = foregroundVolume.GetName()
        self.cornerTexts[0]['3-Background']['text'] = 'B: ' + backgroundVolumeName
        self.cornerTexts[0]['2-Foreground']['text'] = 'F: ' + foregroundVolumeName +  ' (' + str(
                 "%d"%(foregroundOpacity*100)) + '%)'

        bgUids = backgroundVolume.GetAttribute('DICOM.instanceUIDs')
        fgUids = foregroundVolume.GetAttribute('DICOM.instanceUIDs')
        if (bgUids and fgUids):
          bgUid = bgUids.partition(' ')[0]
          fgUid = fgUids.partition(' ')[0]
          self.dicomVolumeNode = 1
          self.makeDicomAnnotation(bgUid,fgUid,sliceViewName)
        elif (bgUids and self.backgroundDICOMAnnotationsPersistence):
          uid = bgUids.partition(' ')[0]
          self.dicomVolumeNode = 1
          self.makeDicomAnnotation(uid,None,sliceViewName)
        else:
          for key in self.cornerTexts[2]:
            self.cornerTexts[2][key]['text'] = ''
          self.dicomVolumeNode = 0

      # Case II: Only background
      elif (backgroundVolume is not None):
        backgroundVolumeName = backgroundVolume.GetName()
        if self.bottomLeft:
          self.cornerTexts[0]['3-Background']['text'] = 'B: ' + backgroundVolumeName

        uids = backgroundVolume.GetAttribute('DICOM.instanceUIDs')
        if uids:
          uid = uids.partition(' ')[0]
          self.makeDicomAnnotation(uid,None,sliceViewName)
          self.dicomVolumeNode = 1
        else:
          self.dicomVolumeNode = 0

      # Case III: Only foreground
      elif (foregroundVolume is not None):
        if self.bottomLeft:
          foregroundVolumeName = foregroundVolume.GetName()
          self.cornerTexts[0]['2-Foreground']['text'] = 'F: ' + foregroundVolumeName

        uids = foregroundVolume.GetAttribute('DICOM.instanceUIDs')
        if uids:
          uid = uids.partition(' ')[0]
          # passed UID as bg
          self.makeDicomAnnotation(uid,None,sliceViewName)
          self.dicomVolumeNode = 1
        else:
          self.dicomVolumeNode = 0

      if (labelVolume is not None):
        labelOpacity = sliceCompositeNode.GetLabelOpacity()
        labelVolumeName = labelVolume.GetName()
        self.cornerTexts[0]['1-Label']['text'] = 'L: ' + labelVolumeName + ' (' + str(
                 "%d"%(labelOpacity*100)) + '%)'

      self.drawCornerAnnotations(sliceViewName)

  @staticmethod
  def updateScalarBarRange(sliceLogic, volumeNode, scalarBar, selectedLayer):
    vdn = volumeNode.GetDisplayNode()
    if vdn:
      vcn = vdn.GetColorNode()
      if vcn is None:
        return
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
      lut2.SetRange(level-width/2,level+width/2)
      scalarBar.SetLookupTable(lut2)

  def makeDicomAnnotation(self,bgUid,fgUid,sliceViewName):
    # Do not attempt to retrieve dicom values if no local database exists
    if not slicer.dicomDatabase:
      return
    viewHeight = self.sliceViews[sliceViewName].height
    if fgUid is not None and bgUid is not None:
      backgroundDicomDic = self.extractDICOMValues(bgUid)
      foregroundDicomDic = self.extractDICOMValues(fgUid)
      # check if background and foreground are from different patients
      # and remove the annotations

      if self.topLeft and viewHeight > 150:
        if backgroundDicomDic['Patient Name'] != foregroundDicomDic['Patient Name'
            ] or backgroundDicomDic['Patient ID'] != foregroundDicomDic['Patient ID'
              ] or backgroundDicomDic['Patient Birth Date'] != foregroundDicomDic['Patient Birth Date']:
              for key in self.cornerTexts[2]:
                self.cornerTexts[2][key]['text'] = ''
        else:
          if self.cornerTexts[2].has_key('1-PatientName'):
            self.cornerTexts[2]['1-PatientName']['text'] = backgroundDicomDic['Patient Name'].replace('^',', ')
          if self.cornerTexts[2].has_key('2-PatientID'):
            self.cornerTexts[2]['2-PatientID']['text'] = 'ID: ' + backgroundDicomDic['Patient ID']
          backgroundDicomDic['Patient Birth Date'] = self.formatDICOMDate(backgroundDicomDic['Patient Birth Date'])
          if self.cornerTexts[2].has_key('3-PatientInfo'):
            self.cornerTexts[2]['3-PatientInfo']['text'] = self.makePatientInfo(backgroundDicomDic)

          if (backgroundDicomDic['Series Date'] != foregroundDicomDic['Series Date']):
            if self.cornerTexts[2].has_key('4-Bg-SeriesDate'):
              self.cornerTexts[2]['4-Bg-SeriesDate']['text'] = 'B: ' + self.formatDICOMDate(backgroundDicomDic['Series Date'])
            if self.cornerTexts[2].has_key('5-Fg-SeriesDate'):
              self.cornerTexts[2]['5-Fg-SeriesDate']['text'] = 'F: ' + self.formatDICOMDate(foregroundDicomDic['Series Date'])
          else:
            if self.cornerTexts[2].has_key('4-Bg-SeriesDate'):
              self.cornerTexts[2]['4-Bg-SeriesDate']['text'] =  self.formatDICOMDate(backgroundDicomDic['Series Date'])

          if (backgroundDicomDic['Series Time'] != foregroundDicomDic['Series Time']):
            if self.cornerTexts[2].has_key('6-Bg-SeriesTime'):
              self.cornerTexts[2]['6-Bg-SeriesTime']['text'] = 'B: ' + self.formatDICOMTime(backgroundDicomDic['Series Time'])
            if self.cornerTexts[2].has_key('7-Fg-SeriesTime'):
              self.cornerTexts[2]['7-Fg-SeriesTime']['text'] = 'F: ' + self.formatDICOMTime(foregroundDicomDic['Series Time'])
          else:
            if self.cornerTexts[2].has_key('6-Bg-SeriesTime'):
              self.cornerTexts[2]['6-Bg-SeriesTime']['text'] = self.formatDICOMTime(backgroundDicomDic['Series Time'])

          if (backgroundDicomDic['Series Description'] != foregroundDicomDic['Series Description']):
            if self.cornerTexts[2].has_key('8-Bg-SeriesDescription'):
              self.cornerTexts[2]['8-Bg-SeriesDescription']['text'] = 'B: ' + backgroundDicomDic['Series Description']
            if self.cornerTexts[2].has_key('9-Fg-SeriesDescription'):
              self.cornerTexts[2]['9-Fg-SeriesDescription']['text'] = 'F: ' + foregroundDicomDic['Series Description']
          else:
            if self.cornerTexts[2].has_key('8-Bg-SeriesDescription'):
              self.cornerTexts[2]['8-Bg-SeriesDescription']['text'] = backgroundDicomDic['Series Description']

    # Only Background or Only Foreground
    else:
      uid = bgUid
      dicomDic = self.extractDICOMValues(uid)

      if self.topLeft and viewHeight > 150:
        self.cornerTexts[2]['1-PatientName']['text'] = dicomDic['Patient Name'].replace('^',', ')
        self.cornerTexts[2]['2-PatientID']['text'] = 'ID: ' + dicomDic ['Patient ID']
        dicomDic['Patient Birth Date'] = self.formatDICOMDate(dicomDic['Patient Birth Date'])
        self.cornerTexts[2]['3-PatientInfo']['text'] = self.makePatientInfo(dicomDic)
        self.cornerTexts[2]['4-Bg-SeriesDate']['text'] = self.formatDICOMDate(dicomDic['Series Date'])
        self.cornerTexts[2]['6-Bg-SeriesTime']['text'] = self.formatDICOMTime(dicomDic['Series Time'])
        self.cornerTexts[2]['8-Bg-SeriesDescription']['text'] = dicomDic['Series Description']

      # top right corner annotation would be hidden if scalarBar is on and
      # view height is less than 260 pixels
      if (self.topRight and (not self.scalarBarEnabled)):
        self.cornerTexts[3]['1-Institution-Name']['text'] = dicomDic['Institution Name']
        self.cornerTexts[3]['2-Referring-Phisycian']['text'] = dicomDic['Referring Physician Name'].replace('^',', ')
        self.cornerTexts[3]['3-Manufacturer']['text'] = dicomDic['Manufacturer']
        self.cornerTexts[3]['4-Model']['text'] = dicomDic['Model']
        self.cornerTexts[3]['5-Patient-Position']['text'] = dicomDic['Patient Position']
        modality = dicomDic['Modality']
        if modality == 'MR':
         self.cornerTexts[3]['6-TR']['text']  = 'TR ' + dicomDic['Repetition Time']
         self.cornerTexts[3]['7-TE']['text'] = 'TE ' + dicomDic['Echo Time']

  @staticmethod
  def makePatientInfo(dicomDic):
    # This will give an string of patient's birth date,
    # patient's age and sex
    patientInfo = dicomDic['Patient Birth Date'
          ] + ', ' + dicomDic['Patient Age'
              ] + ', ' + dicomDic['Patient Sex']
    return patientInfo

  @staticmethod
  def formatDICOMDate(date):
    standardDate = ''
    if date != '':
      date = date.rstrip()
      # convert to ISO 8601 Date format
      standardDate = date[:4] + '-' + date[4:6]+ '-' + date[6:]
    return standardDate

  @staticmethod
  def formatDICOMTime(time):
    if time == '':
      # time field is empty
      return ''
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

  @staticmethod
  def fitText(text,textSize):
    if len(text) > textSize:
      preSize = int(textSize/2)
      postSize = preSize - 3
      text = text[:preSize] + "..." + text[-postSize:]
    return text

  def drawCornerAnnotations(self, sliceViewName):
    if not self.sliceViewAnnotationsEnabled:
      return
    # Auto-Adjust
    # adjust maximum text length based on fontsize and view width
    viewWidth = self.sliceViews[sliceViewName].width
    self.maximumTextLength = int((viewWidth - 40) / self.fontSize)

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
      sliceCornerAnnotation = self.sliceViews[sliceViewName].cornerAnnotation()
      # encode to avoid 'unicode conversion error' for patient names containing international characters
      cornerAnnotation = slicer.util.toVTKString(cornerAnnotation)
      sliceCornerAnnotation.SetText(i, cornerAnnotation)
      textProperty = sliceCornerAnnotation.GetTextProperty()
      textProperty.SetShadow(1)

    self.sliceViews[sliceViewName].scheduleRender()

  def resetTexts(self):
    for i, cornerText in enumerate(self.cornerTexts):
      for key in cornerText.keys():
        self.cornerTexts[i][key]['text'] = ''

  @staticmethod
  def extractDICOMValues(uid):
    p ={}
    tags = {
    "0008,0021": "Series Date",
    "0008,0031": "Series Time",
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
