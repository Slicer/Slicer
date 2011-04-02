/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Daniel Haehn, UPenn
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLSceneViewsModuleLogic_h
#define __vtkMRMLSceneViewsModuleLogic_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

#include "qSlicerSceneViewsModuleExport.h"

#include "GUI/qSlicerSceneViewsModuleWidget.h"

// VTK includes
class vtkImageData;
class vtkMRMLHierarchyNode;

/// \ingroup Slicer_QtModules_SceneViews
class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT vtkSlicerSceneViewsModuleLogic :
  public vtkMRMLAbstractLogic
{
public:

  static vtkSlicerSceneViewsModuleLogic *New();
  vtkTypeRevisionMacro(vtkSlicerSceneViewsModuleLogic,vtkMRMLAbstractLogic);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Initialize listening to MRML events
  void InitializeEventListeners();
  
  // MRML events
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData );

  // After a node was added, propagate to widget
  void AddNodeCompleted(vtkMRMLHierarchyNode* hierarchyNode);

  // Register the widget
  void SetAndObserveWidget(qSlicerSceneViewsModuleWidget* widget);

  void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  void OnMRMLSceneViewNodeModifiedEvent(vtkMRMLNode* node);
  void OnMRMLSceneClosedEvent();


  
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();

  /// Create a sceneView..
  void CreateSceneView(const char* name, const char* description, int screenshotType, vtkImageData* screenshot);

  /// Modify an existing sceneView.
  void ModifySceneView(vtkStdString id, const char* name, const char* description, int screenshotType, vtkImageData* screenshot);

  /// Return the name of an existing sceneView.
  vtkStdString GetSceneViewName(const char* id);

  /// Return the description of an existing sceneView.
  vtkStdString GetSceneViewDescription(const char* id);

  /// Return the screenshotType of an existing sceneView.
  int GetSceneViewScreenshotType(const char* id);

  /// Return the screenshot of an existing sceneView.
  vtkImageData* GetSceneViewScreenshot(const char* id);

  /// Restore an sceneView.
  void RestoreSceneView(const char* id);

  /// Move sceneView up
  const char* MoveSceneViewUp(const char* id);

  /// Move sceneView up
  const char* MoveSceneViewDown(const char* id);

  //
  // Hierarchy functionality
  //
  /// Add a new visible hierarchy.
  /// The active hierarchy node will be the parent. If there is no
  /// active hierarchy node, use the top-level hierarchy node as the parent.
  /// If there is no top-level hierarchy node, create additionally a top-level hierarchy node which serves as
  /// a parent to the new hierarchy node. Return 1 on sucess 0 on failure.
  int AddHierarchy();

  /// Return the toplevel hierarchy node or create one if there is none:
  /// If an optional node is given, insert the new toplevel hierarchy before it. If not,
  /// just add the new toplevel hierarchy node.
  vtkMRMLHierarchyNode* GetTopLevelHierarchyNode(vtkMRMLNode* node=0);

  /// Set the active hierarchy node which will be used as a parent for new
  /// scene views
  void SetActiveHierarchyNode(vtkMRMLHierarchyNode* hierarchyNode);

  /// Set the active hierarchy node which will be used as a parent for new
  /// scene views
  void SetActiveHierarchyNodeByID(const char* id);

  /// return the id of the currently active hierarchy node, or null if none
  const char *GetActiveHierarchyNodeID();
  
protected:

  vtkSlicerSceneViewsModuleLogic();

  virtual ~vtkSlicerSceneViewsModuleLogic();

private:

  vtksys_stl::string m_StringHolder;

  qSlicerSceneViewsModuleWidget* m_Widget;

  vtkMRMLSceneViewNode* m_LastAddedSceneViewNode;

  
  vtkMRMLHierarchyNode* m_ActiveHierarchy;

  //
  // Private hierarchy functionality.
  //
  /// Add a new hierarchy node for a given node.
  /// If there is an optional node, insert the new hierarchy node before it else just add it.
  /// The active hierarchy node will be the parent. If there is no
  /// active hierarchy node, use the top-level hierarchy node as the parent.
  /// If there is no top-level hierarchy node, create additionally a top-level hierarchy node which serves as
  /// a parent to the new hierarchy node. Return 1 on success, 0 on failure.
  int AddHierarchyNodeForNode(vtkMRMLNode* annotationNode=0);
};

#endif
