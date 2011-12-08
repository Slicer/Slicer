/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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

#include "vtkSlicerSceneViewsModuleLogicExport.h"
//#include "qSlicerSceneViewsModuleExport.h"

#include "vtkSlicerModuleLogic.h"


// VTK includes
class vtkImageData;
class vtkMRMLHierarchyNode;

#include <string>

/// \ingroup Slicer_QtModules_SceneViews
class VTK_SLICER_SCENEVIEWS_MODULE_LOGIC_EXPORT vtkSlicerSceneViewsModuleLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerSceneViewsModuleLogic *New();
  vtkTypeRevisionMacro(vtkSlicerSceneViewsModuleLogic,vtkSlicerModuleLogic);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  /// Initialize listening to MRML events
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);

  /// After a node was added, save it as the last added one
  void AddNodeCompleted(vtkMRMLHierarchyNode* hierarchyNode);

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

  /// Remove a scene view node
  void RemoveSceneViewNode(vtkMRMLSceneViewNode *sceneViewNode);

  //
  // Hierarchy functionality
  //
  /// Add a new visible hierarchy.
  /// The active hierarchy node will be the parent. If there is no
  /// active hierarchy node, use the top-level hierarchy node as the parent.
  /// If there is no top-level hierarchy node, create additionally a top-level hierarchy node which serves as
  /// a parent to the new hierarchy node. Return 1 on sucess 0 on failure.
  int AddHierarchy();

  /// Return the toplevel hierarchy node ID or create one and add it to the
  /// scene if there is none:
  /// If an optional node is given, insert the new toplevel hierarchy before it. If not,
  /// just add the new toplevel hierarchy node.
  char * GetTopLevelHierarchyNodeID(vtkMRMLNode* node=0);

  /// Get the active hierarchy node which will be used as a parent for new
  /// scene views
  vtkMRMLHierarchyNode *GetActiveHierarchyNode();

  /// get/set the id of the currently active hierarchy node
  vtkGetStringMacro(ActiveHierarchyNodeID);
  vtkSetStringMacro(ActiveHierarchyNodeID);

  /// Scan through the current mrml scene and make sure all scene view nodes
  /// have hierarchy nodes, adding them if missing
  void AddMissingHierarchyNodes();

protected:

  vtkSlicerSceneViewsModuleLogic();

  virtual ~vtkSlicerSceneViewsModuleLogic();

  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneEndImport();
  virtual void OnMRMLSceneEndRestore();
  virtual void OnMRMLSceneEndClose();

  virtual void OnMRMLNodeModified(vtkMRMLNode* node);

private:

  std::string m_StringHolder;


  vtkMRMLSceneViewNode* m_LastAddedSceneViewNode;

  
  char *ActiveHierarchyNodeID;

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
