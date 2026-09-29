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

#ifndef __vtkSlicerSceneReader_h
#define __vtkSlicerSceneReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

#include "vtkSlicerDataModuleLogicExport.h"

/// Reader of MRML scene files.
///
/// Properties: "fileName", "clear" (bool, clear the scene before loading; if false then the scene is imported).
class VTK_SLICER_DATA_LOGIC_EXPORT vtkSlicerSceneReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerSceneReader* New();
  vtkTypeMacro(vtkSlicerSceneReader, vtkMRMLFileReader);

  bool Load(vtkMRMLIOProperties* properties) override;

  /// Options: "clear".
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override;

protected:
  vtkSlicerSceneReader();
  ~vtkSlicerSceneReader() override;

private:
  vtkSlicerSceneReader(const vtkSlicerSceneReader&) = delete;
  void operator=(const vtkSlicerSceneReader&) = delete;
};

#endif
