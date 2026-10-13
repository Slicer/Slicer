/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkMRMLOperationProgress.h"

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLOperationProgress);

//----------------------------------------------------------------------------
vtkMRMLOperationProgress::vtkMRMLOperationProgress() = default;

//----------------------------------------------------------------------------
vtkMRMLOperationProgress::~vtkMRMLOperationProgress() = default;

//----------------------------------------------------------------------------
void vtkMRMLOperationProgress::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Progress: " << this->Progress << "\n";
  os << indent << "Title: " << this->Title << "\n";
  os << indent << "ProgressText: " << this->ProgressText << "\n";
  os << indent << "NumberOfFiles: " << this->NumberOfFiles << "\n";
  os << indent << "CurrentFileIndex: " << this->CurrentFileIndex << "\n";
  os << indent << "CurrentFileProgress: " << this->CurrentFileProgress << "\n";
  os << indent << "ProgressRange: " << this->ProgressRangeMinimum << " - " << this->ProgressRangeMaximum << "\n";
  os << indent << "CurrentFileRange: " << this->CurrentFileRangeMinimum << " - " << this->CurrentFileRangeMaximum << "\n";
  os << indent << "Abortable: " << (this->Abortable ? "true" : "false") << "\n";
  os << indent << "AbortExecute: " << (this->AbortExecute ? "true" : "false") << "\n";
}
