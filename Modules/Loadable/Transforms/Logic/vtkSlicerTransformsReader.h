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

#ifndef __vtkSlicerTransformsReader_h
#define __vtkSlicerTransformsReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerTransformsModuleLogicExport.h"

class vtkSlicerTransformLogic;

/// Reader of transform files.
class VTK_SLICER_TRANSFORMS_MODULE_LOGIC_EXPORT vtkSlicerTransformsReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerTransformsReader* New();
  vtkTypeMacro(vtkSlicerTransformsReader, vtkMRMLFileReader);

  void SetTransformLogic(vtkSlicerTransformLogic* logic);
  vtkSlicerTransformLogic* GetTransformLogic();

  /// Higher confidence for NIFTI or NRRD files containing displacement field.
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerTransformsReader();
  ~vtkSlicerTransformsReader() override;

  vtkWeakPointer<vtkSlicerTransformLogic> TransformLogic;

private:
  vtkSlicerTransformsReader(const vtkSlicerTransformsReader&) = delete;
  void operator=(const vtkSlicerTransformsReader&) = delete;
};

#endif
