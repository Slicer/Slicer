import os
import qt


import slicer

from .workflow_support import *

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
            ('MRMLDWIRadioButton', 'checked'),
            ('inputDicomDirectory', 'directory'),
            ('outputVolume', 'currentPath'),
            ('useBMatrixGradientDirections','checked'),
            ('inputNRRDVolume','currentPath'),
            ('inputNRRDMRMLNode','currentNodeID'),
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
        self.dwiconvert_parameter_node = None
        self.diffusionweightedvolumemasking_parameter_node = None
        self.diffusiontensorestimation_parameter_node = None
        self.seeding_parameter_node = None

    def post_widget_init(self, widget_name, widget):
        if widget_name == 'dicom2nrrd':
            for child in widget.findChildren(qt.QWidget, qt.QRegExp('.*FormWidget')):
                child.setVisible('DICOM' in child.name)


    def on_entry_dicom2nrrd(self, step_object, coming_from, transition_type, data):
        pass

    def on_exit_dicom2nrrd(self, step_object, going_to, transition_type, data):
        pass

    def validate_dicom2nrrd(self, step_object, data):
        if data[step_object.id()]['DICOMRadioButton']:

            output_volume = data[step_object.id()]['outputVolume']
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

            self.dwiconvert_parameter_node = slicer.cli.run(
                slicer.modules.dwiconvert, self.dwiconvert_parameter_node,
                data[step_object.id()],
                wait_for_completion = True)

            if self.dwiconvert_parameter_node.GetStatusString() == 'Completing' or \
                self.dwiconvert_parameter_node.GetStatusString() == 'Completed':
                file_path = data[step_object.id()]['outputVolume']
                result_status, node = slicer.util.loadVolume(
                    file_path,
                    {},
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
                {},
                True
            )

            if result_status:
                self.dwi_node = node
                self.dwi_node_name = node.GetID()

        elif data[step_object.id()]['MRMLDWIRadioButton']:
            result_status = True
            self.dwi_node_name = data[step_object.id()]['inputNRRDMRMLNode']
            self.dwi_node = slicer.mrmlScene.GetNodeByID(self.dwi_node_name)

        if not result_status:
            display_error("Error in DICOM to NRRD conversion, please see log")

        return result_status

    def validate_dwi2dti(self, step_object, data):
        dimensions = self.dwi_node.GetImageData().GetDimensions()
        if not hasattr(self, 'tensor_node'):
            self.tensor_node = create_diffusion_tensor_volume_node(prefix=WORKFLOW_PREFIX)
        if not hasattr(self, 'baseline_node'):
            self.baseline_node = create_scalar_volume_node(dimensions=dimensions, prefix=WORKFLOW_PREFIX)
        if not hasattr(self, 'mask_node'):
            self.mask_node = create_scalar_volume_node(dimensions=dimensions, prefix=WORKFLOW_PREFIX)
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

        if self.diffusionweightedvolumemasking_parameter_node.GetStatusString() != "Completing" or \
            self.diffusionweightedvolumemasking_parameter_node.GetStatusString() != "Completed":
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
            slicer.modules.dwitodtiestimation, self.diffusiontensorestimation_parameter_node,
            parameters_estimation,
            wait_for_completion=True
        )

        result_status = self.diffusiontensorestimation_parameter_node.GetStatusString() == 'Completing' or \
                             self.diffusiontensorestimation_parameter_node.GetStatusString() == 'Completed'

        if not result_status:
            display_error("Error in diffusion tensor estimation")

        return result_status


    def validate_dti2fibers(self, step_object, data):

        if not hasattr(self, 'tractography_node'):
            self.tractography_node = slicer.vtkMRMLFiberBundleNode()
            slicer.mrmlScene.AddNode(self.tractography_node)

        parameters = {
            'InputVolume':self.tensor_node.GetID(),
            'OutputFibers':self.tractography_node.GetID(),
            'SeedSpacing':data[step_object.id()]['seedSpacing'],
            'StoppingValue':data[step_object.id()]['stoppingFAValue'],
            'StoppingCurvature':data[step_object.id()]['stoppingTrackCurvature'],
            'ClTh':data[step_object.id()]['minimumFAValueSeed'],
        }

        parameters.update(data[step_object.id()])

        self.seeding_parameter_node = slicer.cli.run(
            slicer.modules.tractographylabelmapseeding, self.seeding_parameter_node,
            parameters,
            wait_for_completion=True
        )

        result_status = self.seeding_parameter_node.GetStatusString() == 'Completing' or \
                             self.seeding_parameter_node.GetStatusString() == 'Completed'

        if not result_status:
            display_error("Error in tractography")

        return result_status



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
        super(full_tractography_workflow_slicelet, self).__init__(Workflow, WorkflowConfiguration())



if __name__ == "__main__":
  # TODO: need a way to access and parse command line arguments
  # TODO: ideally command line args should handle --xml

  slicelet = full_tractography_workflow_slicelet()
