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

// .NAME vtkSlicerGeneralizedReformatLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerGeneralizedReformatLogic_h
#define __vtkSlicerGeneralizedReformatLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerGeneralizedReformatModuleLogicExport.h"


class VTK_SLICER_GENERALIZEDREFORMAT_MODULE_LOGIC_EXPORT vtkSlicerGeneralizedReformatLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerGeneralizedReformatLogic *New();
  vtkTypeMacro(vtkSlicerGeneralizedReformatLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkSlicerGeneralizedReformatLogic();
  ~vtkSlicerGeneralizedReformatLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  vtkSlicerGeneralizedReformatLogic(const vtkSlicerGeneralizedReformatLogic&); // Not implemented
  void operator=(const vtkSlicerGeneralizedReformatLogic&); // Not implemented
};

#endif
