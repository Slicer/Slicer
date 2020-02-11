import vtk
import qt
import slicer

class SegmentStatisticsPluginBase(object):
  """Base class for statistics plugins operating on segments.
  Derived classes should specify: self.name, self.keys, self.defaultKeys
  and implement: computeStatistics, getMeasurementInfo
  """

  @staticmethod
  def createCodedEntry(codeValue, codingScheme, codeMeaning, returnAsString=False):
    """Create a coded entry and return as string or vtkCodedEntry"""
    entry = slicer.vtkCodedEntry()
    entry.SetValueSchemeMeaning(codeValue, codingScheme, codeMeaning)
    return entry if not returnAsString else entry.GetAsString()

  @staticmethod
  def createMeasurementInfo(name, description, units, quantityDicomCode=None, unitsDicomCode=None,
                            measurementMethodDicomCode=None, derivationDicomCode=None, componentNames=None):
    """Utility method to create measurement information"""
    info = {
      "name": name,
      "description": description,
      "units": units
      }
    if componentNames:
      info["componentNames"] = componentNames
    if quantityDicomCode:
      info["DICOM.QuantityCode"] = quantityDicomCode
    if unitsDicomCode:
      info["DICOM.UnitsCode"] = unitsDicomCode
    if measurementMethodDicomCode:
      info["DICOM.MeasurementMethodCode"] = measurementMethodDicomCode
    if derivationDicomCode:
      info["DICOM.DerivationCode"] = derivationDicomCode
    return info

  def __init__(self):
    #: name of the statistics plugin
    self.name = ""
    #: keys for all supported measurements
    self.keys = []
    #: measurements that will be calculated by default
    self.defaultKeys = []
    self.requestedKeysCheckboxes = {}
    self.parameterNode = None
    self.parameterNodeObserver = None

  def __del__(self):
    if self.parameterNode and self.parameterNodeObserver:
      self.parameterNode.RemoveObserver(self.parameterNodeObserver)

  def computeStatistics(self, segmentID):
    """Compute measurements for requested keys on the given segment and return
    as dictionary mapping key's to measurement results
    """
    pass

  def getMeasurementInfo(self, key):
    """Get information (name, description, units, ...) about the measurement for the given key.
    Utilize createMeasurementInfo() to create the dictionary containing the measurement information.
    Measurement information should contain at least name, description, and units.
    DICOM codes should be provided where possible.
    """
    if key not in self.keys:
      return None
    return createMeasurementInfo(key, key, None)

  def setDefaultParameters(self, parameterNode, overwriteExisting=False):
    # enable plugin
    pluginName = self.__class__.__name__
    parameter = pluginName+'.enabled'
    if not parameterNode.GetParameter(parameter):
      parameterNode.SetParameter(parameter, str(True))
    # enable all default keys
    for key in self.keys:
      parameter = self.toLongKey(key)+'.enabled'
      if not parameterNode.GetParameter(parameter) or overwriteExisting:
        parameterNode.SetParameter(parameter, str(key in self.defaultKeys))

  def getRequestedKeys(self):
    if not self.parameterNode:
      return ()
    requestedKeys = [key for key in self.keys if self.parameterNode.GetParameter(self.toLongKey(key)+'.enabled')=='True']
    return requestedKeys

  def toLongKey(self, key):
    # add name of plugin as a prefix for use outside of plugin
    pluginName = self.__class__.__name__
    return pluginName+'.'+key

  def toShortKey(self, key):
    # remove prefix used outside of plugin
    pluginName = self.__class__.__name__
    return key[len(pluginName)+1:] if key.startswith(pluginName+'.') else ''

  def setParameterNode(self, parameterNode):
    if self.parameterNode==parameterNode:
      return
    if self.parameterNode and self.parameterNodeObserver:
      self.parameterNode.RemoveObserver(self.parameterNodeObserver)
    self.parameterNode = parameterNode
    if self.parameterNode:
      self.setDefaultParameters(self.parameterNode)
      self.parameterNodeObserver = self.parameterNode.AddObserver(vtk.vtkCommand.ModifiedEvent,
                                                                  self.updateGuiFromParameterNode)
    self.createDefaultOptionsWidget()
    self.updateGuiFromParameterNode()

  def getParameterNode(self):
    return self.parameterNode

  def createDefaultOptionsWidget(self):
    # create list of checkboxes that allow selection of requested keys
    self.optionsWidget = qt.QWidget()
    form = qt.QFormLayout(self.optionsWidget)

    # checkbox to enable/disable plugin
    self.pluginCheckbox = qt.QCheckBox(self.name+" plugin enabled")
    self.pluginCheckbox.checked = True
    self.pluginCheckbox.connect('stateChanged(int)', self.updateParameterNodeFromGui)
    form.addRow(self.pluginCheckbox)

    # select all/none/default buttons
    selectAllNoneFrame = qt.QFrame(self.optionsWidget)
    selectAllNoneFrame.setLayout(qt.QHBoxLayout())
    selectAllNoneFrame.layout().setSpacing(0)
    selectAllNoneFrame.layout().setMargin(0)
    selectAllNoneFrame.layout().addWidget(qt.QLabel("Select measurements: ",self.optionsWidget))
    selectAllButton = qt.QPushButton('all',self.optionsWidget)
    selectAllNoneFrame.layout().addWidget(selectAllButton)
    selectAllButton.connect('clicked()', self.requestAll)
    selectNoneButton = qt.QPushButton('none',self.optionsWidget)
    selectAllNoneFrame.layout().addWidget(selectNoneButton)
    selectNoneButton.connect('clicked()', self.requestNone)
    selectDefaultButton = qt.QPushButton('default',self.optionsWidget)
    selectAllNoneFrame.layout().addWidget(selectDefaultButton)
    selectDefaultButton.connect('clicked()', self.requestDefault)
    form.addRow(selectAllNoneFrame)

    # checkboxes for individual keys
    self.requestedKeysCheckboxes = {}
    requestedKeys = self.getRequestedKeys()
    for key in self.keys:
      label = key
      tooltip = "key: "+key
      info = self.getMeasurementInfo(key)
      if info and ("name" in info or "description" in info):
        label = info["name"] if "name" in info else info["description"]
        if "name" in info: tooltip += "\nname: " + str(info["name"])
        if "description" in info: tooltip += "\ndescription: " + str(info["description"])
        if "units" in info: tooltip += "\nunits: " + (str(info["units"]) if info["units"] else "n/a")
      checkbox = qt.QCheckBox(label,self.optionsWidget)
      checkbox.checked = key in requestedKeys
      checkbox.setToolTip(tooltip)
      form.addRow(checkbox)
      self.requestedKeysCheckboxes[key] = checkbox
      checkbox.connect('stateChanged(int)', self.updateParameterNodeFromGui)

  def updateGuiFromParameterNode(self, caller=None, event=None):
    if not self.parameterNode:
      return
    pluginName = self.__class__.__name__
    isEnabled = self.parameterNode.GetParameter(pluginName+'.enabled')!='False'
    self.pluginCheckbox.checked = isEnabled
    for (key, checkbox) in self.requestedKeysCheckboxes.items():
      parameter = self.toLongKey(key)+'.enabled'
      value = self.parameterNode.GetParameter(parameter)=='True'
      if checkbox.checked!=value:
        previousState = checkbox.blockSignals(True)
        checkbox.checked = value
        checkbox.blockSignals(previousState)
      if checkbox.enabled!=isEnabled:
        previousState = checkbox.blockSignals(True)
        checkbox.enabled = isEnabled
        checkbox.blockSignals(previousState)

  def updateParameterNodeFromGui(self):
    if not self.parameterNode:
      return
    pluginName = self.__class__.__name__
    self.parameterNode.SetParameter(pluginName+'.enabled', str(self.pluginCheckbox.checked))
    for (key, checkbox) in self.requestedKeysCheckboxes.items():
      parameter = self.toLongKey(key)+'.enabled'
      newValue = str(checkbox.checked)
      currentValue = self.parameterNode.GetParameter(parameter)
      if not currentValue or currentValue!=newValue:
        self.parameterNode.SetParameter(parameter, newValue)

  def requestAll(self):
    if not self.parameterNode:
      return
    for (key, checkbox) in self.requestedKeysCheckboxes.items():
      parameter = self.toLongKey(key)+'.enabled'
      newValue = str(True)
      currentValue = self.parameterNode.GetParameter(parameter)
      if not currentValue or currentValue!=newValue:
        self.parameterNode.SetParameter(parameter, newValue)

  def requestNone(self):
    if not self.parameterNode:
      return
    for (key, checkbox) in self.requestedKeysCheckboxes.items():
      parameter = self.toLongKey(key)+'.enabled'
      newValue = str(False)
      currentValue = self.parameterNode.GetParameter(parameter)
      if not currentValue or currentValue!=newValue:
        self.parameterNode.SetParameter(parameter, newValue)

  def requestDefault(self):
    if not self.parameterNode:
      return
    self.setDefaultParameters(self.parameterNode, overwriteExisting=True)
