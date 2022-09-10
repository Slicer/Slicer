import ctk
import qt

import slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL


#
# RSNAQuantTutorial
#

class RSNAQuantTutorial(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "RSNAQuantTutorial"  # TODO make this more human readable by adding spaces
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Steve Pieper (Isomics)"]  # replace with "Firstname Lastname (Org)"
        parent.helpText = """
    This module was developed as a self test to perform the operations needed for the RSNA 2012 Quantitative Imaging Tutorial
    """
        parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
"""  # replace with organization, grant and thanks.


#
# qRSNAQuantTutorialWidget
#

class RSNAQuantTutorialWidget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        # Instantiate and connect widgets ...

        # Collapsible button
        testsCollapsibleButton = ctk.ctkCollapsibleButton()
        testsCollapsibleButton.text = "Tests"
        self.layout.addWidget(testsCollapsibleButton)

        # Layout within the collapsible button
        formLayout = qt.QFormLayout(testsCollapsibleButton)

        # test buttons
        tests = (("Part 1 : Ruler", self.onPart1Ruler), ("Part 2: ChangeTracker", self.onPart2ChangeTracker), ("Part 3 : PETCT", self.onPart3PETCT))
        for text, slot in tests:
            testButton = qt.QPushButton(text)
            testButton.toolTip = "Run the test."
            formLayout.addWidget(testButton)
            testButton.connect('clicked(bool)', slot)

        # A collapsible button to hide screen shot options
        screenShotsCollapsibleButton = ctk.ctkCollapsibleButton()
        screenShotsCollapsibleButton.text = "Screen shot options"
        self.layout.addWidget(screenShotsCollapsibleButton)

        # layout within the collapsible button
        screenShotsFormLayout = qt.QFormLayout(screenShotsCollapsibleButton)

        #
        # check box to trigger taking screen shots for later use in tutorials
        #
        self.enableScreenshotsFlagCheckBox = qt.QCheckBox()
        self.enableScreenshotsFlagCheckBox.checked = 0
        self.enableScreenshotsFlagCheckBox.setToolTip("If checked, take screen shots for tutorials. Use Save Data to write them to disk.")
        screenShotsFormLayout.addRow("Enable Screenshots", self.enableScreenshotsFlagCheckBox)

        #
        # scale factor for screen shots
        #
        self.screenshotScaleFactorSliderWidget = ctk.ctkSliderWidget()
        self.screenshotScaleFactorSliderWidget.singleStep = 1.0
        self.screenshotScaleFactorSliderWidget.minimum = 1.0
        self.screenshotScaleFactorSliderWidget.maximum = 50.0
        self.screenshotScaleFactorSliderWidget.value = 1.0
        self.screenshotScaleFactorSliderWidget.setToolTip("Set scale factor for the screen shots.")
        screenShotsFormLayout.addRow("Screenshot scale factor", self.screenshotScaleFactorSliderWidget)

        # Add vertical spacer
        self.layout.addStretch(1)

    def onPart1Ruler(self):
        enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
        screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

        tester = RSNAQuantTutorialTest()
        tester.setUp()
        tester.test_Part1Ruler(enableScreenshotsFlag, screenshotScaleFactor)

    def onPart2ChangeTracker(self):
        enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
        screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

        tester = RSNAQuantTutorialTest()
        tester.setUp()
        tester.test_Part2ChangeTracker(enableScreenshotsFlag, screenshotScaleFactor)

    def onPart3PETCT(self):
        enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
        screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

        tester = RSNAQuantTutorialTest()
        tester.setUp()
        tester.test_Part3PETCT(enableScreenshotsFlag, screenshotScaleFactor)


#
# RSNAQuantTutorialLogic
#

class RSNAQuantTutorialLogic(ScriptedLoadableModuleLogic):
    """This class should implement all the actual
    computation done by your module.  The interface
    should be such that other python code can import
    this class and make use of the functionality without
    requiring an instance of the Widget
    """
    pass


class RSNAQuantTutorialTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        self.delayDisplay("Closing the scene")
        layoutManager = slicer.app.layoutManager()
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_Part1Ruler()
        self.setUp()
        self.test_Part2ChangeTracker()
        self.setUp()
        self.test_Part3PETCT()

    def test_Part1Ruler(self, enableScreenshotsFlag=0, screenshotScaleFactor=1):
        """ Test using rulers
        """
        self.enableScreenshots = enableScreenshotsFlag
        self.screenshotScaleFactor = screenshotScaleFactor

        self.delayDisplay("Starting the test")

        #
        # first, get some data
        #
        import SampleData
        tumor = SampleData.downloadSample('MRBrainTumor1')

        try:
            # four up view
            layoutManager = slicer.app.layoutManager()
            layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpView)

            # markups module
            m = slicer.util.mainWindow()
            m.moduleSelector().selectModule('Markups')

            # add ruler 1
            rulerNode1 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
            rulerNode1.SetName("d1")
            rulerNode1.AddControlPoint(-7.59519, 43.544, 28.6)
            rulerNode1.AddControlPoint(-5.56987, 14.177, 28.6)
            self.delayDisplay("Ruler 1")

            # add ruler 2
            rulerNode1 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
            rulerNode2.SetName("d2")
            rulerNode2.AddControlPoint(-3.54455, 27.656, 13.1646)
            rulerNode2.AddControlPoint(-2.5319, 27.656, 47.5949)
            self.delayDisplay("Ruler 2")

            # scroll
            markupsLogic = slicer.modules.markups.logic()
            markupsLogic.JumpSlicesToNthPointInMarkup(rulerNode1.GetID(), 0)

            # show slices
            redWidget = layoutManager.sliceWidget('Red')
            redWidget.sliceController().setSliceLink(True)
            redWidget.sliceController().setSliceVisible(True)

            self.takeScreenshot('Ruler', 'Ruler used to measure tumor diameter', -1)

            self.delayDisplay('Test passed!')
        except Exception as e:
            import traceback
            traceback.print_exc()
            self.delayDisplay('Test caused exception!\n' + str(e))

    def test_Part3PETCT(self, enableScreenshotsFlag=0, screenshotScaleFactor=1):
        """ Test using the PETCT module
        """
        self.enableScreenshots = enableScreenshotsFlag
        self.screenshotScaleFactor = screenshotScaleFactor

        self.delayDisplay("Starting the test")

        #
        # first, get some data
        #
        import SampleData
        extractPath = SampleData.downloadFromURL(
            fileNames='dataset3_PETCT.zip',
            uris=TESTING_DATA_URL + 'SHA256/11e81af3462076f4ca371b632e03ed435240042915c2daf07f80059b3f78f88d',
            checksums='SHA256:11e81af3462076f4ca371b632e03ed435240042915c2daf07f80059b3f78f88d')[0]

        self.delayDisplay("Loading PET_CT_pre-treatment.mrb")
        preTreatmentPath = extractPath + '/PET_CT_pre-treatment.mrb'
        slicer.util.loadScene(preTreatmentPath)
        self.takeScreenshot('PETCT-LoadedPre', 'Loaded pre-treatement scene', -1)

        try:
            mainWindow = slicer.util.mainWindow()
            layoutManager = slicer.app.layoutManager()
            threeDView = layoutManager.threeDWidget(0).threeDView()
            redWidget = layoutManager.sliceWidget('Red')
            redController = redWidget.sliceController()
            greenWidget = layoutManager.sliceWidget('Green')
            greenController = greenWidget.sliceController()
            yellowWidget = layoutManager.sliceWidget('Yellow')
            yellowController = yellowWidget.sliceController()
            viewNode = threeDView.mrmlViewNode()
            cameras = slicer.util.getNodes('vtkMRMLCameraNode*')
            for cameraNode in cameras.values():
                if cameraNode.GetActiveTag() == viewNode.GetID():
                    break

            threeDView.resetFocalPoint()
            slicer.util.clickAndDrag(threeDView, button='Right')
            redWidget.sliceController().setSliceVisible(True)
            yellowWidget.sliceController().setSliceVisible(True)
            self.takeScreenshot('PETCT-ConfigureView', 'Configure View', -1)

            mainWindow.moduleSelector().selectModule('Volumes')
            compositNode = redWidget.mrmlSliceCompositeNode()
            compositNode.SetForegroundOpacity(0.2)
            self.takeScreenshot('PETCT-ShowVolumes', 'Show Volumes with lesion', -1)

            compositNode.SetForegroundOpacity(0.5)
            self.takeScreenshot('PETCT-CTOpacity', 'CT1 volume opacity to 0.5', -1)

            yellowWidget.sliceController().setSliceVisible(False)
            greenWidget.sliceController().setSliceVisible(True)
            self.takeScreenshot('PETCT-ShowSlices', 'Show axial and sagittal slices', -1)

            self.delayDisplay('SUV Computation')
            if not hasattr(slicer.modules, 'petstandarduptakevaluecomputation'):
                self.delayDisplay("PET SUV Computation not available, skipping the test.")
                return

            slicer.util.selectModule('PETStandardUptakeValueComputation')

            parameters = {
                "PETDICOMPath": extractPath + '/' + 'PET1',
                "PETVolume": slicer.util.getNode('PET1'),
                "VOIVolume": slicer.util.getNode('PET1-label'),
            }

            suvComputation = slicer.modules.petstandarduptakevaluecomputation
            self.CLINode1 = None
            self.CLINode1 = slicer.cli.runSync(suvComputation, self.CLINode1, parameters, delete_temporary_files=False)

            # close the scene
            slicer.mrmlScene.Clear(0)

            self.delayDisplay("Loading PET_CT_post-treatment.mrb")
            postTreatmentPath = extractPath + '/PET_CT_post-treatment.mrb'
            slicer.util.loadScene(postTreatmentPath)
            self.takeScreenshot('PETCT-LoadedPost', 'Loaded post-treatement scene', -1)

            compositNode.SetForegroundOpacity(0.5)
            self.takeScreenshot('PETCT-CT2Opacity', 'CT2 volume opacity to 0.5', -1)

            redController.setSliceOffsetValue(-165.01)
            self.takeScreenshot('PETCT-LarynxUptake', 'Mild uptake in the larynx and pharynx', -1)

            redController.setSliceOffsetValue(-106.15)
            self.takeScreenshot('PETCT-TumorUptake', 'No uptake in the area of the primary tumor', -1)

            self.delayDisplay('Test passed!')
        except Exception as e:
            import traceback
            traceback.print_exc()
            self.delayDisplay('Test caused exception!\n' + str(e))

    def test_Part2ChangeTracker(self, enableScreenshotsFlag=0, screenshotScaleFactor=1):
        """ Test the ChangeTracker module
        """
        self.enableScreenshots = enableScreenshotsFlag
        self.screenshotScaleFactor = screenshotScaleFactor

        self.delayDisplay("Starting the test")

        if not hasattr(slicer.modules, 'changetracker'):
            self.delayDisplay("ChangeTracker not available, skipping the test.")
            return

        #
        # first, get some data
        #
        import SampleData
        SampleData.downloadFromURL(
            fileNames='ChangeTrackerScene.mrb',
            loadFiles=True,
            uris=TESTING_DATA_URL + 'SHA256/64734cbbf8ebafe4a52f551d1510a8f6f3d0625eb5b6c1e328be117c48e2c653',
            checksums='SHA256:64734cbbf8ebafe4a52f551d1510a8f6f3d0625eb5b6c1e328be117c48e2c653')
        self.takeScreenshot('ChangeTracker-Loaded', 'Finished with download and loading', -1)

        try:
            mainWindow = slicer.util.mainWindow()
            layoutManager = slicer.app.layoutManager()
            threeDView = layoutManager.threeDWidget(0).threeDView()
            redWidget = layoutManager.sliceWidget('Red')
            redController = redWidget.sliceController()
            viewNode = threeDView.mrmlViewNode()
            cameras = slicer.util.getNodes('vtkMRMLCameraNode*')
            for cameraNode in cameras.values():
                if cameraNode.GetActiveTag() == viewNode.GetID():
                    break

            self.delayDisplay('Configure Module')
            mainWindow.moduleSelector().selectModule('ChangeTracker')
            self.takeScreenshot('ChangeTracker-ModuleGUI', 'Select the ChangeTracker module', -1)

            changeTracker = slicer.modules.changetracker.widgetRepresentation().self()

            baselineNode = slicer.util.getNode('2006-spgr1')
            followupNode = slicer.util.getNode('2007-spgr1')
            changeTracker.selectScansStep._ChangeTrackerSelectScansStep__baselineVolumeSelector.setCurrentNode(baselineNode)
            changeTracker.selectScansStep._ChangeTrackerSelectScansStep__followupVolumeSelector.setCurrentNode(followupNode)
            self.takeScreenshot('ChangeTracker-SetInputs', 'Select input scans', -1)

            changeTracker.workflow.goForward()
            self.takeScreenshot('ChangeTracker-GoForward', 'Go Forward', -1)

            slicer.util.clickAndDrag(redWidget, button='Right')
            self.takeScreenshot('ChangeTracker-Zoom', 'Inspect - zoom', -1)

            slicer.util.clickAndDrag(redWidget, button='Middle')
            self.takeScreenshot('ChangeTracker-Pan', 'Inspect - pan', -1)

            for offset in range(-20, 20, 2):
                redController.setSliceOffsetValue(offset)
            self.takeScreenshot('ChangeTracker-Scroll', 'Inspect - scroll', -1)

            self.delayDisplay('Set ROI')
            roi = changeTracker.defineROIStep._ChangeTrackerDefineROIStep__roi
            roi.SetXYZ(-2.81037, 28.7629, 28.4536)
            self.takeScreenshot('ChangeTracker-SetROICenter', 'Center VOI', -1)
            roi.SetRadiusXYZ(22.6467, 22.6804, 22.9897)
            self.takeScreenshot('ChangeTracker-SetROIExtent', 'Resize the VOI', -1)

            layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalWidescreenView)
            self.takeScreenshot('ChangeTracker-ConventionalWidescreen', 'Select the viewing mode Conventional Widescreen', -1)

            slicer.util.clickAndDrag(redWidget, button='Right')
            self.takeScreenshot('ChangeTracker-ZoomVOI', 'Zoom', -1)

            layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpView)
            self.takeScreenshot('ChangeTracker-FourUpLayout', 'Go back to Four-Up layout', -1)

            changeTracker.workflow.goForward()
            self.takeScreenshot('ChangeTracker-GoForward', 'Go Forward', -1)

            changeTracker.segmentROIStep._ChangeTrackerSegmentROIStep__threshRange.minimumValue = 120
            self.takeScreenshot('ChangeTracker-Threshold', 'Set threshold', -1)

            changeTracker.workflow.goForward()
            self.takeScreenshot('ChangeTracker-GoForward', 'Go Forward', -1)

            checkList = changeTracker.analyzeROIStep._ChangeTrackerAnalyzeROIStep__metricCheckboxList
            index = list(checkList.values()).index('IntensityDifferenceMetric')
            list(checkList.keys())[index].checked = True
            self.takeScreenshot('ChangeTracker-PickMetric', 'Select the ROI analysis method', -1)

            changeTracker.workflow.goForward()
            self.takeScreenshot('ChangeTracker-GoForward', 'Go Forward', -1)

            self.delayDisplay('Look!')
            redWidget.sliceController().setSliceVisible(True)

            self.delayDisplay('Crosshairs')
            compareWidget = layoutManager.sliceWidget('Compare1')
            style = compareWidget.interactorStyle()
            interactor = style.GetInteractor()
            for step in range(100):
                interactor.SetEventPosition(10, step)
                style.OnMouseMove()

            self.delayDisplay('Zoom')
            slicer.util.clickAndDrag(compareWidget, button='Right')

            self.delayDisplay('Pan')
            slicer.util.clickAndDrag(compareWidget, button='Middle')

            self.delayDisplay('Inspect - scroll')
            compareController = redWidget.sliceController()
            for offset in range(10, 30, 2):
                compareController.setSliceOffsetValue(offset)

            self.takeScreenshot('ChangeTracker-InspectResults', 'Inspected results', -1)

            self.delayDisplay('Test passed!')
        except Exception as e:
            import traceback
            traceback.print_exc()
            self.delayDisplay('Test caused exception!\n' + str(e))
