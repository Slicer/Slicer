import slicer

#
# Test markups node name with no custom default node added to the scene
#

# Start from fresh scene
slicer.mrmlScene.Clear()
# Create node with AddNewNodeByClass (x2) to see how name iterates
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "L":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "L_1":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node through qMRMLNodeFactory with no baseName set to see how name iterates
node_factory = slicer.qMRMLNodeFactory()
node_factory.setMRMLScene(slicer.mrmlScene)
markup_node = node_factory.createNode("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "L_2":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with AddNewNodeByClass with Name prespecified (x2) to see how name iterates
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with Markups module logic (x2) to see how name iterates
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "L_3":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "L_4":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with Markups module logic with Name prespecified (x2) to see how name iterates
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsLineNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsLineNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())

#
# Test markups node name, but with a default node added to the scene
#

# Start from fresh scene
slicer.mrmlScene.Clear()
# Define a default Markups Line node with a default node name
default_markups_line_node = slicer.vtkMRMLMarkupsLineNode()
default_markups_line_node.SetName("DefaultNodeName")  # Default names are to be used exactly and to not be manipulated through GenerateUniqueName()
slicer.mrmlScene.AddDefaultNode(default_markups_line_node)
# Create node with AddNewNodeByClass (x2) to see how name iterates
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "DefaultNodeName":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "DefaultNodeName":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node through qMRMLNodeFactory with no baseName set to see how name iterates
node_factory = slicer.qMRMLNodeFactory()
node_factory.setMRMLScene(slicer.mrmlScene)
markup_node = node_factory.createNode("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "DefaultNodeName":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with AddNewNodeByClass with Name prespecified (x2) to see how name iterates
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with Markups module logic (x2) to see how name iterates
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "DefaultNodeName":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsLineNode")
if markup_node.GetName() != "DefaultNodeName":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with Markups module logic with Name prespecified (x2) to see how name iterates
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsLineNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsLineNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())

#
# Test markups node name, but with DefaultNodeNamePrefix customized
#

# Start from fresh scene
slicer.mrmlScene.Clear()
# Define a default Markups Angle node with a default node name prefix
default_markups_angle_node = slicer.vtkMRMLMarkupsAngleNode()
default_markups_angle_node.SetDefaultNodeNamePrefix("Angle")
slicer.mrmlScene.AddDefaultNode(default_markups_angle_node)
# Create node with AddNewNodeByClass (x2) to see how name iterates
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsAngleNode")
if markup_node.GetName() != "Angle":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsAngleNode")
if markup_node.GetName() != "Angle_1":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node through qMRMLNodeFactory with no baseName set to see how name iterates
node_factory = slicer.qMRMLNodeFactory()
node_factory.setMRMLScene(slicer.mrmlScene)
markup_node = node_factory.createNode("vtkMRMLMarkupsAngleNode")
if markup_node.GetName() != "Angle_2":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with AddNewNodeByClass with Name prespecified (x2) to see how name iterates
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsAngleNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsAngleNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with Markups module logic (x2) to see how name iterates
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsAngleNode")
if markup_node.GetName() != "Angle_3":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsAngleNode")
if markup_node.GetName() != "Angle_4":
    raise Exception("Unexpected node name: " + markup_node.GetName())
# Create node with Markups module logic with Name prespecified (x2) to see how name iterates
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsAngleNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
markup_node = slicer.modules.markups.logic().AddNewMarkupsNode("vtkMRMLMarkupsAngleNode", "UseThisDefaultNameExactly")
if markup_node.GetName() != "UseThisDefaultNameExactly":
    raise Exception("Unexpected node name: " + markup_node.GetName())
