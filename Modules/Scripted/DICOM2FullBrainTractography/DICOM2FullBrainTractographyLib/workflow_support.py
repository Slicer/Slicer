import os
import time

import vtk
import ctk
import qt

import saferef
import slicer

__all__ = ['Workflow', 'GeneralizedStep', 'display_error']

class Workflow:

    def __init__(self, workflow_configuration, parent=None):
        if not parent:
            self.parent = qt.QWidget()
            self.parent.setLayout(qt.QVBoxLayout())
        else:
            self.parent = parent

        self.workflow_configuration = workflow_configuration

        parent.connect(parent, 'mrmlSceneChanged(vtkMRMLScene*)', self.workflow_configuration.setMRMLScene)
        self.workflow_configuration.setMRMLScene(slicer.mrmlScene)

        loader = qt.QUiLoader()

        self.steps = []

        def onEntryCallback(actual_step, comingFrom, transitionType):
            data = self.extract_data()

            method_name = 'on_entry_'+actual_step.id()
            if hasattr(self.workflow_configuration, method_name):
                on_entry = getattr(self.workflow_configuration, method_name)
                on_entry(actual_step, comingFrom, transitionType, data)
                self.update_data(data)

        def onExitCallback(actual_step, comingFrom, transitionType):
            data = self.extract_data()

            method_name = 'on_exit_'+actual_step.id()
            if hasattr(self.workflow_configuration, method_name):
                on_exit = getattr(self.workflow_configuration, method_name)
                on_exit(actual_step, comingFrom, transitionType, data)
                self.update_data(data)

        def validateCallback(actual_step, desiredBranchId):
            data = self.extract_data()

            method_name = 'validate_'+actual_step.id()
            if hasattr(self.workflow_configuration, method_name):
                validate = getattr(self.workflow_configuration, method_name)
                return validate(actual_step, data)
            else:
                return True

        self.callbacks = {
                'onEntryCallback': onEntryCallback,
                'onExitCallback': onExitCallback,
                'validateCallback': validateCallback
        }

        for step_widget_file in self.workflow_configuration.step_widget_files:
            path = os.path.join(
                 os.path.dirname(__file__), 'Resources', 'UI',
                 step_widget_file + '.ui'
             )

            qfile = qt.QFile(path)
            qfile.open(qt.QFile.ReadOnly)
            widget = loader.load( qfile )

            if hasattr(widget, 'setMRMLScene'):
                widget.connect(parent, 'mrmlSceneChanged(vtkMRMLScene*)',widget.setMRMLScene)
                widget.setMRMLScene(slicer.mrmlScene)

            if hasattr(self.workflow_configuration, 'post_widget_init'):
                self.workflow_configuration.post_widget_init(step_widget_file, widget)

            step = GeneralizedStep(step_widget_file, widget,
                                   onEntryCallback=onEntryCallback,
                                   onExitCallback=onExitCallback,
                                   validateCallback=validateCallback)

            self.steps.append((step_widget_file, step))


        self.workflow = ctk.ctkWorkflow(self.parent)
        self.workflowWidget = ctk.ctkWorkflowStackedWidget()

        self.workflowWidget.setWorkflow(self.workflow)
        self.parent.layout().addWidget(self.workflowWidget)

        for i in xrange(len(self.steps) - 1):
            self.workflow.addTransition(self.steps[i][1], self.steps[i + 1][1])

        self.workflow.start()

    def extract_data(self):

        data = {}
        for name, step in self.steps:
            widget = step.qt_widget
            data[ name ] = {}
            if name in self.workflow_configuration.step_widget_fields:
                for field_name, attribute_name in self.workflow_configuration.step_widget_fields[ name ]:
                    data[name][field_name] = getattr(widget_find_field( widget, field_name ), attribute_name)


        return data

    def update_data(self, data):

        data = {}
        for name, step in self.steps:
            if name not in data:
                continue

            widget = step.qt_widget
            if name in self.workflow_configuration.step_widget_fields:
                for field_name, attribute_name in self.workflow_configuration.step_widget_fields[ name ]:
                    if field_name in data[name]:
                        setattr( widget_find_field( widget, field_name ), attribute_name )

