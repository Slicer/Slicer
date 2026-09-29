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

#ifndef __vtkSlicerShaderPropertyReader_h
#define __vtkSlicerShaderPropertyReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerVolumeRenderingModuleLogicExport.h"

class vtkSlicerVolumeRenderingLogic;

/// Reader of GPU shader property files.
class VTK_SLICER_VOLUMERENDERING_MODULE_LOGIC_EXPORT vtkSlicerShaderPropertyReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerShaderPropertyReader* New();
  vtkTypeMacro(vtkSlicerShaderPropertyReader, vtkMRMLFileReader);

  void SetVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic);
  vtkSlicerVolumeRenderingLogic* GetVolumeRenderingLogic();

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerShaderPropertyReader();
  ~vtkSlicerShaderPropertyReader() override;

  vtkWeakPointer<vtkSlicerVolumeRenderingLogic> VolumeRenderingLogic;

private:
  vtkSlicerShaderPropertyReader(const vtkSlicerShaderPropertyReader&) = delete;
  void operator=(const vtkSlicerShaderPropertyReader&) = delete;
};

#endif
