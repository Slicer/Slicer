import os
import time

import ctk
import qt
import vtk

import slicer

if not '__file__' in locals():
    __file__ = os.path.join(os.getcwd(), 'full_tractography_workflow.py')


WORKFLOW_PREFIX = "DICOM2FBTWorkflow"
class WorkflowConfiguration:
    step_widget_files = [
        'dicom2nrrd',
        'dwi2dti',
        'dti2fibers',
        'done',
    ]

    step_widget_fields = {
        'dicom2nrrd':[
            ('DICOMRadioButton', 'checked'),
            ('NRRDDWIRadioButton', 'checked'),
            ('inputDicomDirectory', 'directory'),
            ('outputVolume', 'currentPath'),
            ('useBMatrixGradientDirections','checked'),
            ('inputNRRDVolume','currentPath'),
        ],
        'dwi2dti':[
            ('leastSquaresEstimation', 'checked'),
            ('weightedLeastSquaresEstimation', 'checked'),
            ('thresholdParameter', 'value'),
            ('removeIslands', 'checked'),
            ('applyMask', 'checked'),
        ],
        'dti2fibers':[
            ('seedSpacing','value'),
            ('stoppingFAValue','value'),
            ('minimumFAValueSeed','value'),
            ('stoppingTrackCurvature','value'),
        ],
        'done':[],
    }

    dwi_node_name = None

    def __init__(self):
        self.slicerVolumesLogic = slicer.vtkSlicerVolumesLogic()
        self.slicerVolumesLogic.SetMRMLScene(slicer.mrmlScene)
        self.dicomtonrrdconverter_parameter_node = None
        self.diffusionweightedvolumemasking_parameter_node = None
        self.diffusiontensorestimation_parameter_node = None
        self.seeding_parameter_node = None

    def on_entry_dicom2nrrd(self, step_object, coming_from, transition_type, data):
        pass

    def on_exit_dicom2nrrd(self, step_object, going_to, transition_type, data):
        pass

    def validate_dicom2nrrd(self, step_object, data):
        if data[step_object.id()]['DICOMRadioButton']:

            output_volume = data[step_object.id()]['outputVolume']
            print "\t output volume:", output_volume
            if not (
                (
                    (not os.path.exists(output_volume)) or
                    os.path.isdir(os.path.basename(output_volume))
                ) or (
                    os.path.exists(output_volume) and
                    os.path.isfile(output_volume)
                )
            ):
                display_error("DICOM to NRRD conversion needs a valid output NRRD file")
                return False

            self.dicomtonrrdconverter_parameter_node = slicer.cli.run(
                slicer.modules.dicomtonrrdconverter, self.dicomtonrrdconverter_parameter_node,
                data[step_object.id()],
                wait_for_completion = True)

            if self.dicomtonrrdconverter_parameter_node.GetStatusString() == 'Completed':
                file_path = data[step_object.id()]['outputVolume']
                result_status, node = slicer.util.loadVolume(
                    file_path,
                    True
                )
            else:
                result_status = False

            if result_status:
                self.dwi_node = node
                self.dwi_node_name = node.GetID()

        elif data[step_object.id()]['NRRDDWIRadioButton']:
            result_status, node = slicer.util.loadVolume(
                data[step_object.id()]['inputNRRDVolume'],
                True
            )

            if result_status:
                self.dwi_node = node
                self.dwi_node_name = node.GetID()


        if not result_status:
            display_error("Error in DICOM to NRRD conversion, please see log")

        return result_status

    def validate_dwi2dti(self, step_object, data):
        dimensions = self.dwi_node.GetImageData().GetDimensions()
        if not hasattr(self, 'tensor_node'):
            self.tensor_node = create_diffusion_tensor_volume_node()
        if not hasattr(self, 'baseline_node'):
            self.baseline_node = create_scalar_volume_node(dimensions=dimensions)
        if not hasattr(self, 'mask_node'):
            self.mask_node = create_scalar_volume_node(dimensions=dimensions)
            self.mask_node.LabelMapOn()

        step_parameters = data[step_object.id()]

        parameters_mask = {
            'inputVolume': self.dwi_node.GetID(),
            'otsuMegaThreshold': step_parameters['thresholdParameter'],
            'removeIslands': step_parameters['removeIslands'],
            'thresholdMask': self.mask_node.GetID(),
            'outputBaseline': self.baseline_node.GetID(),
        }

        self.diffusionweightedvolumemasking_parameter_node = slicer.cli.run(
            slicer.modules.diffusionweightedvolumemasking, self.diffusionweightedvolumemasking_parameter_node,
            parameters_mask,
            wait_for_completion=True
        )

        if self.diffusionweightedvolumemasking_parameter_node.GetStatusString() != "Completed":
            display_error("Error in diffusion tensor estimation")
            return False

        parameters_estimation = {
            'inputVolume': self.dwi_node.GetID(),
            'outputTensor': self.tensor_node.GetID(),
            'outputBaseline': self.baseline_node.GetID(),
            'leastSquaresEstimation':step_parameters['leastSquaresEstimation'],
            'weightedLeastSquaresEstimation':step_parameters['weightedLeastSquaresEstimation']
        }

        if step_parameters['applyMask']:
            parameters_estimation['inputMaskVolume'] = self.mask_node.GetID()
        else:
            if 'inputMaskVolume' in parameters_estimation:
                parameters_estimation['inputMaskVolume'] = ''

        self.diffusiontensorestimation_parameter_node = slicer.cli.run(
            slicer.modules.diffusiontensorestimation, self.diffusiontensorestimation_parameter_node,
            parameters_estimation,
            wait_for_completion=True
        )

        result_status = self.diffusiontensorestimation_parameter_node.GetStatusString() == 'Completed'

        if not result_status:
            display_error("Error in diffusion tensor estimation")

        return result_status


    def validate_dti2fibers(self, step_object, data):

        if not hasattr(self, 'tractography_node'):
            self.tractography_node = slicer.vtkMRMLFiberBundleNode()
            slicer.mrmlScene.AddNode(self.tractography_node)

        parameters = {
            'InputVolume':self.tensor_node.GetID(),
            'InputROI':self.mask_node.GetID(),
            'OutputFibers':self.tractography_node.GetID(),
            'SeedSpacing':data[step_object.id()]['seedSpacing'],
            'StoppingValue':data[step_object.id()]['stoppingFAValue'],
            'StoppingCurvature':data[step_object.id()]['stoppingTrackCurvature'],
            'ClTh':data[step_object.id()]['minimumFAValueSeed'],
        }

        parameters.update(data[step_object.id()])

        self.seeding_parameter_node = slicer.cli.run(
            slicer.modules.seeding, self.seeding_parameter_node,
            parameters,
            wait_for_completion=True
        )

        result_status = self.seeding_parameter_node.GetStatusString() == 'Completed'

        if not result_status:
            display_error("Error in tractography")

        return result_status





