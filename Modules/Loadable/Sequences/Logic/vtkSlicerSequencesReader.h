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

#ifndef __vtkSlicerSequencesReader_h
#define __vtkSlicerSequencesReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerSequencesModuleLogicExport.h"

// VTK includes
#include <vtkCommand.h>

class vtkSlicerSequencesLogic;

/// Reader of sequence files.
///
/// Properties: "fileName", "name", "show" (create a sequence browser and show the proxy node), "colorNodeID".
///
/// If a sequence browser node is created then ShowSequenceBrowserRequestedEvent is invoked, with the
/// sequence browser node as call data, so that the application can show the sequence browser toolbar.
class VTK_SLICER_SEQUENCES_MODULE_LOGIC_EXPORT vtkSlicerSequencesReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerSequencesReader* New();
  vtkTypeMacro(vtkSlicerSequencesReader, vtkMRMLFileReader);

  enum
  {
    ShowSequenceBrowserRequestedEvent = vtkCommand::UserEvent + 182
  };

  void SetSequencesLogic(vtkSlicerSequencesLogic* logic);
  vtkSlicerSequencesLogic* GetSequencesLogic();

  /// For generic file extensions the file content is inspected to determine if it is a sequence.
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerSequencesReader();
  ~vtkSlicerSequencesReader() override;

  vtkWeakPointer<vtkSlicerSequencesLogic> SequencesLogic;

private:
  vtkSlicerSequencesReader(const vtkSlicerSequencesReader&) = delete;
  void operator=(const vtkSlicerSequencesReader&) = delete;
};

#endif
