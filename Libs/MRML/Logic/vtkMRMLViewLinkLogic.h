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

///  vtkMRMLViewLinkLogic - slicer logic class for linked view manipulation
///
/// This class manages the logic associated with linking the controls
/// of multiple view and camera nodes. It listens to the
/// MRML scene for new view and camera nodes and observes
/// these nodes for ModifiedEvents. When notified of a ModifiedEvent
/// on a view or camera node, this logic class will
/// propagate state to other view and camera nodes. A
/// critical component of the design is that view and camera
/// nodes "know" when they are be changed interactively verses when
/// their state is being updated programmatically.

#ifndef __vtkMRMLViewLinkLogic_h
#define __vtkMRMLViewLinkLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// STD includes
#include <vector>

class vtkMRMLViewNode;
class vtkMRMLCameraNode;

class VTK_MRML_LOGIC_EXPORT vtkMRMLViewLinkLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual VTK class functions
  static vtkMRMLViewLinkLogic *New();
  vtkTypeMacro(vtkMRMLViewLinkLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkMRMLViewLinkLogic();
  ~vtkMRMLViewLinkLogic() override;

  // On a change in scene, we need to manage the observations.
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void OnMRMLNodeModified(vtkMRMLNode* node) override;

  /// Broadcast a view node to other view nodes.
  void BroadcastViewNodeEvent(vtkMRMLViewNode* viewNode);

  /// Broadcast a camera node to other camera nodes
  void BroadcastCameraNodeEvent(vtkMRMLCameraNode* cameraNode);

private:
  vtkMRMLViewLinkLogic(const vtkMRMLViewLinkLogic&) = delete;
  void operator=(const vtkMRMLViewLinkLogic&) = delete;

};

#endif
