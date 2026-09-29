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

#ifndef __vtkSlicerSceneWriter_h
#define __vtkSlicerSceneWriter_h

// Slicer includes
#include "vtkMRMLFileWriter.h"

#include "vtkSlicerDataModuleLogicExport.h"

// VTK includes
#include <vtkWeakPointer.h>

class vtkSlicerApplicationLogic;

/// Writer of the MRML scene to a .mrml file, a Medical Reality Bundle (.mrb) file,
/// or a Slicer Data Bundle (directory).
///
/// Properties: "fileName", "screenShot" (vtkImageData, optional).
class VTK_SLICER_DATA_LOGIC_EXPORT vtkSlicerSceneWriter : public vtkMRMLFileWriter
{
public:
  static vtkSlicerSceneWriter* New();
  vtkTypeMacro(vtkSlicerSceneWriter, vtkMRMLFileWriter);

  /// Application logic is used for saving screenshots and data bundles.
  /// If not set then the application logic of the file IO manager where the writer is registered is used.
  void SetApplicationLogic(vtkSlicerApplicationLogic* applicationLogic);
  vtkSlicerApplicationLogic* GetApplicationLogic();

  /// Returns true if the object is a scene.
  bool CanWriteObject(vtkObject* object) override;

  bool Write(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerSceneWriter();
  ~vtkSlicerSceneWriter() override;

  bool WriteToMRML(vtkMRMLIOProperties* properties);
  bool WriteToMRB(vtkMRMLIOProperties* properties);
  bool WriteToDirectory(vtkMRMLIOProperties* properties);

  vtkWeakPointer<vtkSlicerApplicationLogic> ApplicationLogic;

private:
  vtkSlicerSceneWriter(const vtkSlicerSceneWriter&) = delete;
  void operator=(const vtkSlicerSceneWriter&) = delete;
};

#endif
