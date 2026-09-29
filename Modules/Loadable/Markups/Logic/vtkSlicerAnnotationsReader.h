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

#ifndef __vtkSlicerAnnotationsReader_h
#define __vtkSlicerAnnotationsReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerMarkupsModuleLogicExport.h"

class vtkSlicerMarkupsLogic;

/// Reader of legacy annotation files (.acsv).
///
/// Properties: "fileName", "name", "annotationType" ("fiducial", "ruler", or "roi").
/// For backward compatibility, the annotation type can be specified by "fiducial", "ruler", "roi" boolean properties.
class VTK_SLICER_MARKUPS_MODULE_LOGIC_EXPORT vtkSlicerAnnotationsReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerAnnotationsReader* New();
  vtkTypeMacro(vtkSlicerAnnotationsReader, vtkMRMLFileReader);

  void SetMarkupsLogic(vtkSlicerMarkupsLogic* logic);
  vtkSlicerMarkupsLogic* GetMarkupsLogic();

  bool Load(vtkMRMLIOProperties* properties) override;

  /// Options: "name", "annotationType". Default annotation type is determined from the file name.
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override;

protected:
  vtkSlicerAnnotationsReader();
  ~vtkSlicerAnnotationsReader() override;

  vtkWeakPointer<vtkSlicerMarkupsLogic> MarkupsLogic;

private:
  vtkSlicerAnnotationsReader(const vtkSlicerAnnotationsReader&) = delete;
  void operator=(const vtkSlicerAnnotationsReader&) = delete;
};

#endif
