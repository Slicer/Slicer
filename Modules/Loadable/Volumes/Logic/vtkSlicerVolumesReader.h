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

#ifndef __vtkSlicerVolumesReader_h
#define __vtkSlicerVolumesReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerVolumesModuleLogicExport.h"

class vtkSlicerVolumesLogic;

/// Reader of volume files.
///
/// Properties: "fileName", "name", "labelmap", "center", "singleFile", "autoWindowLevel",
/// "discardOrientation", "show", "fileNames" (list of strings), "colorNodeID".
class VTK_SLICER_VOLUMES_MODULE_LOGIC_EXPORT vtkSlicerVolumesReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerVolumesReader* New();
  vtkTypeMacro(vtkSlicerVolumesReader, vtkMRMLFileReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetVolumesLogic(vtkSlicerVolumesLogic* logic);
  vtkSlicerVolumesLogic* GetVolumesLogic();

  /// Returns a positive number (>0) if the reader can load this file.
  /// In case the file uses a generic file extension (such as .nrrd) then the confidence value is adjusted based on
  /// the file content: if the file contains a dwmri nrrd file then confidence is increased to 0.7
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

  /// Options: "name", "labelmap", "singleFile", "center", "discardOrientation", "show", "colorNodeID".
  /// Default name, labelmap, and single file options are determined from the file names.
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override;

  /// Check each file to see if it's recognized as part of a series. If so,
  /// keep it as the archetype and remove all the others from the list.
  std::string ExamineFileList(std::vector<std::string>& fileList, vtkMRMLIOProperties* ioProperties) override;

protected:
  vtkSlicerVolumesReader();
  ~vtkSlicerVolumesReader() override;

  vtkWeakPointer<vtkSlicerVolumesLogic> VolumesLogic;

private:
  vtkSlicerVolumesReader(const vtkSlicerVolumesReader&) = delete;
  void operator=(const vtkSlicerVolumesReader&) = delete;
};

#endif
