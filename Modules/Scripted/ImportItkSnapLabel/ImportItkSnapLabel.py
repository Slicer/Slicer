import os
import logging
import slicer
from slicer.ScriptedLoadableModule import *


#
# ImportItkSnapLabel
#

class ImportItkSnapLabel(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "ImportItkSnapLabel"
        self.parent.categories = ["Informatics"]
        self.parent.dependencies = []
        self.parent.contributors = ["Andras Lasso (PerkLab)"]
        self.parent.helpText = """Load ITK-Snap label description file (.label or .txt)."""
        self.parent.acknowledgementText = """This file was originally developed by Andras Lasso, PerkLab."""
        # don't show this module - it is only for registering a reader
        parent.hidden = True


#
# Reader plugin
# (identified by its special name <moduleName>FileReader)
#

class ImportItkSnapLabelFileReader:

    def __init__(self, parent):
        self.parent = parent

    def description(self):
        return 'ITK-Snap Label Description'

    def fileType(self):
        return 'ItkSnapLabel'

    def extensions(self):
        return ['ITK-Snap label description file (*.label)', 'ITK-Snap label description file (*.txt)']

    def canLoadFile(self, filePath):
        try:
            colors = ImportItkSnapLabelFileReader.parseLabelFile(filePath)
            if not colors:
                return False
        except Exception as e:
            return False
        return True

    def load(self, properties):
        try:
            filePath = properties['fileName']
            colors = ImportItkSnapLabelFileReader.parseLabelFile(filePath)

            maxColorIndex = -1
            for color in colors:
                maxColorIndex = max(maxColorIndex, color['index'])

            filenameWithoutExtension = os.path.splitext(os.path.basename(filePath))[0]
            name = slicer.mrmlScene.GenerateUniqueName(filenameWithoutExtension)
            colorNode = slicer.mrmlScene.CreateNodeByClass("vtkMRMLColorTableNode")
            colorNode.UnRegister(None)  # to prevent memory leaks
            colorNode.SetName(name)
            colorNode.SetAttribute("Category", "Segmentation")
            colorNode.SetTypeToUser()
            colorNode.SetNumberOfColors(maxColorIndex + 1)
            # The color node is a procedural color node, which is saved using a storage node.
            # Hidden nodes are not saved if they use a storage node, therefore
            # the color node must be visible.
            colorNode.SetHideFromEditors(False)

            colorNode.SetNamesInitialised(True)  # prevent automatic color name generation
            for color in colors:
                colorNode.SetColor(color['index'], color['name'], color['r'], color['g'], color['b'], color['a'])

            slicer.mrmlScene.AddNode(colorNode)

        except Exception as e:
            logging.error('Failed to load file: ' + str(e))
            import traceback
            traceback.print_exc()
            return False

        self.parent.loadedNodes = [colorNode.GetID()]
        return True

    @staticmethod
    def parseLabelFile(filename):
        """Read ITK-Snap label file header.
        File format description:
        ################################################
        # ITK-SnAP Label Description File
        # File format:
        # IDX   -R-  -G-  -B-  -A--  VIS MSH  LABEL
        # Fields:
        #    IDX:   Zero-based index
        #    -R-:   Red color component (0..255)
        #    -G-:   Green color component (0..255)
        #    -B-:   Blue color component (0..255)
        #    -A-:   Label transparency (0.00 .. 1.00)
        #    VIS:   Label visibility (0 or 1)
        #    IDX:   Label mesh visibility (0 or 1)
        #  LABEL:   Label description
        ################################################
        """

        import re
        commentLineRegex = re.compile(r'^\s*#(.*)')
        # Color line: index, r, g, b, a, label visibility, mesh visibility, description
        # Example:
        #     1   255    0    0        1  1  1    "Label 1"
        colorLineRegex = re.compile(r'^\s*(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+([01]+)\s+([01]+)\s+([01]+)\s+\"([^\"]*)\"')
        colors = []

        lineIndex = 0
        with open(filename) as fileobj:
            for line in fileobj:
                lineIndex += 1
                commentLine = commentLineRegex.search(line)
                if commentLine:
                    continue
                colorLine = colorLineRegex.search(line)
                if colorLine:
                    fields = colorLine.groups()
                    color = {'index': int(fields[0]),
                             'r': int(fields[1]) / 255.0, 'g': int(fields[2]) / 255.0, 'b': int(fields[3]) / 255.0, 'a': float(fields[4]),
                             'labelVis': int(fields[5]) != 0, 'meshVis': int(fields[6]) != 0,
                             'name': fields[7]}
                    colors.append(color)
                    continue
                raise ValueError(f"Syntax error in line {lineIndex}")

        return colors


#
# ImportItkSnapLabelTest
#

class ImportItkSnapLabelTest(ScriptedLoadableModuleTest):

    def setUp(self):
        slicer.mrmlScene.Clear()

    def runTest(self):
        self.setUp()
        self.test_ImportItkSnapLabel1()

    def test_ImportItkSnapLabel1(self):

        self.delayDisplay("Loading test image as label")
        testDataPath = os.path.join(os.path.dirname(__file__), 'Resources')
        labelFilePath = os.path.join(testDataPath, 'Untitled.label')
        node = slicer.util.loadNodeFromFile(labelFilePath, 'ItkSnapLabel')
        self.assertIsNotNone(node)

        self.delayDisplay('Checking loaded label')
        self.assertEqual(node.GetNumberOfColors(), 7)

        self.delayDisplay('Test passed')
