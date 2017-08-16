import slicer

from slicer.ScriptedLoadableModule import *


class SlicerStartupCompletedTest(ScriptedLoadableModule):

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "SlicerStartupCompletedTest"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)"]
    self.parent.widgetRepresentationCreationEnabled = False

    slicer.app.connect("startupCompleted()", self.onStartupCompleted)

  def onStartupCompleted(self):
    print("StartupCompleted emitted")

