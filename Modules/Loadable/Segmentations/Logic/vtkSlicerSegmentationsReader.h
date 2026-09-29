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

#ifndef __vtkSlicerSegmentationsReader_h
#define __vtkSlicerSegmentationsReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerSegmentationsModuleLogicExport.h"

class vtkSlicerSegmentationsModuleLogic;

/// Reader of segmentation files.
///
/// Properties: "fileName", "name", "autoOpacities" (bool), "colorNodeID".
class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkSlicerSegmentationsReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerSegmentationsReader* New();
  vtkTypeMacro(vtkSlicerSegmentationsReader, vtkMRMLFileReader);

  void SetSegmentationsLogic(vtkSlicerSegmentationsModuleLogic* logic);
  vtkSlicerSegmentationsModuleLogic* GetSegmentationsLogic();

  /// For generic file extensions (.nrrd) the file content is inspected.
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

  /// Options: "autoOpacities", "colorNodeID".
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override;

protected:
  vtkSlicerSegmentationsReader();
  ~vtkSlicerSegmentationsReader() override;

  vtkWeakPointer<vtkSlicerSegmentationsModuleLogic> SegmentationsLogic;

private:
  vtkSlicerSegmentationsReader(const vtkSlicerSegmentationsReader&) = delete;
  void operator=(const vtkSlicerSegmentationsReader&) = delete;
};

#endif
