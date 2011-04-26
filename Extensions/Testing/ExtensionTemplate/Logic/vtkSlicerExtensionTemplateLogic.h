/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerExtensionTemplateLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerExtensionTemplateLogic_h
#define __vtkSlicerExtensionTemplateLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerExtensionTemplateModuleLogicExport.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_EXTENSIONTEMPLATE_MODULE_LOGIC_EXPORT vtkSlicerExtensionTemplateLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerExtensionTemplateLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Initialize listening to MRML events
  void InitializeEventListeners();

  /// MRML events
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();

protected:
  vtkSlicerExtensionTemplateLogic();
  virtual ~vtkSlicerExtensionTemplateLogic();

private:

  vtkSlicerExtensionTemplateLogic(const vtkSlicerExtensionTemplateLogic&); // Not implemented
  void operator=(const vtkSlicerExtensionTemplateLogic&);               // Not implemented
};

#endif
