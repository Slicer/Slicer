from __main__ import vtk, qt, ctk, slicer

#
# ScriptedLoadableExtensionTemplate
#

class ScriptedLoadableExtensionTemplate:
  def __init__(self, parent):
    parent.title = "Scripted Loadable Extension Template"
    parent.categories = ["Examples"]
    parent.contributor = ""
    parent.helpText = """
    Example of scripted loadable extension.
    """
    parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
and was partially funded by NIH grant 3P41RR013218-12S1.
    """
    self.parent = parent

#
# qScriptedLoadableExtensionTemplateWidget
#

class ScriptedLoadableExtensionTemplateWidget:
  def __init__(self, parent = None):
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
    
  def setup(self):
    # Instantiate and connect widgets ...
    
    # Collapsible button
    dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    dummyCollapsibleButton.text = "A collapsible button"
    self.layout.addWidget(dummyCollapsibleButton)
    
    # Layout within the dummy collapsible button
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)
    
    # HelloWorld button
    helloWorldButton = qt.QPushButton("Hello world")
    helloWorldButton.toolTip = "Print 'Hello world' in standard ouput."
    dummyFormLayout.addWidget(helloWorldButton)
    helloWorldButton.connect('clicked(bool)', self.onHelloWorldButtonClicked)
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
    # Set local var as instance attribute
    self.helloWorldButton = helloWorldButton
    
  def onHelloWorldButtonClicked(self):
    print "Hello World !"

