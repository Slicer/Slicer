from __future__ import print_function
import slicer
import os

from slicer.ScriptedLoadableModule import *

class SlicerStartupCompletedTestHelperModule(ScriptedLoadableModule):

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "SlicerStartupCompletedTest"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware), Andras Lasso (PerkLab)"]
    self.parent.widgetRepresentationCreationEnabled = False

    self.testOutputFileName = os.environ['SLICER_STARTUP_COMPLETED_TEST_OUTPUT']
    if os.path.isfile(self.testOutputFileName):
     os.remove(self.testOutputFileName)

    slicer.app.connect("startupCompleted()", self.onStartupCompleted)

    print("SlicerStartupCompletedTestHelperModule initialized")

  def onStartupCompleted(self):
    print("StartupCompleted emitted")
    import os
    fd = os.open(self.testOutputFileName, os.O_RDWR|os.O_CREAT)
    os.write(fd, 'SlicerStartupCompletedTestHelperModule.py generated this file')
    os.write(fd, 'when slicer.app emitted startupCompleted() signal\n')
    os.close(fd)
