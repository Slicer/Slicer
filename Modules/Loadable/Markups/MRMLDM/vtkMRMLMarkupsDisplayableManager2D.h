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

#ifndef __vtkMRMLMarkupsDisplayableManager2D_h
#define __vtkMRMLMarkupsDisplayableManager2D_h

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsClickCounter.h"
#include "vtkMRMLMarkupsDisplayableManagerHelper.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractSliceViewDisplayableManager.h>

// VTK includes
#include <vtkHandleWidget.h>
#include <vtkSeedWidget.h>

class vtkMRMLMarkupsNode;
class vtkSlicerViewerWidget;
class vtkMRMLMarkupsDisplayNode;
class vtkAbstractWidget;

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsDisplayableManager2D :
    public vtkMRMLAbstractSliceViewDisplayableManager
{
public:

  static vtkMRMLMarkupsDisplayableManager2D *New();
  vtkTypeRevisionMacro(vtkMRMLMarkupsDisplayableManager2D, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Hide/Show a widget so that the node's display node visibility setting
  /// matches that of the widget
  void UpdateWidgetVisibility(vtkMRMLMarkupsNode* node);

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLMarkupsNode* node);
  /// Get the sliceNode, if registered.
  vtkMRMLSliceNode * GetSliceNode();

  /// Check if the displayCoordinates are inside the viewport and if not,
  /// correct the displayCoordinates. Coordinates are reset if the normalized
  /// viewport coordinates are less than 0.001 or greater than 0.999 and are
  /// reset to those values.
  /// If the coordinates have been reset, return true, otherwise return false.
  bool RestrictDisplayCoordinatesToViewport(double* displayCoordinates);

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

  /// Set/Get the 2d scale factor to divide 3D scale by to show 2D elements appropriately (usually set to 300)
  vtkSetMacro(ScaleFactor2D, double);
  vtkGetMacro(ScaleFactor2D, double);

  /// Create a new widget for this markups node and save it to the helper.
  /// Returns widget on success, null on failure.
  vtkAbstractWidget *AddWidget(vtkMRMLMarkupsNode *markupsNode);

  vtkMRMLMarkupsDisplayableManagerHelper *  GetHelper() { return this->Helper; };

  /// Checks if this 2D displayable manager is in light box mode. Returns true
  /// if there is a slice node and it has grid columns or rows greater than 1,
  /// and false otherwise.
  bool IsInLightboxMode();

  /// Gets the world coordinate of the markups node point, transforms it to
  /// display coordinates, takes the z element to calculate the light box index.
  /// Returns -1 if not in lightbox mode or the indices are out of range.
  int GetLightboxIndex(vtkMRMLMarkupsNode *node, int markupIndex, int pointIndex);

  /// Update a single seed from markup position, implemented by the subclasses, return
  /// true if the position changed
  virtual bool UpdateNthSeedPositionFromMRML(int vtkNotUsed(n),
                 vtkAbstractWidget *vtkNotUsed(widget),
                 vtkMRMLMarkupsNode *vtkNotUsed(markupsNode))
    { return false; }

  /// Update a single markup position from the seed widget, implemented by the subclasses,
  /// return true if the position changed
  virtual bool UpdateNthMarkupPositionFromWidget(int vtkNotUsed(n),
                 vtkMRMLMarkupsNode *vtkNotUsed(pointsNode),
                 vtkAbstractWidget *vtkNotUsed(widget))
    { return false; }

protected:

  vtkMRMLMarkupsDisplayableManager2D();
  virtual ~vtkMRMLMarkupsDisplayableManager2D();

  virtual void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData);

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

  /// Handler for specific SliceView actions, iterate over the widgets in the helper
  virtual void OnMRMLSliceNodeModifiedEvent();

  /// Check, if the widget is displayable in the current slice geometry for
  /// this markup, returns true if a 3d displayable manager
  virtual bool IsWidgetDisplayableOnSlice(vtkMRMLMarkupsNode* node, int markupIndex = 0);

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
  /// Subclasses need to react to new markups being added to a markups node or modified
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

  /// Update just the position for the widget, implemented by subclasses.
  virtual void UpdatePosition(vtkAbstractWidget *vtkNotUsed(widget), vtkMRMLNode *vtkNotUsed(node)) {};

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

  vtkMRMLMarkupsDisplayableManager2D(const vtkMRMLMarkupsDisplayableManager2D&); /// Not implemented
  void operator=(const vtkMRMLMarkupsDisplayableManager2D&); /// Not Implemented


  int DisableInteractorStyleEventsProcessing;

  vtkMRMLSliceNode * SliceNode;

  /// Scale factor for 2d windows
  double ScaleFactor2D;
};

#endif
