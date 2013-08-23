/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLMarkupsDisplayableManager3D_h
#define __vtkMRMLMarkupsDisplayableManager3D_h

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsClickCounter.h"
#include "vtkMRMLMarkupsDisplayableManagerHelper.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

// VTK includes
#include <vtkHandleWidget.h>
#include <vtkSeedWidget.h>

class vtkMRMLMarkupsNode;
class vtkSlicerViewerWidget;
class vtkMRMLMarkupsDisplayNode;
class vtkAbstractWidget;

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsDisplayableManager3D :
    public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:

  static vtkMRMLMarkupsDisplayableManager3D *New();
  vtkTypeRevisionMacro(vtkMRMLMarkupsDisplayableManager3D, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Hide/Show a widget so that the node's display node visibility setting
  /// matches that of the widget
  void UpdateWidgetVisibility(vtkMRMLMarkupsNode* node);

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLMarkupsNode* node);
  /// Check if there are real changes between two sets of displayCoordinates
  bool GetDisplayCoordinatesChanged(double * displayCoordinates1, double * displayCoordinates2);

  /// Check if there are real changes between two sets of worldCoordinates
  bool GetWorldCoordinatesChanged(double * worldCoordinates1, double * worldCoordinates2);

  /// Convert display to world coordinates
  void GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates);
  void GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates);

  /// Convert world coordinates to local using mrml parent transform
  virtual void GetWorldToLocalCoordinates(vtkMRMLMarkupsNode *node,
                                  double *worldCoordinates, double *localCoordinates);

  /// Set mrml parent transform to widgets
  virtual void SetParentTransformToWidget(vtkMRMLMarkupsNode *vtkNotUsed(node), vtkAbstractWidget *vtkNotUsed(widget)){};

  /// Create a new widget for this markups node and save it to the helper.
  /// Returns widget on success, null on failure.
  vtkAbstractWidget *AddWidget(vtkMRMLMarkupsNode *markupsNode);

//  vtkMRMLMarkupsDisplayableManagerHelper *  GetHelper() { return this->Helper; };
  vtkGetObjectMacro(Helper, vtkMRMLMarkupsDisplayableManagerHelper);

protected:

  vtkMRMLMarkupsDisplayableManager3D();
  virtual ~vtkMRMLMarkupsDisplayableManager3D();

  virtual void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData);

//  virtual void Create();

  /// Wrap the superclass render request in a check for batch processing
  virtual void RequestRender();

  /// Remove MRML observers
  virtual void RemoveMRMLObservers();

  /// Called from RequestRender method if UpdateFromMRMLRequested is true
  /// \sa RequestRender() SetUpdateFromMRMLRequested()
  virtual void UpdateFromMRML();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// Called after the corresponding MRML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessMRMLSceneEvents
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneEndClose();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

  /// Called after the corresponding MRML View container was modified
  virtual void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller);

  /// Observe one node
  void SetAndObserveNode(vtkMRMLMarkupsNode *markupsNode);
  /// Observe all associated nodes.
  void SetAndObserveNodes();

  /// Observe the interaction node.
  void AddObserversToInteractionNode();
  void RemoveObserversFromInteractionNode();

  /// Preset functions for certain events.
  void OnMRMLMarkupsNodeModifiedEvent(vtkMRMLNode* node);
  void OnMRMLMarkupsNodeTransformModifiedEvent(vtkMRMLNode* node);
  void OnMRMLMarkupsNodeLockModifiedEvent(vtkMRMLNode* node);
  void OnMRMLMarkupsDisplayNodeModifiedEvent(vtkMRMLNode *node);
  void OnMRMLMarkupsPointModifiedEvent(vtkMRMLNode *node, int n);
  /// Subclasses need to react to new markups being added to or removed
  /// from a markups node or modified
  virtual void OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * vtkNotUsed(markupsNode)) {};
  virtual void OnMRMLMarkupsNodeMarkupRemovedEvent(vtkMRMLMarkupsNode * vtkNotUsed(markupsNode)) {};
  virtual void OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode* vtkNotUsed(node), int vtkNotUsed(n)) {};

  //
  // Handling of interaction within the RenderWindow
  //

  // Get the coordinates of a click in the RenderWindow
  void OnClickInRenderWindowGetCoordinates();
  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID = NULL);
  /// Counter for clicks in Render Window
  vtkMRMLMarkupsClickCounter* ClickCounter;

  /// Update a single seed from markup position, implemented by the subclasses, return
  /// true if the position changed
  virtual bool UpdateNthSeedPositionFromMRML(int vtkNotUsed(n),
                 vtkAbstractWidget *vtkNotUsed(widget),
                 vtkMRMLMarkupsNode *vtkNotUsed(markupsNode))
    { return false; }
  /// Update just the position for the widget, implemented by subclasses.
  virtual void UpdatePosition(vtkAbstractWidget *vtkNotUsed(widget), vtkMRMLNode *vtkNotUsed(node)) {}

  //
  // Seeds for widget placement
  //

  /// Place a seed for widgets
  virtual void PlaceSeed(double x, double y);
  /// Return the placed seeds
  vtkHandleWidget * GetSeed(int index);

  //
  // Coordinate Conversions
  //

  /// Convert display to world coordinates
  void GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates);
  void GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates);

  /// Convert display to viewport coordinates
  void GetDisplayToViewportCoordinates(double x, double y, double * viewportCoordinates);
  void GetDisplayToViewportCoordinates(double *displayCoordinates, double * viewportCoordinates);

  //
  // Widget functionality
  //

  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLMarkupsNode* node);
  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLMarkupsNode * node);
  /// Get the widget of a node.
  vtkAbstractWidget * GetWidget(vtkMRMLMarkupsNode * node);

  /// Check if it is the right displayManager
  virtual bool IsCorrectDisplayableManager();

  /// Return true if this displayable manager supports(can manage) that node,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(const char*), IsCorrectDisplayableManager()
  virtual bool IsManageable(vtkMRMLNode* node);
  /// Return true if this displayable manager supports(can manage) that node class,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(vtkMRMLNode*), IsCorrectDisplayableManager()
  virtual bool IsManageable(const char* nodeClassName);

  /// Focus of this displayableManager is set to a specific markups type when inherited
  const char* Focus;

  /// Disable processing when updating is in progress.
  int Updating;

  /// Respond to interactor style events
  virtual void OnInteractorStyleEvent(int eventid);

  /// Accessor for internal flag that disables interactor style event processing
  vtkGetMacro(DisableInteractorStyleEventsProcessing, int);

  vtkMRMLMarkupsDisplayableManagerHelper * Helper;

  double LastClickWorldCoordinates[4];

private:

  vtkMRMLMarkupsDisplayableManager3D(const vtkMRMLMarkupsDisplayableManager3D&); /// Not implemented
  void operator=(const vtkMRMLMarkupsDisplayableManager3D&); /// Not Implemented


  int DisableInteractorStyleEventsProcessing;

};

#endif
