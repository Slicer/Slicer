import os
import glob
from __main__ import qt
from __main__ import vtk
from __main__ import ctk
from __main__ import slicer

#
# SelfTests
#
# This code includes the GUI for the slicer module.  
# The purpose is to provide a BIST (http://en.wikipedia.org/wiki/Built-in_self-test)
# framework for slicer as discussed here: http://na-mic.org/Bug/view.php?id=1922
#

class SelfTests:
  def __init__(self, parent):
    import string
    parent.title = "SelfTests"
    parent.categories = ["Testing",]
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = string.Template("""
The SelfTests module allows developers to provide built-in self-tests (BIST) for slicer so that users can tell if their installed version of slicer are running as designed.  See <a href=\"$a/Documentation/$b.$c/Modules/SelfTests\">$a/Documentation/$b.$c/Modules/SelfTests</a> for more information.
""").substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
    <img src=':Logos/Isomics.png'><br><br>
This work is supported by NA-MIC, NAC, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
    """
    #parent.icon = qt.QIcon(':Icons/Medium/SlicerLoadSelfTests.png')
    self.parent = parent

#
# SelfTests widget
#

class SelfTestsWidget:
  """
  Slicer module that creates the Qt GUI for interacting with SelfTests
  """

  def __init__(self, parent=None):
    
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

    globals()['selfTests'] = self

  def enter(self):
    pass

  def exit(self):
    pass

  def updateGUIFromMRML(self, caller, event):
    pass

  # sets up the widget
  def setup(self):

    #
    # test list 
    #

    self.testList = ctk.ctkCollapsibleButton(self.parent)
    self.testList.setLayout(qt.QVBoxLayout())
    self.testList.setText("Self Tests")
    self.layout.addWidget(self.testList)
    self.testList.collapsed = False

    self.runAll = qt.QPushButton("Run All")
    self.testList.layout().addWidget(self.runAll)
    self.runAll.connect('clicked()', self.onRunAll)

    # Add spacer to layout
    self.layout.addStretch(1)

  def onRunAll(self):
    print("Running all tests...")

  def question(self,text,title='SelfTests'):
    return qt.QMessageBox.question(slicer.util.mainWindow(), title, text, 0x14000) == 0x4000

  def okayCancel(self,text,title='SelfTests'):
    return qt.QMessageBox.question(slicer.util.mainWindow(), title, text, 0x400400) == 0x400

def SelfTestsTest():
  w = slicer.modules.selftests.widgetRepresentation()
  runAllButton = slicer.util.findChildren(w.dicomApp, text='Run All')[0]
  runAllButton.click()


  print("SelfTestsTest Passed!")
  return True

def SelfTestsDemo():
  pass

if __name__ == "__main__":
  import sys
  if '--test' in sys.argv:
    if SelfTestsTest():
      exit(0)
    exit(1)
  if '--demo' in sys.argv:
    SelfTestsDemo()
    exit()
  # TODO - 'exit()' returns so this code gets run
  # even if the argument matches one of the cases above
  #print ("usage: SelfTests.py [--test | --demo]")
