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

==============================================================================*/

#ifndef __vtkSlicerTerminologiesReader_h
#define __vtkSlicerTerminologiesReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerTerminologiesModuleLogicExport.h"

class vtkSlicerTerminologiesModuleLogic;

/// Reader of terminology and anatomic context files.
class VTK_SLICER_TERMINOLOGIES_LOGIC_EXPORT vtkSlicerTerminologiesReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerTerminologiesReader* New();
  vtkTypeMacro(vtkSlicerTerminologiesReader, vtkMRMLFileReader);

  void SetTerminologiesLogic(vtkSlicerTerminologiesModuleLogic* logic);
  vtkSlicerTerminologiesModuleLogic* GetTerminologiesLogic();

  /// For generic file extensions (.json) the file content is inspected.
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerTerminologiesReader();
  ~vtkSlicerTerminologiesReader() override;

  vtkWeakPointer<vtkSlicerTerminologiesModuleLogic> TerminologiesLogic;

private:
  vtkSlicerTerminologiesReader(const vtkSlicerTerminologiesReader&) = delete;
  void operator=(const vtkSlicerTerminologiesReader&) = delete;
};

#endif
