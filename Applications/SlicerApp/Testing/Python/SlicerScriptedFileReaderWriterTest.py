import logging
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

class SlicerScriptedFileReaderWriterTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = 'SlicerScriptedFileReaderWriterTest'
    parent.categories = ['Testing.TestCases']
    parent.dependencies = []
    parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
    parent.helpText = '''
    This module is used to test qSlicerScriptedFileReader and qSlicerScriptedFileWriter classes.
    '''
    parent.acknowledgementText = '''
    This file was originally developed by Andras Lasso, PerkLab.
    '''
    self.parent = parent

class SlicerScriptedFileReaderWriterTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Default reload&test widgets are enough.
    # Note that reader and writer is not reloaded.

class SlicerScriptedFileReaderWriterTestFileReader(object):

  def __init__(self, parent):
    self.parent = parent

  def description(self):
    return 'My file type'

  def fileType(self):
    return 'MyFileType'

  def extensions(self):
    return ['My file type (*.mft)']

  def canLoadFile(self, filePath):
    firstLine = ''
    with open(filePath, 'r') as f:
      firstLine = f.readline()
    validFile = 'magic' in firstLine
    return validFile

  def load(self, properties):
    try:
      filePath = properties['fileName']

      # Get node base name from filename
      if 'name' in properties.keys():
        baseName = properties['name']
      else:
        baseName = os.path.splitext(os.path.basename(filePath))[0]
        baseName = slicer.mrmlScene.GenerateUniqueName(baseName)

      # Read file content
      with open (filePath, 'r') as myfile:
        data = myfile.readlines()

      # Check if file is valid
      firstLine = data[0].rstrip()
      if firstLine != 'magic':
        raise ValueError('Cannot read file, it is expected to start with magic')

      # Load content into new node
      loadedNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLTextNode', baseName)
      loadedNode.SetText(''.join(data[1:]))

    except Exception as e:
      logging.error('Failed to load file: '+str(e))
      import traceback
      traceback.print_exc()
      return False

    self.parent.loadedNodes = [loadedNode.GetID()]
    return True


class SlicerScriptedFileReaderWriterTestFileWriter(object):

  def __init__(self, parent):
    self.parent = parent

  def description(self):
    return 'My file type'

  def fileType(self):
    return 'MyFileType'

  def extensions(self, obj):
    return ['My file type (*.mft)']

  def canWriteObject(self, obj):
    return bool(obj.IsA("vtkMRMLTextNode"))

  def write(self, properties):
    try:

      # Get node
      node = slicer.mrmlScene.GetNodeByID(properties["nodeID"])

      # Write node content to file
      filePath = properties['fileName']
      with open (filePath, 'w') as myfile:
        myfile.write("magic\n")
        myfile.write(node.GetText())

    except Exception as e:
      logging.error('Failed to write file: '+str(e))
      import traceback
      traceback.print_exc()
      return False

    self.parent.writtenNodes = [node.GetID()]
    return True


class SlicerScriptedFileReaderWriterTestTest(ScriptedLoadableModuleTest):
  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_Writer()
    self.test_Reader()
    self.tearDown()
    self.delayDisplay('Testing complete')

  def setUp(self):
    self.tempDir = slicer.util.tempDirectory()
    logging.info("tempDir: " + self.tempDir)
    self.textInNode = "This is\nsome example test"
    self.validFilename = self.tempDir + "/tempSlicerScriptedFileReaderWriterTestValid.mft"
    self.invalidFilename = self.tempDir + "/tempSlicerScriptedFileReaderWriterTestInvalid.mft"
    slicer.mrmlScene.Clear()

  def tearDown(self):
    import shutil
    shutil.rmtree(self.tempDir, True)

  def test_WriterReader(self):
    # Writer and reader tests are put in the same function to ensure
    # that writing is done before reading (it generates input data for reading).

    self.delayDisplay('Testing node writer')
    slicer.mrmlScene.Clear()
    textNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLTextNode')
    textNode.SetText(self.textInNode)
    self.assertTrue(slicer.util.saveNode(textNode, self.validFilename, {'fileType': 'MyFileType'}))

    self.delayDisplay('Testing node reader')
    slicer.mrmlScene.Clear()
    loadedNode = slicer.util.loadNodeFromFile(self.validFilename, 'MyFileType')
    self.assertIsNotNone(loadedNode)
    self.assertTrue(loadedNode.IsA('vtkMRMLTextNode'))
    self.assertEqual(loadedNode.GetText(), self.textInNode)
