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

#ifndef __vtkMRMLModuleLogic_h
#define __vtkMRMLModuleLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLLogicExport.h"

// VTK includes
#include <vtkSmartPointer.h>

// STD includes
#include <vector>

class vtkMRMLFileIOHandler;

/// \brief Superclass for logic classes of modules.
///
/// A module logic is the logic instance that is registered in the application logic
/// by vtkMRMLApplicationLogic::SetModuleLogic(). In addition to the features of vtkMRMLAbstractLogic,
/// a module logic can provide file readers and writers.
class VTK_MRML_LOGIC_EXPORT vtkMRMLModuleLogic : public vtkMRMLAbstractLogic
{
public:
  static vtkMRMLModuleLogic* New();
  vtkTypeMacro(vtkMRMLModuleLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create new instances of the file readers and writers of the module. The default implementation returns an empty list.
  /// Called by vtkMRMLApplicationLogic::SetModuleLogic(), which registers the returned readers and writers
  /// (it should not be called directly). Readers and writers must only keep a weak reference to the logic.
  /// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#the-reader
  virtual std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> CreateFileIOHandlers();

protected:
  vtkMRMLModuleLogic();
  ~vtkMRMLModuleLogic() override;

private:
  vtkMRMLModuleLogic(const vtkMRMLModuleLogic&) = delete;
  void operator=(const vtkMRMLModuleLogic&) = delete;
};

#endif
