import traceback
from __main__ import qt
from __main__ import ctk
from __main__ import slicer

#
# SelfTests
#
# This code includes the GUI for the slicer module.  
# The purpose is to provide a BIST (http://en.wikipedia.org/wiki/Built-in_self-test)
# framework for slicer as discussed here: http://na-mic.org/Bug/view.php?id=1922
#

class ExampleSelfTests(object):

  @staticmethod
  def closeScene():
    """Close the scene"""
    slicer.mrmlScene.Clear(0)




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
    # slicer.selfTests is a dictionary of tests that are registered
    # here or in other parts of the code.  The key is the name of the test
    # and the value is a python callable that runs the test and returns
    # if the test passed or raises and exception if it fails.
    # the __doc__ attribute of the test is used as a tooltip for the test
    # button.
    #
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}

    # register the example tests
    slicer.selfTests['MRMLSceneExists'] = lambda : slicer.app.mrmlScene
    slicer.selfTests['CloseScene'] = ExampleSelfTests.closeScene

#
# SelfTests widget
#

class SelfTestsWidget:
  """
  Slicer module that creates the Qt GUI for interacting with SelfTests
  """

  def __init__(self, parent=None):
    self.logic = SelfTestsLogic(slicer.selfTests)
    
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

    self.testButtons = {}
    self.testMapper = qt.QSignalMapper() 
    self.testMapper.connect('mapped(const QString&)', self.onRun)
    for test in slicer.selfTests.keys():
      self.testButtons[test] = qt.QPushButton(test)
      self.testButtons[test].setToolTip(slicer.selfTests[test].__doc__)
      self.testList.layout().addWidget(self.testButtons[test])
      self.testMapper.setMapping(self.testButtons[test],test)
      self.testButtons[test].connect('clicked()', self.testMapper, 'map()')

    # Add spacer to layout
    self.layout.addStretch(1)

  def onRunAll(self):
    self.logic.run(continueCheck=self.continueCheck)
    self.information(self.logic)

  def onRun(self,test):
    self.logic.run([test,], continueCheck=self.continueCheck)

  def continueCheck(self,logic):
    slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)
    return True

  def information(self,text,title='SelfTests'):
    qt.QMessageBox.information(slicer.util.mainWindow(), title, text)

  def question(self,text,title='SelfTests'):
    return qt.QMessageBox.question(slicer.util.mainWindow(), title, text, 0x14000) == 0x4000

  def okayCancel(self,text,title='SelfTests'):
    return qt.QMessageBox.question(slicer.util.mainWindow(), title, text, 0x400400) == 0x400

class SelfTestsLogic:
  """Logic to handle invoking the tests and reporting the results"""

  def __init__(self,selfTests):
    self.selfTests = selfTests
    self.results = {}
    self.passed = []
    self.failed = []

  def __str__(self):
    testsRun = len(self.results.keys())
    if testsRun == 0:
      return "No tests run"
    s = "%.0f%% passed (%d of %d)" % (
        (100. * len(self.passed) / testsRun), 
        len(self.passed), testsRun )
    s +="\n---\n"
    for test in self.results:
      s += "%s\t%s\n" % (test, self.results[test])
    return s

  def run(self,tests=None,continueCheck=None):
    if not tests:
      tests = self.selfTests.keys()

    for test in tests:
      try:
        result = self.selfTests[test]()
        self.passed.append(test)
      except Exception, e:
        traceback.print_exc()
        result = "Failed with: %s" % e
        self.failed.append(test)
      self.results[test] = result
      if continueCheck:
        if not continueCheck(self):
          return

def SelfTestsTest():
  if hasattr(slicer,'selfTests'):
    logic = SelfTestsLogic(slicer.selfTests.keys())
    logic.run()
  print(logic.results)
  print("SelfTestsTest Passed!")
  return logic.failed == []

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
