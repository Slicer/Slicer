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

#ifndef __vtkSlicerSegmentationsNodeWriter_h
#define __vtkSlicerSegmentationsNodeWriter_h

// Slicer includes
#include "vtkMRMLNodeWriter.h"

#include "vtkSlicerSegmentationsModuleLogicExport.h"

/// Writer of segmentation nodes.
///
/// Additional properties: "cropToMinimumExtent" (bool).
class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkSlicerSegmentationsNodeWriter : public vtkMRMLNodeWriter
{
public:
  static vtkSlicerSegmentationsNodeWriter* New();
  vtkTypeMacro(vtkSlicerSegmentationsNodeWriter, vtkMRMLNodeWriter);

  bool Write(vtkMRMLIOProperties* properties) override;

  /// Options of vtkMRMLNodeWriter and "cropToMinimumExtent".
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override;

protected:
  vtkSlicerSegmentationsNodeWriter();
  ~vtkSlicerSegmentationsNodeWriter() override;

private:
  vtkSlicerSegmentationsNodeWriter(const vtkSlicerSegmentationsNodeWriter&) = delete;
  void operator=(const vtkSlicerSegmentationsNodeWriter&) = delete;
};

#endif
