import logging

import ctk
import qt
import vtk

import slicer
from slicer.ScriptedLoadableModule import *
from slicer.i18n import tr as _
from slicer.i18n import translate

from SegmentStatisticsPlugins import *


class SegmentStatistics(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = _("Segment Statistics")
        self.parent.categories = [translate("qSlicerAbstractCoreModule", "Quantification")]
        self.parent.dependencies = ["SubjectHierarchy"]
        self.parent.contributors = ["Andras Lasso (PerkLab), Christian Bauer (University of Iowa), Steve Pieper (Isomics)"]
        self.parent.helpText = _("""
Use this module to calculate counts and volumes for segments plus statistics on the grayscale background volume.
Computed fields:
Segment labelmap statistics (LM): voxel count, volume mm3, volume cm3.
Requires segment labelmap representation.
Scalar volume statistics (SV): voxel count, volume mm3, volume cm3 (where segments overlap scalar volume),
min, max, mean, stdev (intensity statistics).
Requires segment labelmap representation and selection of a scalar volume
Closed surface statistics (CS): surface mm2, volume mm3, volume cm3 (computed from closed surface).
Requires segment closed surface representation.
""")
        self.parent.helpText += parent.defaultDocumentationLink
        self.parent.acknowledgementText = _("""
Supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See https://www.slicer.org for details.
""")

    def setup(self):
        # Register subject hierarchy plugin
        import SubjectHierarchyPlugins

        scriptedPlugin = slicer.qSlicerSubjectHierarchyScriptedPlugin(None)
        scriptedPlugin.setPythonSource(SubjectHierarchyPlugins.SegmentStatisticsSubjectHierarchyPlugin.filePath)


class SegmentStatisticsWidget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        self.logic = SegmentStatisticsLogic()
        self.grayscaleNode = None
        self.labelNode = None
        self.parameterNode = None
        self.parameterNodeObserver = None

        # Instantiate and connect widgets ...
        #

        # Parameter set selector
        self.parameterNodeSelector = slicer.qMRMLNodeComboBox()
        self.parameterNodeSelector.nodeTypes = ["vtkMRMLScriptedModuleNode"]
        self.parameterNodeSelector.addAttribute("vtkMRMLScriptedModuleNode", "ModuleName", "SegmentStatistics")
        self.parameterNodeSelector.selectNodeUponCreation = True
        self.parameterNodeSelector.addEnabled = True
        self.parameterNodeSelector.renameEnabled = True
        self.parameterNodeSelector.removeEnabled = True
        self.parameterNodeSelector.noneEnabled = False
        self.parameterNodeSelector.showHidden = True
        self.parameterNodeSelector.showChildNodeTypes = False
        self.parameterNodeSelector.baseName = "SegmentStatistics"
        self.parameterNodeSelector.setMRMLScene(slicer.mrmlScene)
        self.parameterNodeSelector.setToolTip(_("Pick parameter set"))
        self.layout.addWidget(self.parameterNodeSelector)

        # Inputs
        inputsCollapsibleButton = ctk.ctkCollapsibleButton()
        inputsCollapsibleButton.text = _("Inputs")
        self.layout.addWidget(inputsCollapsibleButton)
        inputsFormLayout = qt.QFormLayout(inputsCollapsibleButton)

        # Segmentation selector
        self.segmentationSelector = slicer.qMRMLNodeComboBox()
        self.segmentationSelector.nodeTypes = ["vtkMRMLSegmentationNode"]
        self.segmentationSelector.addEnabled = False
        self.segmentationSelector.removeEnabled = True
        self.segmentationSelector.renameEnabled = True
        self.segmentationSelector.setMRMLScene(slicer.mrmlScene)
        self.segmentationSelector.setToolTip(_("Pick the segmentation to compute statistics for"))
        inputsFormLayout.addRow(_("Segmentation:"), self.segmentationSelector)

        # Scalar volume selector
        self.scalarSelector = slicer.qMRMLNodeComboBox()
        self.scalarSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
        self.scalarSelector.addEnabled = False
        self.scalarSelector.removeEnabled = True
        self.scalarSelector.renameEnabled = True
        self.scalarSelector.noneEnabled = True
        self.scalarSelector.showChildNodeTypes = False
        self.scalarSelector.setMRMLScene(slicer.mrmlScene)
        self.scalarSelector.setToolTip(_("Select the scalar volume for intensity statistics calculations"))
        inputsFormLayout.addRow(_("Scalar volume:"), self.scalarSelector)

        # Output table selector
        outputCollapsibleButton = ctk.ctkCollapsibleButton()
        outputCollapsibleButton.text = _("Output")
        self.layout.addWidget(outputCollapsibleButton)
        outputFormLayout = qt.QFormLayout(outputCollapsibleButton)

        self.outputTableSelector = slicer.qMRMLNodeComboBox()
        self.outputTableSelector.noneDisplay = _("Create new table")
        self.outputTableSelector.setMRMLScene(slicer.mrmlScene)
        self.outputTableSelector.nodeTypes = ["vtkMRMLTableNode"]
        self.outputTableSelector.addEnabled = True
        self.outputTableSelector.selectNodeUponCreation = True
        self.outputTableSelector.renameEnabled = True
        self.outputTableSelector.removeEnabled = True
        self.outputTableSelector.noneEnabled = True
        self.outputTableSelector.setToolTip(_("Select the table where statistics will be saved into"))
        self.outputTableSelector.setCurrentNode(None)

        outputFormLayout.addRow(_("Output table:"), self.outputTableSelector)

        # Parameter set
        parametersCollapsibleButton = ctk.ctkCollapsibleButton()
        parametersCollapsibleButton.text = _("Advanced")
        parametersCollapsibleButton.collapsed = True
        self.layout.addWidget(parametersCollapsibleButton)
        self.parametersLayout = qt.QFormLayout(parametersCollapsibleButton)

        # Edit parameter set button to open SegmentStatisticsParameterEditorDialog
        # Note: we add the plugins' option widgets to the module widget instead of using the editor dialog
        # self.editParametersButton = qt.QPushButton("Edit Parameter Set")
        # self.editParametersButton.toolTip = "Editor Statistics Plugin Parameter Set."
        # self.parametersLayout.addRow(self.editParametersButton)
        # self.editParametersButton.connect('clicked()', self.onEditParameters)
        # add caclulator's option widgets
        self.addPluginOptionWidgets()

        # Apply Button
        self.applyButton = qt.QPushButton(_("Apply"))
        self.applyButton.toolTip = _("Calculate Statistics.")
        self.applyButton.enabled = False
        self.parent.layout().addWidget(self.applyButton)

        # Add vertical spacer
        self.parent.layout().addStretch(1)

        # connections
        self.applyButton.connect("clicked()", self.onApply)
        self.scalarSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeSelectionChanged)
        self.segmentationSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeSelectionChanged)
        self.outputTableSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeSelectionChanged)
        self.parameterNodeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeSelectionChanged)
        self.parameterNodeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onParameterSetSelected)

        self.parameterNodeSelector.setCurrentNode(self.logic.getParameterNode())
        self.onNodeSelectionChanged()
        self.onParameterSetSelected()

    def enter(self):
        """Runs whenever the module is reopened"""
        if self.parameterNodeSelector.currentNode() is None:
            parameterNode = self.logic.getParameterNode()
            slicer.mrmlScene.AddNode(parameterNode)
            self.parameterNodeSelector.setCurrentNode(parameterNode)
        if self.segmentationSelector.currentNode() is None:
            segmentationNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLSegmentationNode")
            self.segmentationSelector.setCurrentNode(segmentationNode)

    def cleanup(self):
        if self.parameterNode and self.parameterNodeObserver:
            self.parameterNode.RemoveObserver(self.parameterNodeObserver)

    def onNodeSelectionChanged(self):
        self.applyButton.enabled = (self.segmentationSelector.currentNode() is not None and
                                    self.parameterNodeSelector.currentNode() is not None)
        if self.segmentationSelector.currentNode():
            self.outputTableSelector.baseName = self.segmentationSelector.currentNode().GetName() + " statistics"

    def onApply(self):
        """Calculate the label statistics"""

        with slicer.util.tryWithErrorDisplay(_("Failed to compute results."), waitCursor=True):
            if not self.outputTableSelector.currentNode():
                newTable = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
                self.outputTableSelector.setCurrentNode(newTable)
            # Lock GUI
            self.applyButton.text = _("Working...")
            self.applyButton.setEnabled(False)
            slicer.app.processEvents()
            # set up parameters for computation
            self.logic.getParameterNode().SetParameter("Segmentation", self.segmentationSelector.currentNode().GetID())
            if self.scalarSelector.currentNode():
                self.logic.getParameterNode().SetParameter("ScalarVolume", self.scalarSelector.currentNode().GetID())
            else:
                self.logic.getParameterNode().UnsetParameter("ScalarVolume")
            self.logic.getParameterNode().SetParameter("MeasurementsTable", self.outputTableSelector.currentNode().GetID())
            # Compute statistics
            self.logic.computeStatistics()
            self.logic.exportToTable(self.outputTableSelector.currentNode())
            self.logic.showTable(self.outputTableSelector.currentNode())

        # Unlock GUI
        self.applyButton.setEnabled(True)
        self.applyButton.text = _("Apply")

    def onEditParameters(self, pluginName=None):
        """Open dialog box to edit plugin's parameters"""
        if self.parameterNodeSelector.currentNode():
            SegmentStatisticsParameterEditorDialog.editParameters(self.parameterNodeSelector.currentNode(), pluginName)

    def addPluginOptionWidgets(self):
        self.pluginEnabledCheckboxes = {}
        self.parametersLayout.addRow(qt.QLabel(_("Enabled segment statistics plugins:")))
        for plugin in self.logic.plugins:
            checkbox = qt.QCheckBox(_("{pluginName} Statistics").format(pluginName=plugin.title))
            checkbox.checked = True
            checkbox.connect("stateChanged(int)", self.updateParameterNodeFromGui)
            optionButton = qt.QPushButton(_("Options"))

            from functools import partial

            optionButton.connect("clicked()", partial(self.onEditParameters, plugin.name))
            editWidget = qt.QWidget()
            editWidget.setLayout(qt.QHBoxLayout())
            editWidget.layout().margin = 0
            editWidget.layout().addWidget(checkbox, 0)
            editWidget.layout().addStretch(1)
            editWidget.layout().addWidget(optionButton, 0)
            self.pluginEnabledCheckboxes[plugin.name] = checkbox
            self.parametersLayout.addRow(editWidget)
        # embed widgets for editing plugin' parameters
        # for plugin in self.logic.plugins:
        #  pluginOptionsCollapsibleButton = ctk.ctkCollapsibleGroupBox()
        #  pluginOptionsCollapsibleButton.setTitle( plugin.name )
        #  pluginOptionsFormLayout = qt.QFormLayout(pluginOptionsCollapsibleButton)
        #  pluginOptionsFormLayout.addRow(plugin.optionsWidget)
        #  self.parametersLayout.addRow(pluginOptionsCollapsibleButton)

    def onParameterSetSelected(self):
        if self.parameterNode and self.parameterNodeObserver:
            self.parameterNode.RemoveObserver(self.parameterNodeObserver)
        self.parameterNode = self.parameterNodeSelector.currentNode()
        if self.parameterNode:
            self.logic.setParameterNode(self.parameterNode)
            self.parameterNodeObserver = self.parameterNode.AddObserver(vtk.vtkCommand.ModifiedEvent,
                                                                        self.updateGuiFromParameterNode)
        self.updateGuiFromParameterNode()

    def updateGuiFromParameterNode(self, caller=None, event=None):
        if not self.parameterNode:
            return
        for plugin in self.logic.plugins:
            pluginName = plugin.__class__.__name__
            parameter = pluginName + ".enabled"
            checkbox = self.pluginEnabledCheckboxes[plugin.name]
            value = self.parameterNode.GetParameter(parameter) == "True"
            if checkbox.checked != value:
                previousState = checkbox.blockSignals(True)
                checkbox.checked = value
                checkbox.blockSignals(previousState)

    def updateParameterNodeFromGui(self):
        if not self.parameterNode:
            return
        for plugin in self.logic.plugins:
            pluginName = plugin.__class__.__name__
            parameter = pluginName + ".enabled"
            checkbox = self.pluginEnabledCheckboxes[plugin.name]
            self.parameterNode.SetParameter(parameter, str(checkbox.checked))


