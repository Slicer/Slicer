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

#include "vtkMRMLModuleLogic.h"
#include "vtkMRMLFileIOHandler.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLModuleLogic);

//----------------------------------------------------------------------------
vtkMRMLModuleLogic::vtkMRMLModuleLogic() = default;

//----------------------------------------------------------------------------
vtkMRMLModuleLogic::~vtkMRMLModuleLogic() = default;

//----------------------------------------------------------------------------
void vtkMRMLModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> vtkMRMLModuleLogic::CreateFileIOHandlers()
{
  // no readers or writers by default
  return {};
}