class GeneralizedStep(ctk.ctkWorkflowWidgetStep, ) :
    """Step implemented using the derivation approach"""

    def __init__(self, stepid, qt_widget, onEntryCallback=None, validateCallback=None, onExitCallback=None):
        self.initialize(stepid)
        self.qt_widget = qt_widget

        if onEntryCallback:
            self.onEntryCallback = onEntryCallback

        if validateCallback:
            self.validateCallback = validateCallback

        if onExitCallback:
            self.onExitCallback = onExitCallback


    def createUserInterface(self):
        layout = qt.QVBoxLayout(self)
        scrollArea = qt.QScrollArea()
        scrollArea.setWidget(self.qt_widget)
        layout.addWidget(scrollArea)

    def onEntry(self, comingFrom, transitionType):
        comingFromId = "None"
        if comingFrom: comingFromId = comingFrom.id()
        super(GeneralizedStep, self).onEntry(comingFrom, transitionType)

        if hasattr(self, 'onEntryCallback'):
            self.onEntryCallback(self, comingFrom, transitionType)

    def onExit(self, goingTo, transitionType):
        goingToId = "None"
        if goingTo: goingToId = goingTo.id()
        super(GeneralizedStep, self).onExit(goingTo, transitionType)

        if hasattr(self, 'onExitCallback'):
            self.onExitCallback(self, goingTo, transitionType)

    def validate(self, desiredBranchId):
        validationSuceeded = True

        if hasattr(self, 'validateCallback'):
            validationSuceeded = self.validateCallback(self, desiredBranchId)

        super(GeneralizedStep, self).validate(validationSuceeded, desiredBranchId)




