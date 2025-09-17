import slicer
import os
import sys

from slicer.ScriptedLoadableModule import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, this_dir)

from SlicerStartupModuleTestHelperPackage import *

class SlicerStartupModuleTestHelperModule(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "SlicerStartupModuleTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.contributors = ["Rafael Palomar (Oslo University Hospital), Jean-Christophe Fillion-Robin (Kitware), Andras Lasso (PerkLab)"]
        self.parent.widgetRepresentationCreationEnabled = False

        self.testOutputFileName = os.environ["SLICER_STARTUP_MODULE_TEST_OUTPUT"]
        if os.path.isfile(self.testOutputFileName):
            os.remove(self.testOutputFileName)

        print("SlicerStartupModuleTestHelperModule initialized")