class SegmentStatisticsParameterEditorDialog(qt.QDialog):
    """Dialog to edit parameters of segment statistics plugins.
    Most users will only need to call the static method editParameters(...)
    """

    @staticmethod
    def editParameters(parameterNode, pluginName=None):
        """Executes a modal dialog to edit a segment statistics parameter node if a pluginName is specified, only
        options for this plugin are displayed"
        """
        dialog = SegmentStatisticsParameterEditorDialog(parent=None, parameterNode=parameterNode,
                                                        pluginName=pluginName)
        return dialog.exec_()

    def __init__(self, parent=None, parameterNode=None, pluginName=None):
        super(qt.QDialog, self).__init__(parent)
        self.title = _("Edit Segment Statistics Parameters")
        self.parameterNode = parameterNode
        self.pluginName = pluginName
        self.logic = SegmentStatisticsLogic()  # for access to plugins and editor widgets
        self.logic.setParameterNode(self.parameterNode)
        self.setup()

    def setParameterNode(self, parameterNode):
        """Set the parameter node the dialog will operate on"""
        if parameterNode == self.parameterNode:
            return
        self.parameterNode = parameterNode
        self.logic.setParameterNode(self.parameterNode)

    def setup(self):
        self.setLayout(qt.QVBoxLayout())

        self.descriptionLabel = qt.QLabel(_("Edit segment statistics plugin parameters:"), 0)

        self.doneButton = qt.QPushButton(_("Done"))
        self.doneButton.toolTip = _("Finish editing.")
        doneWidget = qt.QWidget(self)
        doneWidget.setLayout(qt.QHBoxLayout())
        doneWidget.layout().addStretch(1)
        doneWidget.layout().addWidget(self.doneButton, 0)

        parametersScrollArea = qt.QScrollArea(self)
        self.parametersWidget = qt.QWidget(parametersScrollArea)
        self.parametersLayout = qt.QFormLayout(self.parametersWidget)
        self._addPluginOptionWidgets()
        parametersScrollArea.setWidget(self.parametersWidget)
        parametersScrollArea.widgetResizable = True
        parametersScrollArea.setVerticalScrollBarPolicy(qt.Qt.ScrollBarAsNeeded)
        parametersScrollArea.setHorizontalScrollBarPolicy(qt.Qt.ScrollBarAsNeeded)

        self.layout().addWidget(self.descriptionLabel, 0)
        self.layout().addWidget(parametersScrollArea, 1)
        self.layout().addWidget(doneWidget, 0)
        self.doneButton.connect("clicked()", lambda: self.done(1))

    def _addPluginOptionWidgets(self):
        description = _("Edit segment statistics plugin parameters:")
        if self.pluginName:
            description = _("Edit {pluginName} plugin parameters:").format(pluginName=self.pluginName)
        self.descriptionLabel.text = description
        if self.pluginName:
            for plugin in self.logic.plugins:
                if plugin.name == self.pluginName:
                    self.parametersLayout.addRow(plugin.optionsWidget)
        else:
            for plugin in self.logic.plugins:
                pluginOptionsCollapsibleButton = ctk.ctkCollapsibleGroupBox(self.parametersWidget)
                pluginOptionsCollapsibleButton.setTitle(plugin.title)
                pluginOptionsFormLayout = qt.QFormLayout(pluginOptionsCollapsibleButton)
                pluginOptionsFormLayout.addRow(plugin.optionsWidget)
                self.parametersLayout.addRow(pluginOptionsCollapsibleButton)


