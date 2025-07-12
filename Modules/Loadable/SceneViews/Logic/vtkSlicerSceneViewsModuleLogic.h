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

#ifndef vtkSlicerSceneViewsModuleLogic_h
#define vtkSlicerSceneViewsModuleLogic_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

#include "vtkSlicerSceneViewsModuleLogicExport.h"
// #include "qSlicerSceneViewsModuleExport.h"

#include "vtkSlicerModuleLogic.h"

// MRML includes
class vtkMRMLSequenceBrowserNode;
class vtkMRMLSequenceNode;
class vtkMRMLSceneViewNode;
class vtkMRMLTextNode;
class vtkMRMLVolumeNode;

// VTK includes
class vtkImageData;

// STD includes
#include <string>

class VTK_SLICER_SCENEVIEWS_MODULE_LOGIC_EXPORT vtkSlicerSceneViewsModuleLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerSceneViewsModuleLogic* New();
  vtkTypeMacro(vtkSlicerSceneViewsModuleLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Initialize listening to MRML events
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;

  /// Create a sceneView..
  void CreateSceneView(std::string name,
                       std::string description = "",
                       int screenshotType = ScreenShotType3D,
                       vtkImageData* screenshot = nullptr,
                       bool saveDisplayNodes = true,
                       bool saveViewNodes = true,
                       vtkMRMLSequenceBrowserNode* sequenceBrowser = nullptr);
  void CreateSceneView(vtkCollection* savedNodes,
                       std::string name,
                       std::string description = "",
                       int screenshotType = ScreenShotType3D,
                       vtkImageData* screenshot = nullptr,
                       vtkMRMLSequenceBrowserNode* sequenceBrowser = nullptr);
  void CreateSceneView(std::vector<vtkMRMLNode*> savedNodes,
                       std::string name,
                       std::string description = "",
                       int screenshotType = ScreenShotType3D,
                       vtkImageData* screenshot = nullptr,
                       vtkMRMLSequenceBrowserNode* sequenceBrowser = nullptr);

  /// Returns the index for a scene view with the given name. If no matching scene view is found, returns -1.
  int GetSceneViewIndexByName(std::string name);

  /// Modify the metadata of an existing sceneView.
  void ModifyNthSceneView(int sceneViewIndex, std::string name, std::string, int screenshotType, vtkImageData* screenshot);

  //@{
  /// Update the contents of a sceneView to match the current state of the scene.
  /// If new nodes are specified, they will be added to the scene view.
  void UpdateNthSceneView(int sceneViewIndex, bool updateExistingNodes = true, bool saveDisplayNodes = true, bool saveViewNodes = true);
  void UpdateNthSceneView(vtkCollection* savedNodes, int sceneViewIndex, bool updateExistingNodes = true);
  void UpdateNthSceneView(std::vector<vtkMRMLNode*> savedNodes, int sceneViewIndex, bool updateExistingNodes = true);
  //@}

  /// Convert the index of the scene view to the corresponding value index of the sequence browser that holds the snapshot.
  int SceneViewIndexToSequenceBrowserIndex(int sceneViewIndex);

  //@{
  /// Set/Get the name of an existing sceneView.
  void SetNthSceneViewName(int index, std::string name);
  std::string GetNthSceneViewName(int index);
  //@}

  /// Get the number of sceneViews.
  int GetNumberOfSceneViews();

  //@{
  /// Set/Get the description of an existing sceneView.
  void SetNthSceneViewDescription(int index, std::string description);
  std::string GetNthSceneViewDescription(int index);
  //@}

  //@{
  /// Set/Get the screenshot type of an existing sceneView.
  void SetNthSceneViewScreenshotType(int index, int type);
  int GetNthSceneViewScreenshotType(int index);
  //@}

  //@{
  /// Set/Get the screenshot of an existing sceneView.
  void SetNthSceneViewScreenshot(int index, vtkImageData* screenshot);
  vtkImageData* GetNthSceneViewScreenshot(int index);
  //@}

  //@{
  /// Restore a sceneView.
  bool RestoreSceneView(int sceneViewIndex);
  bool RestoreSceneView(std::string sceneViewName);
  //@}

  /// Move sceneView up
  std::string MoveSceneViewUp(std::string id);

  /// Move sceneView up
  std::string MoveSceneViewDown(std::string id);

  /// Remove a sceneView.
  bool RemoveSceneView(int index);

  //@{
  // Attributes and values for marking sequence browsers as scene views.
  static const char* GetSceneViewNodeAttributeName();
  static const char* GetSceneViewNodeAttributeValue();
  //@}

  //@{
  // Attributes on the screenshot node which contain scene view names, descriptions and screenshot types.
  static const char* GetSceneViewNameAttributeName();
  static const char* GetSceneViewDescriptionAttributeName();
  static const char* GetSceneViewScreenshotTypeAttributeName();
  //@}

  // Reference role for screenshot volume node.
  static const char* GetSceneViewScreenshotReferenceRole();

  // Returns the sequence browser used to record the scene views. If not found and addMissingNode is true,
  // then it will be created.
  vtkMRMLSequenceBrowserNode* GetSceneViewSequenceBrowserNode(bool addMissingNode);

  // Returns the sequence browser for the Nth scene view.
  vtkMRMLSequenceBrowserNode* GetNthSceneViewSequenceBrowserNode(int index);

  // Initialize a new scene view sequence browser node.
  vtkMRMLSequenceBrowserNode* AddNewSceneViewSequenceBrowserNode();

  /// The screenshot type of a sceneView
  enum
  {
    ScreenShotType3D = 0,
    ScreenShotTypeRed = 1,
    ScreenShotTypeYellow = 2,
    ScreenShotTypeGreen = 3,
    ScreenShotTypeFullLayout = 4
  };

  enum
  {
    SceneViewsModifiedEvent = 22001,
  };

  //@{
  /// Convert string to/from scene view enum type.
  std::string GetScreenShotTypeAsString(int type);
  int GetScreenShotTypeFromString(const std::string& type);
  //@}

  //@{
  /// Returns true if the node is a scene view node.
  bool IsSceneViewNode(vtkMRMLNode* node);
  //@}

protected:
  vtkSlicerSceneViewsModuleLogic();

  ~vtkSlicerSceneViewsModuleLogic() override;

  /// Called when MRMLScene events are invoked
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneEndRestore() override;
  void OnMRMLSceneEndClose() override;

  /// Called when a scene view sequence browser node is modified
  void OnMRMLNodeModified(vtkMRMLNode* node) override;

  /// Returns the proxy node containing the scene view screenshot.
  vtkMRMLVolumeNode* GetSceneViewScreenshotProxyNode(vtkMRMLSequenceBrowserNode* sequenceBrowser = nullptr);

  /// Return the data node for the specified proxy node in the Nth scene view index.
  vtkMRMLNode* GetNthSceneViewDataNode(int index, vtkMRMLNode* proxyNode);

  /// Return the screenshot data node in the Nth scene view index.
  vtkMRMLVolumeNode* GetNthSceneViewScreenshotDataNode(int index);

  /// Returns the proxy node in the scene for the Nth scene view.
  vtkMRMLVolumeNode* GetNthSceneViewScreenshotProxyNode(int index);

  //@{
  // Set/Get an attribute on the data node for the Nth scene view.
  void SetNthNodeAttribute(vtkMRMLNode* proxyTextNode, int index, std::string attributeName, std::string text);
  std::string GetNthNodeAttribute(vtkMRMLNode* proxyTextNode, int index, std::string attributeName);
  //@}

protected:
  /// Convert all existing vtkMRMLSceneViewNode in the scene to use Sequences.
  void ConvertSceneViewNodesToSequenceBrowserNodes(vtkMRMLScene* scene);

  /// Convert the specified vtkMRMLSceneViewNode to use sequences.
  vtkMRMLSequenceBrowserNode* ConvertSceneViewNodeToSequenceBrowserNode(vtkMRMLSceneViewNode* sceneView, vtkMRMLSequenceBrowserNode* sequenceBrowserNode);

  /// Add all display-related nodes to the vector.
  void GetDisplayNodes(std::vector<vtkMRMLNode*>& displayNodes);

  /// Add all view-related nodes to the vector.
  void GetViewNodes(std::vector<vtkMRMLNode*>& viewNodes);

  /// Returns the sequence node for a given proxy node. Will create a new vtkMRMLSequenceNode if none exists.
  vtkMRMLSequenceNode* GetOrAddSceneViewSequenceNode(vtkMRMLSequenceBrowserNode* sequenceBrowser, vtkMRMLNode* proxyNode);

private:
  std::string m_StringHolder;

private:
  vtkSlicerSceneViewsModuleLogic(const vtkSlicerSceneViewsModuleLogic&) = delete;
  void operator=(const vtkSlicerSceneViewsModuleLogic&) = delete;
};

#endif
