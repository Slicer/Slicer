""" This module is a place holder for convenient functions allowing to interact with CLI."""
from __future__ import print_function

def createNode(cliModule, parameters = None):
  """Creates a new vtkMRMLCommandLineModuleNode for a specific module, with
  optional parameters"""
  if not cliModule:
    return None
  cliLogic = cliModule.logic()
  if not cliLogic:
    print("Could not find logic for module '%s'" % cliModule.name)
    return None
  node = cliLogic.CreateNodeInScene()
  setNodeParameters(node, parameters)
  return node

def setNodeParameters(node, parameters):
  """Sets parameters for a vtkMRMLCommandLineModuleNode given a dictionary
  of (parameterName, parameterValue) pairs
  For vectors: provide a list, tuple or comma-separated string
  For enumerations, provide the single enumeration value
  For files and directories, provide a string
  For images, geometry, points and regions, provide a vtkMRMLNode
  """
  import slicer
  if not node:
    return None
  if not parameters:
    return None
  for key, value in parameters.items():
    if isinstance(value, str):
      node.SetParameterAsString(key, value)
    elif isinstance(value, bool):
      node.SetParameterAsBool(key, value)
    elif isinstance(value, int):
      node.SetParameterAsInt(key, value)
    elif isinstance(value, float):
      node.SetParameterAsDouble(key, value)
    elif isinstance(value, slicer.vtkMRMLNode):
      node.SetParameterAsNode(key, value)
    elif isinstance(value, list) or isinstance(value, tuple):
      commaSeparatedString = str(value)
      commaSeparatedString = commaSeparatedString[1:len(commaSeparatedString)-1]
      node.SetParameterAsString(key, commaSeparatedString)
    #TODO: file support
    else:
      print("parameter ", key, " has unsupported type ", value.__class__.__name__)

def runSync(module, node=None, parameters=None, delete_temporary_files=True, update_display=True):
  """Run a CLI synchronously, optionally given a node with optional parameters,
  returning the node (or the new one if created)
  node: existing parameter node (None by default)
  parameters: dictionary of parameters for cli (None by default)
  delete_temporary_files: remove temp files created during execution (True by default)
  update_display: show output nodes after completion
  """
  return run(module, node=node, parameters=parameters, wait_for_completion=True, delete_temporary_files=delete_temporary_files, update_display=update_display)

def run(module, node = None, parameters = None, wait_for_completion = False, delete_temporary_files = True, update_display=True):
  """Runs a CLI, optionally given a node with optional parameters, returning
  back the node (or the new one if created)
  node: existing parameter node (None by default)
  parameters: dictionary of parameters for cli (None by default)
  wait_for_completion: block if True (False by default)
  delete_temporary_files: remove temp files created during exectuion (True by default)
  update_display: show output nodes after completion
  """
  import slicer.util
  if node:
    setNodeParameters(node, parameters)
  else:
    node = createNode(module, parameters)
    if not node:
      return None

  logic = module.logic()

  logic.SetDeleteTemporaryFiles(1 if delete_temporary_files else 0)

  if wait_for_completion:
      logic.ApplyAndWait(node, update_display)
  else:
      logic.Apply(node, update_display)
  #widget = slicer.util.getModuleGui(module)
  #if not widget:
  #  print "Could not find widget representation for module"
  #  return None
  #widget.setCurrentCommandLineModuleNode(node)
  #widget.apply()
  return node

def cancel(node):
  print("Not yet implemented")
