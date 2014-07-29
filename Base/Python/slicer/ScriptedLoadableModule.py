import os
import unittest
from __main__ import qt, ctk, slicer

class ScriptedLoadableModule:
  def __init__(self, parent):
    parent.title = ""
    parent.categories = []
    parent.dependencies = []
    parent.contributors = []
    parent.helpText = ""
    parent.acknowledgementText = ""
    self.parent = parent

    self.moduleName = self.__class__.__name__

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests[self.moduleName] = self.runTest

  def runTest(self):
    # Name of the tester class is expected to be MyModuleNameTest()
    tester = eval(self.moduleName + 'Test()')
    tester.runTest()

class ScriptedLoadableModuleWidget:
  def __init__(self, parent = None):
    # Get module name by strippint 'Widget' from the class name
    self.moduleName = self.__class__.__name__
    if self.moduleName.endswith('Widget'):
      self.moduleName = self.moduleName[:-6]
    settings = qt.QSettings()
    self.developerMode = settings.value('Developer/DeveloperMode').lower() == 'true'
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setupDeveloperSection(self):

    if not self.developerMode:
      return

    #
    # Reload and Test area
    #
    reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "ScriptedLoadableModuleTemplate Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    reloadFormLayout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

  def setup(self):
    # Instantiate and connect default widgets ...
    self.setupDeveloperSection()

  def cleanup(self):
    pass

  def onReload(self):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[self.moduleName] = slicer.util.reloadScriptedModule(self.moduleName)

  def onReloadAndTest(self):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (self.moduleName, self.moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(),
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")

class ScriptedLoadableModuleLogic():

  def delayDisplay(self,message,msec=1000):
    #
    # logic version of delay display
    #
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

class ScriptedLoadableModuleTest(unittest.TestCase):
  """
  Base class for module tester class.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.delayDisplay('No test is defined in '+self.__class__.__name__)

