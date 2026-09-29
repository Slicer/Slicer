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

#ifndef __vtkSlicerModelsReader_h
#define __vtkSlicerModelsReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerModelsModuleLogicExport.h"

class vtkSlicerModelsLogic;

/// Reader of model files.
///
/// Properties: "fileName", "name", "coordinateSystem" (vtkMRMLStorageNode::CoordinateSystemType).
class VTK_SLICER_MODELS_MODULE_LOGIC_EXPORT vtkSlicerModelsReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerModelsReader* New();
  vtkTypeMacro(vtkSlicerModelsReader, vtkMRMLFileReader);

  void SetModelsLogic(vtkSlicerModelsLogic* logic);
  vtkSlicerModelsLogic* GetModelsLogic();

  /// .vtk files can store either an image or a mesh. Confidence is 0.6 for meshes and 0.0 for images.
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

  /// Options: "coordinateSystem".
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override;

protected:
  vtkSlicerModelsReader();
  ~vtkSlicerModelsReader() override;

  vtkWeakPointer<vtkSlicerModelsLogic> ModelsLogic;

private:
  vtkSlicerModelsReader(const vtkSlicerModelsReader&) = delete;
  void operator=(const vtkSlicerModelsReader&) = delete;
};

#endif
