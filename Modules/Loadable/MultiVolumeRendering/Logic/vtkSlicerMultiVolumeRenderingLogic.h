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

// .NAME vtkSlicerMultiVolumeRenderingLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerMultiVolumeRenderingLogic_h
#define __vtkSlicerMultiVolumeRenderingLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerMultiVolumeRenderingModuleLogicExport.h"

// MRML includes
class vtkMRMLAnnotationROINode;
class vtkMRMLLabelMapVolumeDisplayNode;
class vtkMRMLNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLViewNode;
class vtkMRMLVolumeDisplayNode;
class vtkMRMLVolumeNode;
class vtkMRMLVolumePropertyNode;

// VTK includes
class vtkColorTransferFunction;
class vtkPiecewiseFunction;
class vtkVolumeProperty;

class vtkMRMLMultiVolumeRenderingDisplayNode;

/// \ingroup Slicer_QtModules_MultiVolumeRendering
class VTK_SLICER_MULTIVOLUMERENDERING_MODULE_LOGIC_EXPORT vtkSlicerMultiVolumeRenderingLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerMultiVolumeRenderingLogic *New();
  vtkTypeMacro(vtkSlicerMultiVolumeRenderingLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

public:
  // Description:
  // Find first volume rendering display node matching bg, fg, and labelmap set
  vtkMRMLMultiVolumeRenderingDisplayNode* FindFirstMatchedDisplayNode(vtkMRMLVolumeNode *bg, vtkMRMLVolumeNode *fg, vtkMRMLVolumeNode *label);

  // Description:
  // Create a new display node
  vtkMRMLMultiVolumeRenderingDisplayNode* CreateDisplayNode();

  // Description:
  // Find volume rendering display node reference in the volume
  vtkMRMLMultiVolumeRenderingDisplayNode* GetDisplayNodeByID(vtkMRMLVolumeNode *volumeNode, char *displayNodeID);

protected:
  vtkSlicerMultiVolumeRenderingLogic();
  virtual ~vtkSlicerMultiVolumeRenderingLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
private:

  vtkSlicerMultiVolumeRenderingLogic(const vtkSlicerMultiVolumeRenderingLogic&); // Not implemented
  void operator=(const vtkSlicerMultiVolumeRenderingLogic&);               // Not implemented

  bool IsDisplayNodeMatch(vtkMRMLMultiVolumeRenderingDisplayNode *dnode, vtkMRMLVolumeNode *bg, vtkMRMLVolumeNode *fg, vtkMRMLVolumeNode *label);
};

#endif