class GeneralizedStep(ctk.ctkWorkflowWidgetStep, ) :
    """Step implemented using the derivation approach"""

    def __init__(self, stepid, qt_widget, onEntryCallback=None, validateCallback=None, onExitCallback=None):
        self.initialize(stepid)
        self.qt_widget = qt_widget

        if onEntryCallback:
            self.weak_onEntryCallback = saferef.safeRef(onEntryCallback)

        if validateCallback:
            self.weak_validateCallback = saferef.safeRef(validateCallback)

        if onExitCallback:
            self.weak_onExitCallback = saferef.safeRef(onExitCallback)

    def createUserInterface(self):
        layout = qt.QVBoxLayout(self)
        scrollArea = qt.QScrollArea()
        scrollArea.setWidget(self.qt_widget)
        layout.addWidget(scrollArea)

    def onEntry(self, comingFrom, transitionType):
        comingFromId = "None"
        if comingFrom: comingFromId = comingFrom.id()
        super(GeneralizedStep, self).onEntry(comingFrom, transitionType)

        if hasattr(self, 'weak_onEntryCallback'):
            onEntryCallback = self.weak_onEntryCallback()
            onEntryCallback(self, comingFrom, transitionType)

    def onExit(self, goingTo, transitionType):
        goingToId = "None"
        if goingTo: goingToId = goingTo.id()
        super(GeneralizedStep, self).onExit(goingTo, transitionType)

        if hasattr(self, 'weak_onExitCallback'):
            onExitCallback = self.weak_onExitCallback()
            onExitCallback(self, goingTo, transitionType)

    def validate(self, desiredBranchId):
        validationSuceeded = True

        if hasattr(self, 'weak_validateCallback'):
            validateCallback = self.weak_validateCallback()
            validationSuceeded = validateCallback(self, desiredBranchId)

        super(GeneralizedStep, self).validate(validationSuceeded, desiredBranchId)


def widget_find_field(widget, objectName):
    if widget.objectName == objectName:
        return widget
    else:
        children = []
        for w in widget.children():
            resulting_widget = widget_find_field(w, objectName)
            if resulting_widget:
                children.append(resulting_widget)

        if len( children ) == 0:
            return None
        else:
            if len(children) > 1:
                raise ValueError("More than one widget with the specified object name: " + objectName)
            return children[0]



__VOLUME_TYPES__ =(
    'Scalar',
    'DiffusionWeighted',
    'DiffusionTensor'
)



def create_volume_node(volume_type, attach_display_node = False, dimensions=None, prefix=''):
    """
    Creates a volume node and inserts it into the MRML tree
    """
    if volume_type not in __VOLUME_TYPES__:
        raise ValueError('Volume type %s is not valid' % volume_type )

    volume_node = eval('slicer.vtkMRML%sVolumeNode()' % volume_type)
    volume_node.SetName(slicer.mrmlScene.GetUniqueNameByString('%s%s' % (prefix, volume_type)))

    if dimensions:
        image_data = vtk.vtkImageData()
        image_data.SetDimensions(dimensions)
        image_data.AllocateScalars(vtk.VTK_UNSIGNED_INT, 1)

        volume_node.SetAndObserveImageData(image_data)

    slicer.mrmlScene.AddNode(volume_node)

    if attach_display_node:
        display_node = eval('slicer.vtkMRML%sVolumeDisplayNode()' % volume_type)
        slicer.mrmlScene.AddNode(display_node)
        volume_node.AddAndObserveDisplayNodeID( display_node.GetID() )

    return volume_node

import sys
import re
this_module = sys.modules[__name__]
for volume_type in __VOLUME_TYPES__:
    function_name =\
            'create' +\
            re.sub('([A-Z])','_\\1',volume_type).lower() +\
            '_volume_node'

    setattr(this_module, function_name,
            eval(
                'lambda attach_display_node=False, dimensions=None, prefix="":\
                create_volume_node( "%s", attach_display_node=attach_display_node, dimensions=dimensions, prefix=prefix)' %\
                volume_type)
               )

    this_module.__all__.append(function_name)


def display_error(message):
    mb = qt.QMessageBox()
    mb.setText(message)
    mb.setStandardButtons(mb.Ok)
    mb.setDefaultButton(mb.Ok )
    mb.exec_()