class Slicelet(object):
    """A slicer modlet is a module widget that comes up in stand alone mode
    implemented as a python class.
    This class provides common wrapper functionality used by all slicer modlets.
    """
    # TODO: put this in a SliceletLib
    # TODO: parse command line arge

    def __init__(self, widgetClass=None):
        self.parent = qt.QFrame()
        self.parent.setLayout( qt.QVBoxLayout() )

        # TODO: should have way to pop up python interactor

        if widgetClass:
            self.widget = widgetClass(self.parent)
            self.widget.setup()
        self.parent.show()

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

class full_tractography_workflow:

    def __init__(self, workflow_configuration, parent=None):
        if not parent:
            self.parent = qt.QWidget()
            self.parent.setLayout(qt.QVBoxLayout())
        else:
            self.parent = parent

        self.workflow_configuration = workflow_configuration

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

        for step_widget_file in self.workflow_configuration.step_widget_files:
            path = os.path.join(
                 os.path.dirname(__file__), 'Resources', 'UI',
                 step_widget_file + '.ui'
             )

            qfile = qt.QFile(path)
            qfile.open(qt.QFile.ReadOnly)
            widget = loader.load( qfile )


            step = GeneralizedStep(step_widget_file, widget,
                                   onEntryCallback=onEntryCallback,
                                   onExitCallback=onExitCallback,
                                   validateCallback=validateCallback)

            self.steps.append((step_widget_file, step))

        self.workflow = ctk.ctkWorkflow()
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

class Slicelet(object):
    """A Slicelet is a module widget that comes up in stand alone mode
    implemented as a python class.
    This class provides common wrapper functionality used by all slicer modlets.
    """
    # TODO: put this in a SliceletLib
    # TODO: parse command line arge

    def __init__(self, widgetClass=None, *args, **kwargs):
        self.parent = qt.QFrame()
        self.parent.setLayout(qt.QVBoxLayout())

        # TODO: should have way to pop up python interactor

        if widgetClass:
            kwargs.update({'parent':self.parent})
            self.widget = widgetClass(*args, **kwargs)
        self.parent.show()

class full_tractography_workflow_slicelet(Slicelet):
    """ Creates the interface when module is run as a stand alone gui app.
    """

    def __init__(self):
        super(full_tractography_workflow_slicelet, self).__init__(full_tractography_workflow, WorkflowConfiguration())

def wait_for_module(module, time_pause=5):
    while module.GetStatusString() not in ('Completed','CompletedWithErrors','Cancelled'):
        time.sleep(time_pause)


__VOLUME_TYPES__ =(
    'Scalar',
    'DiffusionWeighted',
    'DiffusionTensor'
)

#def create_scalar_volume_node(attach_display_node=False, dimensions=None):
#    return create_volume_node('Scalar', attach_display_node, dimensions=dimensions)
#
#def create_diffusion_tensor_volume_node(attach_display_node=False, dimensions=None):
#   return create_volume_node('DiffusionTensor', attach_display_node, dimensions=dimensions)

def create_volume_node(volume_type, attach_display_node = False, dimensions=None):
    """
    Creates a volume node and inserts it into the MRML tree
    """
    if volume_type not in __VOLUME_TYPES__:
        raise ValueError('Volume type %s is not valid' % volume_type )

    volume_node = eval('slicer.vtkMRML%sVolumeNode()' % volume_type)
    volume_node.SetName(slicer.mrmlScene.GetUniqueNameByString('%s%s' % (WORKFLOW_PREFIX, volume_type)))

    if dimensions:
        image_data = vtk.vtkImageData()
        image_data.SetDimensions(dimensions)
        image_data.AllocateScalars()
        volume_node.SetAndObserveImageData(image_data)

    slicer.mrmlScene.AddNode(volume_node)

    if attach_display_node:
        display_node = eval('slicer.vtkMRML%sVolumeDisplayNode()' % volume_type)
        slicer.mrmlScene.AddNode(display_node)
        volume_node.AddAndObserveDisplayNodeID( display_node.GetID() )

    return volume_node


def display_error(message):
    mb = qt.QMessageBox()
    mb.setText(message)
    mb.setStandardButtons(mb.Ok)
    mb.setDefaultButton(mb.Ok )
    mb.exec_()


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
                'lambda attach_display_node=False, dimensions=None:\
                create_volume_node( "%s", attach_display_node=attach_display_node, dimensions=dimensions)' %\
                volume_type)
               )


if __name__ == "__main__":
  # TODO: need a way to access and parse command line arguments
  # TODO: ideally command line args should handle --xml

  import sys
  slicelet = full_tractography_workflow_slicelet()
