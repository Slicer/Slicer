import os
import unittest
import math
import datetime
import time
import random
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *


#
# WebEngine
#

class WebEngine(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "WebEngine"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = """
    Module to test WebEngine.
    """
    parent.acknowledgementText = """
    This file was originally developed by Steve Pieper and was partially funded by NSF grant 1759883
""" # replace with organization, grant and thanks.

#
# qWebEngineWidget
#

class WebEngineWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModuleWidget.__init__(self, parent)
    self.webWidgets = [] # hold references so windows persist

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

    # Collapsible button
    sitesCollapsibleButton = ctk.ctkCollapsibleButton()
    sitesCollapsibleButton.text = "Sample Sites"
    self.layout.addWidget(sitesCollapsibleButton)

    # Layout within the collapsible button
    sitesFormLayout = qt.QFormLayout(sitesCollapsibleButton)

    # site buttons
    buttons = []
    self.sites = [
      {
        "label": "Web Console", "url": "http://localhost:1337"
      },
      {
        "label": "Crowds Cure Cancer", "url": "http://cancer.crowds-cure.org"
      },
      {
        "label": "Slicer Home Page", "url": "http://slicer.org"
      },
      {
        "label": "MorphoSource", "url": "https://www.morphosource.org"
      },
      {
        "label": "Slicer SampleData", "url": "https://www.slicer.org/wiki/SampleData"
      },
      {
        "label": "SlicerMorph", "url": "https://slicermorph.github.io"
      },
    ]
    for site in self.sites:
      button = qt.QPushButton(site["label"])
      button.toolTip = "Open %s" % site["url"]
      sitesFormLayout.addWidget(button)
      onClick = lambda click, site=site: self.onSiteButtonClicked(site)
      button.connect('clicked(bool)', onClick)
      buttons.append(button)

    button = qt.QPushButton("Close All")
    button.toolTip = "Close all the web views"
    button.connect('clicked(bool)', self.onCloseAll)
    self.layout.addWidget(button)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onSiteButtonClicked(self, site):
    webWidget = slicer.qSlicerWebWidget()
    slicerGeometry = slicer.util.mainWindow().geometry
    webWidget.size = qt.QSize(1536,1024)
    webWidget.pos = qt.QPoint(slicerGeometry.x() + 256, slicerGeometry.y() + 128)
    webView = webWidget.webView()
    webView.url = qt.QUrl(site["url"])
    webWidget.show()
    self.webWidgets.append(webWidget)

  def onCloseAll(self):
    for widget in self.webWidgets:
      del widget
    self.webWidgets = []


class WebEngineTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    self.gotResponse = False

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_WebEngine1()

  def onEvalResult(self, js, result):
    if js == "valueFromSlicer;":
      if result != "42":
        Exception("Did not get back expected result!")
      self.delayDisplay("Got the expected result back from JavaScript")
    else:
      self.delayDisplay("Got a result back from JavaScript")
      self.gotResponse = True
      print(js, result)


  def test_WebEngine1(self):
    """ Testing WebEngine
    """

    self.delayDisplay("Starting the test")


    webWidget = slicer.qSlicerWebWidget()
    webWidget.size = qt.QSize(1024,512)
    webView = webWidget.webView()
    webWidget.show()
    self.delayDisplay('Showing widget')



    webWidget.connect("evalResult(QString,QString)", self.onEvalResult)

    webWidget.evalJS("""
        const paragraph = document.createElement('p');
        paragraph.innerText = 'Hello from Slicer!';
        document.body.appendChild(paragraph);
    """)

    self.delayDisplay('Slicer should be saying hello!')

    self.delayDisplay('Slicer setting a javascript value')

    webWidget.evalJS("const valueFromSlicer = 42;")
    webWidget.evalJS("valueFromSlicer;");

    iteration = 0
    while not self.gotResponse:
      self.delayDisplay('Waiting for response...')
    webWidget.disconnect("evalResult(QString,QString)", self.onEvalResult)

    if iteration >= 10:
      raise Exception("Never got response from evalJS")


    #
    # Note - calling slicerPython code from JavaScript code works when
    # executed from the web page, but oddly not from within the
    # context of a self test, so the following tests are disabled for now.
    #

    if False:

        self.delayDisplay('Call a python method')

        webWidget.evalJS("""
            new QWebChannel(qt.webChannelTransport, channel => {
                const slicerPython = channel.objects.slicerPython;

                let pythonCode = "dialog = qt.QInputDialog(slicer.util.mainWindow())\n";
                pythonCode += "dialog.setLabelText('hello')\n";
                pythonCode += "dialog.open()\n";
                pythonCode += "qt.QTimer.singleShot(1000, dialog.close)\n";

                slicerPython.evalPython(pythonCode);
            });
        """)



        self.delayDisplay('Test access to python via js')

        if hasattr(slicer.modules, 'slicerPythonValueFromJS'):
          del slicer.modules.slicerPythonValueFromJS

        webWidget.evalJS("""
            new QWebChannel(qt.webChannelTransport, channel => {
                channel.objects.slicerPython.evalPython("slicer.modules.slicerPythonValueFromJS = 42");
            });
        """)

        iteration = 0
        while not hasattr(slicer.modules, 'slicerPythonValueFromJS') and iteration < 5:
          self.delayDisplay('Waiting for python value from JS...')
          iteration += 1

        if iteration >= 5:
          raise Exception("Couldn't get python value back from JS")

        del slicer.modules.slicerPythonValueFromJS


        self.delayDisplay('Value of %d receieved via javascipt' % slicerPythonValueFromJS)


    self.delayDisplay('Test passed!')
