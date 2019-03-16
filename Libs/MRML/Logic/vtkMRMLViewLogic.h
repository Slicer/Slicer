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
  static vtkMRMLViewLogic *New();
  vtkTypeMacro(vtkMRMLViewLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set / Get SliceLogic name
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  /// The MRML View node for this View logic
  vtkGetObjectMacro (ViewNode, vtkMRMLViewNode);
  void SetViewNode (vtkMRMLViewNode* newViewNode);

  /// The MRML camera node for this View logic
  vtkGetObjectMacro (CameraNode, vtkMRMLCameraNode);
  void SetCameraNode (vtkMRMLCameraNode* newCameraNode);

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

  /// Manage and synchronise the CameraNode
  void UpdateCameraNode();

  /// Manage and synchronise the ViewNode
  void UpdateViewNode();

  /// Convenient method to get the view node from scene and name of the Logic.
  /// The name of the Logic is the same of the widget one to which it is associated
  static vtkMRMLViewNode* GetViewNode(vtkMRMLScene* scene,
                                      const char* layoutName);

  /// Convenient method to get the camera node from scene and name of the Logic.
  /// The name of the Logic is the same of the widget one to which it is associated
  static vtkMRMLCameraNode* GetCameraNode(vtkMRMLScene* scene,
                                          const char* layoutName);

protected:
  vtkMRMLViewLogic();
  ~vtkMRMLViewLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void UpdateFromMRMLScene() override;

  void UpdateMRMLNodes();

  char* Name;
  vtkMRMLViewNode* ViewNode;
  vtkMRMLCameraNode* CameraNode;

private:
  vtkMRMLViewLogic(const vtkMRMLViewLogic&) = delete;
  void operator=(const vtkMRMLViewLogic&) = delete;

};

#endif
