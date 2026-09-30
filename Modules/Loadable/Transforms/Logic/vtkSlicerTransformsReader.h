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

  /// Returns higher confidence than the default for NIFTI and NRRD files that contain a displacement field,
  /// and for .txt files that look like ITK transform files. Returns lower than default confidence for
  /// NIFTI and NRRD files that do not contain a displacement field and for other .txt files.
  double CanLoadFileConfidence(const std::string& filePath) override;

  /// Returns true if the file looks like an ITK text transform file (it only inspects the beginning of the file).
  static bool IsITKTextTransformFile(const std::string& filePath);

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
