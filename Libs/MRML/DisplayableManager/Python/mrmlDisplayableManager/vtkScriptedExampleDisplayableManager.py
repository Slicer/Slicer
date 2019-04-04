from __future__ import print_function

#
# ScriptedExampleDisplayableManager
#

#
# Since it's not possible to derive a VTK class in python, all scripted DisplayableManager
# should expose the following methods:
#    - Create
#    - GetMRMLSceneEventsToObserve
#    - ProcessMRMLSceneEvents
#    - ProcessMRMLNodesEvents
#    - RemoveMRMLObservers
#    - UpdateFromMRML
#    - OnInteractorStyleEvent
#    - OnMRMLDisplayableNodeModifiedEvent
#
# The constructor has one parameter named 'parent' corresponding to the associated instance of
# vtkScriptedDisplayableManager in the C++ world.
#
# The python methods listed above corresponds to the implementation of the virtual method
# available in vtkScriptedDisplayableManager.
#
# The only exception is the virtual method SetMRMLSceneInternal, the python class only needs to
# implement the method GetMRMLSceneEventsToObserve. This later one just return a list of integer
# representing the eventid to observe.
#
# It's also possible to access the API of the associated C++ instance using the self.Parent
# For example:
#   self.Parent.RemoveInteractorStyleObservableEvent(26) # vtkCommand::MouseWheelForwardEvent
#
# Make also sure NOT to call the corresponding C++ method from it's python equivalent, it will
# result in an infinite loop.
# The following statement will likely lead to an unstable state:
#    def Create(self): self.Parent.Create()
#
# If a a method isn't implemented, the following syntax should be used:
#   def Create(self): pass
#
# NOTE
#   Ideally, a DisplayableManager should deal only with MRMLNodes. Incriminated code should
# be moved either in the DisplayableManager itself, in the associated MRML Node or
# in a MRMLNode helper class.
#
# TODO
#   While porting existing code, to overcome this problem, the following need to be done:
#     - DisplayableManager abstract base class should have a reference to the current MRMLApplicationLogic
#     - The MRMLApplicationLogic should contain a map of logics
#     - The list of logic internally used by the qSlicerLayoutManager should be removed and
#     the list from the MRMLApplicationLogic used instead.

class vtkScriptedExampleDisplayableManager(object):

  def __init__(self, parent):
    self.Parent = parent
    print("vtkScriptedExampleDisplayableManager - __init__")

  def Create(self):
    print("vtkScriptedExampleDisplayableManager - Create")
    pass

  def GetMRMLSceneEventsToObserve(self):
    print("vtkScriptedExampleDisplayableManager - GetMRMLSceneEventsToObserve")
    sceneEvents = vtkIntArray()
    sceneEvents.InsertNextValue(slicer.vtkMRMLScene.NodeAddedEvent)
    sceneEvents.InsertNextValue(slicer.vtkMRMLScene.NodeRemovedEvent)
    return sceneEvents

  def ProcessMRMLSceneEvents(self, scene, eventid, node):
    print("vtkScriptedExampleDisplayableManager - ProcessMRMLSceneEvents(eventid,", eventid, ")")
    pass

  def ProcessMRMLNodesEvents(self, scene, eventid, callData):
    print("vtkScriptedExampleDisplayableManager - ProcessMRMLNodesEvents(eventid,", eventid, ")")
    pass

  def RemoveMRMLObservers(self):
    print("vtkScriptedExampleDisplayableManager - RemoveMRMLObservers")
    pass

  def UpdateFromMRML(self):
    print("vtkScriptedExampleDisplayableManager - UpdateFromMRML")
    pass

  def OnInteractorStyleEvent(self, eventid):
    print("vtkScriptedExampleDisplayableManager - OnInteractorStyleEvent(eventid,", eventid, ")")

  def OnMRMLDisplayableNodeModifiedEvent(self, viewNode):
    print("vtkScriptedExampleDisplayableManager - onMRMLDisplayableNodeModifiedEvent")