class SegmentStatisticsLogic(ScriptedLoadableModuleLogic):
    """Implement the logic to calculate label statistics.
    Nodes are passed in as arguments.
    Results are stored as 'statistics' instance variable.
    Additional plugins for computation of other statistical measurements may be registered.
    Uses ScriptedLoadableModuleLogic base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    registeredPlugins = [LabelmapSegmentStatisticsPlugin, ScalarVolumeSegmentStatisticsPlugin,
                         ClosedSurfaceSegmentStatisticsPlugin]

    segmentColumnName = "Segment"
    segmentColumnTitle = _("Segment")

    @staticmethod
    def registerPlugin(plugin):
        """Register a subclass of SegmentStatisticsPluginBase for calculation of additional measurements"""
        if not isinstance(plugin, SegmentStatisticsPluginBase):
            return
        for key in plugin.keys:
            if key.count(".") > 0:
                logging.warning("Plugin keys should not contain extra '.' as it might mix pluginname.measurementkey in "
                                "the parameter node")
        if plugin.__class__ not in SegmentStatisticsLogic.registeredPlugins:
            SegmentStatisticsLogic.registeredPlugins.append(plugin.__class__)
        else:
            logging.warning("SegmentStatisticsLogic.registerPlugin will not register plugin because \
                       another plugin with the same name has already been registered")

    def __init__(self, parent=None):
        ScriptedLoadableModuleLogic.__init__(self, parent)
        self.plugins = [x() for x in SegmentStatisticsLogic.registeredPlugins]

        self.isSingletonParameterNode = False
        self.parameterNode = None

        self.keys = [SegmentStatisticsLogic.segmentColumnName]
        self.notAvailableValueString = ""
        self.reset()

    def getParameterNode(self):
        """Returns the current parameter node and creates one if it doesn't exist yet"""
        if not self.parameterNode:
            self.setParameterNode(ScriptedLoadableModuleLogic.getParameterNode(self))
        return self.parameterNode

    def setParameterNode(self, parameterNode):
        """Set the current parameter node and initialize all unset parameters to their default values"""
        if self.parameterNode == parameterNode:
            return
        self.setDefaultParameters(parameterNode)
        self.parameterNode = parameterNode
        for plugin in self.plugins:
            plugin.setParameterNode(parameterNode)

    def setDefaultParameters(self, parameterNode):
        """Set all plugins to enabled and all plugins' parameters to their default value"""
        for plugin in self.plugins:
            plugin.setDefaultParameters(parameterNode)
        if not parameterNode.GetParameter("visibleSegmentsOnly"):
            parameterNode.SetParameter("visibleSegmentsOnly", str(True))

    def getStatistics(self):
        """Get the calculated statistical measurements"""
        params = self.getParameterNode()
        if not hasattr(params, "statistics"):
            params.statistics = {"SegmentIDs": [], "MeasurementInfo": {}}
        return params.statistics

    def reset(self):
        """Clear all computation results"""
        self.keys = [SegmentStatisticsLogic.segmentColumnName]
        for plugin in self.plugins:
            self.keys += [plugin.toLongKey(k) for k in plugin.keys]
        params = self.getParameterNode()
        params.statistics = {"SegmentIDs": [], "MeasurementInfo": {}}

    def computeStatistics(self):
        """Compute statistical measures for all (visible) segments"""
        self.reset()

        segmentationNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("Segmentation"))
        transformedSegmentationNode = None
        try:
            if not segmentationNode.GetParentTransformNode() is None:
                # Create a temporary segmentation and harden the transform to ensure that the statistics are calculated
                # in world coordinates
                transformedSegmentationNode = slicer.vtkMRMLSegmentationNode()
                transformedSegmentationNode.Copy(segmentationNode)
                transformedSegmentationNode.HideFromEditorsOn()
                slicer.mrmlScene.AddNode(transformedSegmentationNode)
                transformedSegmentationNode.HardenTransform()
                self.getParameterNode().SetParameter("Segmentation", transformedSegmentationNode.GetID())

            # Get segment ID list
            visibleSegmentIds = vtk.vtkStringArray()
            if self.getParameterNode().GetParameter("visibleSegmentsOnly") == "True":
                segmentationNode.GetDisplayNode().GetVisibleSegmentIDs(visibleSegmentIds)
            else:
                segmentationNode.GetSegmentation().GetSegmentIDs(visibleSegmentIds)
            if visibleSegmentIds.GetNumberOfValues() == 0:
                logging.debug("computeStatistics will not return any results: there are no visible segments")

            # update statistics for all segment IDs
            for segmentIndex in range(visibleSegmentIds.GetNumberOfValues()):
                segmentID = visibleSegmentIds.GetValue(segmentIndex)
                self.updateStatisticsForSegment(segmentID)
        finally:
            if transformedSegmentationNode is not None:
                # We made a copy and hardened the segmentation transform
                self.getParameterNode().SetParameter("Segmentation", segmentationNode.GetID())
                slicer.mrmlScene.RemoveNode(transformedSegmentationNode)

    def updateStatisticsForSegment(self, segmentID):
        """
        Update statistical measures for specified segment.
        Note: This will not change or reset measurement results of other segments
        """

        segmentationNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("Segmentation"))

        if not segmentationNode.GetSegmentation().GetSegment(segmentID):
            logging.debug("updateStatisticsForSegment will not update any results because the segment doesn't exist")
            return

        segment = segmentationNode.GetSegmentation().GetSegment(segmentID)
        statistics = self.getStatistics()
        if segmentID not in statistics["SegmentIDs"]:
            statistics["SegmentIDs"].append(segmentID)
        statistics[segmentID, SegmentStatisticsLogic.segmentColumnName] = segment.GetName()

        # apply all enabled plugins
        for plugin in self.plugins:
            pluginName = plugin.__class__.__name__
            if self.getParameterNode().GetParameter(pluginName + ".enabled") == "True":
                stats = plugin.computeStatistics(segmentID)
                for key in stats:
                    statistics[segmentID, pluginName + "." + key] = stats[key]
                    statistics["MeasurementInfo"][pluginName + "." + key] = plugin.getMeasurementInfo(key)

    def getPluginByKey(self, key):
        """Get plugin responsible for obtaining measurement value for given key"""
        for plugin in self.plugins:
            if plugin.toShortKey(key) in plugin.keys:
                return plugin
        return None

    def getMeasurementInfo(self, key):
        """Get information (name, description, units, ...) about the measurement for the given key"""
        plugin = self.getPluginByKey(key)
        if plugin:
            return plugin.getMeasurementInfo(plugin.toShortKey(key))
        return None

    def getStatisticsValueAsString(self, segmentID, key):
        statistics = self.getStatistics()
        if (segmentID, key) in statistics:
            value = statistics[segmentID, key]
            if isinstance(value, float):
                return "%0.3f" % value  # round to 3 decimals
            else:
                return str(value)
        else:
            return self.notAvailableValueString

    def getNonEmptyKeys(self):
        # Fill columns
        statistics = self.getStatistics()
        nonEmptyKeys = []
        for key in self.keys:
            for segmentID in statistics["SegmentIDs"]:
                if (segmentID, key) in statistics:
                    nonEmptyKeys.append(key)
                    break
        return nonEmptyKeys

    @staticmethod
    def makeUnique(names, suffixes, differentiators=None):
        # Add suffix to name if name+differentiator is not unique in the list
        if differentiators:
            fullNames = [f"{name} & {differentiator}" for name, differentiator in zip(names, differentiators)]
        else:
            fullNames = names
        uniqueNames = []
        for i in range(len(names)):
            isUnique = fullNames.count(fullNames[i]) == 1
            if isUnique:
                uniqueNames.append(names[i])
            else:
                uniqueNames.append(names[i]+suffixes[i])
        return uniqueNames

    def getColumnNamesTitles(self, nonEmptyKeysOnly=True):
        """Get unique column header names and titles for each measurement.
        Two maps are returned, from measurement key to column name or title.
        Name is for machine processing, not translated, based on measurement name
        (or terminology entries).
        Title is for display, translatable. It comes from measurement title and units.
        Duplicate names get a suffix (acronym of the measurement plugin) to make them unique.
        """
        keys = self.getNonEmptyKeys() if nonEmptyKeysOnly else self.keys
        # keys = ['Segment', 'LabelmapSegmentStatisticsPlugin.voxel_count',
        # 'LabelmapSegmentStatisticsPlugin.volume_mm3', 'LabelmapSegmentStatisticsPlugin.volume_cm3',
        # 'ScalarVolumeSegmentStatisticsPlugin.voxel_count', 'ScalarVolumeSegmentStatisticsPlugin.volume_mm3',
        # 'ScalarVolumeSegmentStatisticsPlugin.volume_cm3', 'ScalarVolumeSegmentStatisticsPlugin.min',...

        statistics = self.getStatistics()
        # statistics = { 'SegmentIDs': ['Segment_1', 'Segment_2', 'Segment_3'],
        # 'MeasurementInfo': {
        #     'LabelmapSegmentStatisticsPlugin.volume_mm3': {
        #         'name': 'Volume mm3',
        #         'title': 'Terfogat',
        #         'description': 'Terfogat cimketerkep reprezentaciobol szamolva',
        #         'units': 'mm3',
        #         'DICOM.QuantityCode': 'CodeValue:118565006|CodingSchemeDesignator:SCT|CodeMeaning:Volume',
        #         'DICOM.UnitsCode': 'CodeValue:mm3|CodingSchemeDesignator:UCUM|CodeMeaning:mm3'},
        #     'LabelmapSegmentStatisticsPlugin.volume_cm3': {
        #         'name': 'Volume cm3'...

        columnNames = []
        columnTitles = []
        unitsTitles = []
        pluginNameSuffixes = []
        pluginTitleSuffixes = []
        for key in keys:
            name = key
            title = ""
            unitsTitle = ""
            info = statistics["MeasurementInfo"][key] if key in statistics["MeasurementInfo"] else {}
            if info:
                # Units
                if info.get("units"):
                    unitsTitle = info["units"]

                # Column name
                if info.get("name"):
                    name = info["name"]
                else:
                    if info.get("DICOM.DerivationCode"):
                        entry = slicer.vtkCodedEntry()
                        entry.SetFromString(info["DICOM.DerivationCode"])
                        name = entry.GetCodeMeaning()
                    elif info.get("DICOM.QuantityCode"):
                        entry = slicer.vtkCodedEntry()
                        entry.SetFromString(info["DICOM.QuantityCode"])
                        name = entry.GetCodeMeaning()
                    # Append unit to name (use unitsName if present, fall back to unitsTitle)
                    units = ""
                    if info.get("DICOM.UnitsCode"):
                        entry = slicer.vtkCodedEntry()
                        entry.SetFromString(info["DICOM.UnitsCode"])
                        units = entry.GetCodeValue()
                        if len(units) > 0 and units[0] == "[" and units[-1] == "]":
                            units = units[1:-1]
                    if (not units) and unitsTitle:
                        units = unitsTitle  # fall back to use displayable units
                    if units:
                        name += " " + units

                # Column title (translatable)
                if info.get("title"):
                    title = info["title"]

            elif key == SegmentStatisticsLogic.segmentColumnName:
                title = SegmentStatisticsLogic.segmentColumnTitle

            columnNames.append(name)
            columnTitles.append(title)
            unitsTitles.append(unitsTitle)

            # Save suffix generated from acronym of the plugin name (Scalar Volume => SV)
            # that is added to the column name if the column name is not unique within the table.
            plugin = self.getPluginByKey(key)
            if plugin and plugin.name:
                pluginAcronym = "".join(word[0].upper() for word in plugin.name.split())
                pluginNameSuffixes.append(f" ({pluginAcronym})")
            else:
                pluginNameSuffixes.append("")
            if plugin and plugin.title:
                pluginAcronym = "".join(word[0].upper() for word in plugin.title.split())
                pluginTitleSuffixes.append(f" ({pluginAcronym})")
            else:
                pluginTitleSuffixes.append("")

        # Make unique
        columnNames = SegmentStatisticsLogic.makeUnique(columnNames, pluginNameSuffixes)
        columnTitles = SegmentStatisticsLogic.makeUnique(columnTitles, pluginTitleSuffixes, unitsTitles)
        # Make a map from key
        columnNames = {keys[i]: columnNames[i] for i in range(len(keys))}
        columnTitles = {keys[i]: columnTitles[i] for i in range(len(keys))}
        return columnNames, columnTitles

    def exportToTable(self, table, nonEmptyKeysOnly=True):
        """Export statistics to table node"""
        tableWasModified = table.StartModify()
        table.RemoveAllColumns()

        keys = self.getNonEmptyKeys() if nonEmptyKeysOnly else self.keys
        columnNames, columnTitles = self.getColumnNamesTitles(nonEmptyKeysOnly)

        # Define table columns
        statistics = self.getStatistics()
        for key in keys:
            # create table column appropriate for data type; currently supported: float, int, long, string
            measurements = [statistics[segmentID, key] for segmentID in statistics["SegmentIDs"] if
                            (segmentID, key) in statistics]
            if len(measurements) == 0:  # there were not measurements and therefore use the default "string" representation
                col = table.AddColumn()
            elif isinstance(measurements[0], int):
                col = table.AddColumn(vtk.vtkLongArray())
            elif isinstance(measurements[0], float):
                col = table.AddColumn(vtk.vtkDoubleArray())
            elif isinstance(measurements[0], list):
                length = len(measurements[0])
                if length == 0:
                    col = table.AddColumn()
                else:
                    value = measurements[0][0]
                    if isinstance(value, int):
                        array = vtk.vtkLongArray()
                        array.SetNumberOfComponents(length)
                        col = table.AddColumn(array)
                    elif isinstance(value, float):
                        array = vtk.vtkDoubleArray()
                        array.SetNumberOfComponents(length)
                        col = table.AddColumn(array)
                    else:
                        col = table.AddColumn()
            else:  # default
                col = table.AddColumn()
            plugin = self.getPluginByKey(key)
            columnName = columnNames[key]
            columnTitle = columnTitles[key]
            col.SetName(columnName)
            table.SetColumnTitle(columnName, columnTitle)
            if plugin:
                table.SetColumnProperty(columnName, "Plugin", plugin.name)
            measurementInfo = statistics["MeasurementInfo"][key] if key in statistics["MeasurementInfo"] else {}
            if measurementInfo:
                for mik, miv in measurementInfo.items():
                    if mik == "description":
                        table.SetColumnDescription(columnName, str(miv))
                    elif mik == "units":
                        table.SetColumnUnitLabel(columnName, str(miv))
                    elif mik == "componentNames":
                        componentNames = miv
                        array = table.GetTable().GetColumnByName(columnName)
                        componentIndex = 0
                        for componentName in miv:
                            array.SetComponentName(componentIndex, componentName)
                            componentIndex += 1
                    elif mik not in ["name", "title"]:  # name and title are set already
                        table.SetColumnProperty(columnName, str(mik), str(miv))

        # Fill columns
        for segmentID in statistics["SegmentIDs"]:
            rowIndex = table.AddEmptyRow()
            columnIndex = 0
            for key in keys:
                value = statistics[segmentID, key] if (segmentID, key) in statistics else None
                if value is None and key != SegmentStatisticsLogic.segmentColumnName:
                    value = float("nan")
                if isinstance(value, list):
                    for i in range(len(value)):
                        table.GetTable().GetColumn(columnIndex).SetComponent(rowIndex, i, value[i])
                else:
                    table.GetTable().GetColumn(columnIndex).SetValue(rowIndex, value)
                columnIndex += 1

        table.Modified()
        table.EndModify(tableWasModified)

    def showTable(self, table):
        """Switch to a layout where tables are visible and show the selected table"""
        currentLayout = slicer.app.layoutManager().layout
        layoutWithTable = slicer.modules.tables.logic().GetLayoutWithTable(currentLayout)
        slicer.app.layoutManager().setLayout(layoutWithTable)
        slicer.app.applicationLogic().GetSelectionNode().SetActiveTableID(table.GetID())
        table.SetUseColumnTitleAsColumnHeader(True)  # Make column titles visible (instead of column names)
        slicer.app.applicationLogic().PropagateTableSelection()

    def exportToString(self, nonEmptyKeysOnly=True):
        """Returns string with comma separated values, with header keys in quotes."""
        keys = self.getNonEmptyKeys() if nonEmptyKeysOnly else self.keys
        # Header
        csv = '"' + '","'.join(keys) + '"'
        # Rows
        statistics = self.getStatistics()
        for segmentID in statistics["SegmentIDs"]:
            csv += "\n" + str(statistics[segmentID, keys[0]])
            for key in keys[1:]:
                if (segmentID, key) in statistics:
                    csv += "," + str(statistics[segmentID, key])
                else:
                    csv += ","
        return csv

    def exportToCSVFile(self, fileName, nonEmptyKeysOnly=True):
        fp = open(fileName, "w")
        fp.write(self.exportToString(nonEmptyKeysOnly))
        fp.close()


class SegmentStatisticsTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        slicer.mrmlScene.Clear(0)

    def runTest(self, scenario=None):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_SegmentStatisticsBasic()

        self.setUp()
        self.test_SegmentStatisticsPlugins()

    def test_SegmentStatisticsBasic(self):
        """This tests some aspects of the label statistics"""

        self.delayDisplay("Starting test_SegmentStatisticsBasic")

        import SampleData
        from SegmentStatistics import SegmentStatisticsLogic

        self.delayDisplay("Load source volume")

        sourceVolumeNode = SampleData.downloadSample("MRBrainTumor1")

        self.delayDisplay("Create segmentation containing a few spheres")

        segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
        segmentationNode.CreateDefaultDisplayNodes()
        segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(sourceVolumeNode)

        # Geometry for each segment is defined by: radius, posX, posY, posZ
        segmentGeometries = [[10, -6, 30, 28], [20, 0, 65, 32], [15, 1, -14, 30], [12, 0, 28, -7], [5, 0, 30, 64],
                             [12, 31, 33, 27], [17, -42, 30, 27]]
        for segmentGeometry in segmentGeometries:
            sphereSource = vtk.vtkSphereSource()
            sphereSource.SetRadius(segmentGeometry[0])
            sphereSource.SetCenter(segmentGeometry[1], segmentGeometry[2], segmentGeometry[3])
            sphereSource.Update()
            uniqueSegmentID = segmentationNode.GetSegmentation().GenerateUniqueSegmentID("Test")
            segmentationNode.AddSegmentFromClosedSurfaceRepresentation(sphereSource.GetOutput(), uniqueSegmentID)

        self.delayDisplay("Compute statistics")

        segStatLogic = SegmentStatisticsLogic()
        segStatLogic.getParameterNode().SetParameter("Segmentation", segmentationNode.GetID())
        segStatLogic.getParameterNode().SetParameter("ScalarVolume", sourceVolumeNode.GetID())
        segStatLogic.computeStatistics()

        self.delayDisplay("Check a few numerical results")
        self.assertEqual(segStatLogic.getStatistics()["Test_2", "LabelmapSegmentStatisticsPlugin.voxel_count"], 9807)
        self.assertEqual(segStatLogic.getStatistics()["Test_4", "ScalarVolumeSegmentStatisticsPlugin.voxel_count"], 380)

        self.delayDisplay("Export results to table")
        resultsTableNode = slicer.vtkMRMLTableNode()
        slicer.mrmlScene.AddNode(resultsTableNode)
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)

        self.delayDisplay("Export results to string")
        logging.info(segStatLogic.exportToString())

        outputFilename = slicer.app.temporaryPath + "/SegmentStatisticsTestOutput.csv"
        self.delayDisplay("Export results to CSV file: " + outputFilename)
        segStatLogic.exportToCSVFile(outputFilename)

        self.delayDisplay("test_SegmentStatisticsBasic passed!")

    def test_SegmentStatisticsPlugins(self):
        """This tests some aspects of the segment statistics plugins"""

        self.delayDisplay("Starting test_SegmentStatisticsPlugins")

        import vtkSegmentationCorePython as vtkSegmentationCore
        import SampleData
        from SegmentStatistics import SegmentStatisticsLogic

        self.delayDisplay("Load source volume")

        sourceVolumeNode = SampleData.downloadSample("MRBrainTumor1")

        self.delayDisplay("Create segmentation containing a few spheres")

        segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
        segmentationNode.CreateDefaultDisplayNodes()
        segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(sourceVolumeNode)

        # Geometry for each segment is defined by: radius, posX, posY, posZ
        segmentGeometries = [[10, -6, 30, 28], [20, 0, 65, 32], [15, 1, -14, 30], [12, 0, 28, -7], [5, 0, 30, 64],
                             [12, 31, 33, 27], [17, -42, 30, 27]]
        for segmentGeometry in segmentGeometries:
            sphereSource = vtk.vtkSphereSource()
            sphereSource.SetRadius(segmentGeometry[0])
            sphereSource.SetCenter(segmentGeometry[1], segmentGeometry[2], segmentGeometry[3])
            sphereSource.Update()
            segment = vtkSegmentationCore.vtkSegment()
            uniqueSegmentID = segmentationNode.GetSegmentation().GenerateUniqueSegmentID("Test")
            segmentationNode.AddSegmentFromClosedSurfaceRepresentation(sphereSource.GetOutput(), uniqueSegmentID)

        # test calculating only measurements for selected segments
        self.delayDisplay("Test calculating only measurements for individual segments")
        segStatLogic = SegmentStatisticsLogic()
        segStatLogic.getParameterNode().SetParameter("Segmentation", segmentationNode.GetID())
        segStatLogic.getParameterNode().SetParameter("ScalarVolume", sourceVolumeNode.GetID())
        segStatLogic.updateStatisticsForSegment("Test_2")
        resultsTableNode = slicer.vtkMRMLTableNode()
        slicer.mrmlScene.AddNode(resultsTableNode)
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)
        self.assertEqual(segStatLogic.getStatistics()["Test_2", "LabelmapSegmentStatisticsPlugin.voxel_count"], 9807)
        with self.assertRaises(KeyError):
            segStatLogic.getStatistics()["Test_4", "ScalarVolumeSegmentStatisticsPlugin.voxel count"]
        # assert there are no result for this segment
        segStatLogic.updateStatisticsForSegment("Test_4")
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)
        self.assertEqual(segStatLogic.getStatistics()["Test_2", "LabelmapSegmentStatisticsPlugin.voxel_count"], 9807)
        self.assertEqual(segStatLogic.getStatistics()["Test_4", "LabelmapSegmentStatisticsPlugin.voxel_count"], 380)
        with self.assertRaises(KeyError):
            segStatLogic.getStatistics()["Test_5", "ScalarVolumeSegmentStatisticsPlugin.voxel count"]
        # assert there are no result for this segment

        # calculate measurements for all segments
        segStatLogic.computeStatistics()
        self.assertEqual(segStatLogic.getStatistics()["Test", "LabelmapSegmentStatisticsPlugin.voxel_count"], 2948)
        self.assertEqual(segStatLogic.getStatistics()["Test_1", "LabelmapSegmentStatisticsPlugin.voxel_count"], 23281)

        # test updating measurements for segments one by one
        self.delayDisplay("Update some segments in the segmentation")
        segmentGeometriesNew = [[5, -6, 30, 28], [21, 0, 65, 32]]
        # We add/remove representations, so we temporarily block segment modifications
        # to make sure display managers don't try to access data while it is in an
        # inconsistent state.
        wasModified = segmentationNode.StartModify()
        for i in range(len(segmentGeometriesNew)):
            segmentGeometry = segmentGeometriesNew[i]
            sphereSource = vtk.vtkSphereSource()
            sphereSource.SetRadius(segmentGeometry[0])
            sphereSource.SetCenter(segmentGeometry[1], segmentGeometry[2], segmentGeometry[3])
            sphereSource.Update()
            segment = segmentationNode.GetSegmentation().GetNthSegment(i)
            segment.RemoveAllRepresentations()
            closedSurfaceName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName()
            segment.AddRepresentation(closedSurfaceName,
                                      sphereSource.GetOutput())
        segmentationNode.EndModify(wasModified)
        self.assertEqual(segStatLogic.getStatistics()["Test", "LabelmapSegmentStatisticsPlugin.voxel_count"], 2948)
        self.assertEqual(segStatLogic.getStatistics()["Test_1", "LabelmapSegmentStatisticsPlugin.voxel_count"], 23281)
        segStatLogic.updateStatisticsForSegment("Test_1")
        self.assertEqual(segStatLogic.getStatistics()["Test", "LabelmapSegmentStatisticsPlugin.voxel_count"], 2948)
        self.assertTrue(segStatLogic.getStatistics()["Test_1", "LabelmapSegmentStatisticsPlugin.voxel_count"] != 23281)
        segStatLogic.updateStatisticsForSegment("Test")
        self.assertTrue(segStatLogic.getStatistics()["Test", "LabelmapSegmentStatisticsPlugin.voxel_count"] != 2948)
        self.assertTrue(segStatLogic.getStatistics()["Test_1", "LabelmapSegmentStatisticsPlugin.voxel_count"] != 23281)

        # test enabling/disabling of individual measurements
        self.delayDisplay("Test disabling of individual measurements")
        segStatLogic = SegmentStatisticsLogic()
        segStatLogic.getParameterNode().SetParameter("Segmentation", segmentationNode.GetID())
        segStatLogic.getParameterNode().SetParameter("ScalarVolume", sourceVolumeNode.GetID())
        segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.voxel_count.enabled", str(False))
        segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.volume_cm3.enabled", str(False))
        segStatLogic.computeStatistics()
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)
        columnHeaders = [resultsTableNode.GetColumnName(i) for i in range(resultsTableNode.GetNumberOfColumns())]
        self.assertEqual(columnHeaders, ["Segment", "Volume mm3 (LM)", "Voxel count", "Volume mm3 (SV)", "Volume cm3 (SV)",
            "Minimum", "Maximum", "Mean", "Median", "Standard deviation", "Surface mm2", "Volume mm3 (CS)", "Volume cm3 (CS)"])

        self.delayDisplay("Test re-enabling of individual measurements")
        segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.voxel_count.enabled", str(True))
        segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.volume_cm3.enabled", str(True))
        segStatLogic.computeStatistics()
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)
        columnHeaders = [resultsTableNode.GetColumnName(i) for i in range(resultsTableNode.GetNumberOfColumns())]
        self.assertTrue("Voxel count (LM)" in columnHeaders)
        self.assertTrue("Volume mm3 (LM)" in columnHeaders)
        self.assertTrue("Volume cm3 (LM)" in columnHeaders)

        # test enabling/disabling of individual plugins
        self.delayDisplay("Test disabling of plugin")
        segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.enabled", str(False))
        segStatLogic.computeStatistics()
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)
        columnHeaders = [resultsTableNode.GetColumnName(i) for i in range(resultsTableNode.GetNumberOfColumns())]
        self.assertFalse("Voxel count (LM)" in columnHeaders)
        self.assertFalse("Volume mm3 (LM)" in columnHeaders)
        self.assertTrue("Volume mm3 (SV)" in columnHeaders)

        self.delayDisplay("Test re-enabling of plugin")
        segStatLogic.getParameterNode().SetParameter("LabelmapSegmentStatisticsPlugin.enabled", str(True))
        segStatLogic.computeStatistics()
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)
        columnHeaders = [resultsTableNode.GetColumnName(i) for i in range(resultsTableNode.GetNumberOfColumns())]
        self.assertTrue("Voxel count (LM)" in columnHeaders)
        self.assertTrue("Volume mm3 (LM)" in columnHeaders)

        # test unregistering/registering of plugins
        self.delayDisplay("Test of removing all registered plugins")
        SegmentStatisticsLogic.registeredPlugins = []  # remove all registered plugins
        segStatLogic = SegmentStatisticsLogic()
        segStatLogic.getParameterNode().SetParameter("Segmentation", segmentationNode.GetID())
        segStatLogic.getParameterNode().SetParameter("ScalarVolume", sourceVolumeNode.GetID())
        segStatLogic.computeStatistics()
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)
        columnHeaders = [resultsTableNode.GetColumnName(i) for i in range(resultsTableNode.GetNumberOfColumns())]
        self.assertEqual(len(columnHeaders), 1)  # only header element should be "Segment"
        self.assertEqual(columnHeaders[0], "Segment")  # only header element should be "Segment"

        self.delayDisplay("Test registering plugins")
        SegmentStatisticsLogic.registerPlugin(LabelmapSegmentStatisticsPlugin())
        SegmentStatisticsLogic.registerPlugin(ScalarVolumeSegmentStatisticsPlugin())
        SegmentStatisticsLogic.registerPlugin(ClosedSurfaceSegmentStatisticsPlugin())
        segStatLogic = SegmentStatisticsLogic()
        segStatLogic.getParameterNode().SetParameter("Segmentation", segmentationNode.GetID())
        segStatLogic.getParameterNode().SetParameter("ScalarVolume", sourceVolumeNode.GetID())
        segStatLogic.computeStatistics()
        segStatLogic.exportToTable(resultsTableNode)
        segStatLogic.showTable(resultsTableNode)
        columnHeaders = [resultsTableNode.GetColumnName(i) for i in range(resultsTableNode.GetNumberOfColumns())]
        self.assertTrue("Voxel count (LM)" in columnHeaders)
        self.assertTrue("Voxel count (SV)" in columnHeaders)
        self.assertTrue("Surface mm2" in columnHeaders)

        self.delayDisplay("test_SegmentStatisticsPlugins passed!")


