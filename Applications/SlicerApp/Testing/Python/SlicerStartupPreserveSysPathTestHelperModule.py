import os
import sys

from slicer.ScriptedLoadableModule import ScriptedLoadableModule

# Ensure the helper package is importable during module load, so its
# __init__ can modify sys.path immediately.
currentDir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(currentDir)

from SlicerStartupPreserveSysPathTestHelperPackage import *

class SlicerStartupPreserveSysPathTestHelperModule(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "SlicerStartupPreserveSysPathTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.contributors = ["Rafael Palomar (Oslo University Hospital), Jean-Christophe Fillion-Robin (Kitware), Andras Lasso (PerkLab)"]
        self.parent.widgetRepresentationCreationEnabled = False

        self.testOutputFileName = os.environ["SLICER_STARTUP_MODULE_TEST_OUTPUT"]
        if os.path.isfile(self.testOutputFileName):
            os.remove(self.testOutputFileName)

        print("SlicerStartupPreserveSysPathTestHelperModule initialized")
