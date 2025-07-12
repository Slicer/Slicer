/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

#ifndef __vtkMRMLViewLogic_h
#define __vtkMRMLViewLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// STD includes
#include <vector>
#include <deque>

class vtkMRMLDisplayNode;
class vtkMRMLLinearTransformNode;
class vtkMRMLModelDisplayNode;
class vtkMRMLModelNode;
class vtkMRMLViewNode;
class vtkMRMLCameraNode;
class vtkMRMLVolumeNode;

class vtkAlgorithmOutput;
class vtkCollection;
class vtkImageBlend;
class vtkTransform;
class vtkImageData;
class vtkImageReslice;
class vtkTransform;

struct SliceLayerInfo;
struct BlendPipeline;

/// \brief Slicer logic class for view manipulation.
///
/// This class manages the logic associated with display of view windows
/// (but not the GUI).  Features of the class include:
///  -- manage the linking of the 3D View linking.
///
class VTK_MRML_LOGIC_EXPORT vtkMRMLViewLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual VTK class functions
  static vtkMRMLViewLogic* New();
  vtkTypeMacro(vtkMRMLViewLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set/Get layout name. This is used for finding the camera and view node in the scene.
  virtual void SetName(const char* name);
  virtual const char* GetName() const;

  /// The MRML View node for this View logic
  vtkGetObjectMacro(ViewNode, vtkMRMLViewNode);

  /// The MRML camera node for this View logic
  vtkGetObjectMacro(CameraNode, vtkMRMLCameraNode);

  /// Indicate an interaction with the camera node is beginning. The
  /// parameters of the camera node being manipulated are passed as a
  /// bitmask. See vtkMRMLViewNode::InteractionFlagType.
  void StartCameraNodeInteraction(unsigned int parameters);

  /// Indicate an interaction with the slice node has been completed
  void EndCameraNodeInteraction();

  /// Indicate an interaction with the view node is
  /// beginning. The parameters of the view node being manipulated
  /// are passed as a bitmask. See vtkMRMLViewNode::InteractionFlagType.
  void StartViewNodeInteraction(unsigned int parameters);

  /// Indicate an interaction with the view node has been completed
  void EndViewNodeInteraction();

  /// Convenience function for adding a view node and setting it in this logic
  vtkMRMLViewNode* AddViewNode(const char* layoutName);

  /// Convenient method to get the view node from scene and name of the Logic.
  /// The name of the Logic is the same of the widget one to which it is associated
  static vtkMRMLViewNode* GetViewNode(vtkMRMLScene* scene, const char* layoutName);

  /// Convenient method to get the camera node from scene and name of the Logic.
  /// The name of the Logic is the same of the widget one to which it is associated
  static vtkMRMLCameraNode* GetCameraNode(vtkMRMLScene* scene, const char* layoutName);

protected:
  vtkMRMLViewLogic();
  ~vtkMRMLViewLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  void SetViewNode(vtkMRMLViewNode* newViewNode);
  void SetCameraNode(vtkMRMLCameraNode* newCameraNode);

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void UpdateFromMRMLScene() override;

  void UpdateMRMLNodes();

  // View and camera nodes are looked up from the scene based on the layout name.
  std::string Name;

  vtkMRMLViewNode* ViewNode;
  vtkMRMLCameraNode* CameraNode;
  bool UpdatingMRMLNodes;

private:
  vtkMRMLViewLogic(const vtkMRMLViewLogic&) = delete;
  void operator=(const vtkMRMLViewLogic&) = delete;
};

#endif