class Slicelet:
    """A slicer slicelet is a module widget that comes up in stand alone mode
    implemented as a python class.
    This class provides common wrapper functionality used by all slicer modlets.
    """

    # TODO: put this in a SliceletLib
    # TODO: parse command line args

    def __init__(self, widgetClass=None):
        self.parent = qt.QFrame()
        self.parent.setLayout(qt.QVBoxLayout())

        # TODO: should have way to pop up python console
        self.buttons = qt.QFrame()
        self.buttons.setLayout(qt.QHBoxLayout())
        self.parent.layout().addWidget(self.buttons)
        self.addDataButton = qt.QPushButton(_("Add Data"))
        self.buttons.layout().addWidget(self.addDataButton)
        self.addDataButton.connect("clicked()", slicer.app.ioManager().openAddDataDialog)
        self.loadSceneButton = qt.QPushButton(_("Load Scene"))
        self.buttons.layout().addWidget(self.loadSceneButton)
        self.loadSceneButton.connect("clicked()", slicer.app.ioManager().openLoadSceneDialog)

        if widgetClass:
            self.widget = widgetClass(self.parent)
            self.widget.setup()
        self.parent.show()


class SegmentStatisticsSlicelet(Slicelet):
    """Creates the interface when module is run as a stand alone gui app."""

    def __init__(self):
        super().__init__(SegmentStatisticsWidget)


if __name__ == "__main__":
    # TODO: need a way to access and parse command line arguments
    # TODO: ideally command line args should handle --xml

    import sys

    print(sys.argv)

    slicelet = SegmentStatisticsSlicelet()
