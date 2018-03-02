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

// .NAME vtkSlicerSuperLoadableModuleTemplateLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerSuperLoadableModuleTemplateLogic_h
#define __vtkSlicerSuperLoadableModuleTemplateLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerSuperLoadableModuleTemplateModuleLogicExport.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_SUPERLOADABLEMODULETEMPLATE_MODULE_LOGIC_EXPORT vtkSlicerSuperLoadableModuleTemplateLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerSuperLoadableModuleTemplateLogic *New();
  vtkTypeMacro(vtkSlicerSuperLoadableModuleTemplateLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSlicerSuperLoadableModuleTemplateLogic();
  virtual ~vtkSlicerSuperLoadableModuleTemplateLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
private:

  vtkSlicerSuperLoadableModuleTemplateLogic(const vtkSlicerSuperLoadableModuleTemplateLogic&); // Not implemented
  void operator=(const vtkSlicerSuperLoadableModuleTemplateLogic&); // Not implemented
};

#endif
