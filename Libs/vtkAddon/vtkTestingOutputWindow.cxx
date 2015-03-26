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

#include "vtkTestingOutputWindow.h"

#include "vtkObjectFactory.h"
#include "vtkNew.h"

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTestingOutputWindow);

//-------------------------------------------------------
vtkTestingOutputWindow* vtkTestingOutputWindow::GetInstance()
{
  vtkTestingOutputWindow* testingOutputWindow = vtkTestingOutputWindow::SafeDownCast(vtkOutputWindow::GetInstance());
  if (testingOutputWindow != NULL)
    {
    return testingOutputWindow;
    }
  vtkNew<vtkTestingOutputWindow> newTestingOutputWindow;
  vtkOutputWindow::SetInstance(newTestingOutputWindow.GetPointer());
  return newTestingOutputWindow.GetPointer();
}

//-------------------------------------------------------
vtkTestingOutputWindow::vtkTestingOutputWindow()
: NumberOfLoggedWarningMessages(0)
, NumberOfLoggedErrorMessages(0)
, NumberOfLoggedMessages(0)
{
}

//-------------------------------------------------------
vtkTestingOutputWindow::~vtkTestingOutputWindow()
{
}

//-------------------------------------------------------
void vtkTestingOutputWindow::DisplayText(const char* text)
{
  this->NumberOfLoggedMessages++;
  Superclass::DisplayText(text);
}

//-------------------------------------------------------
void vtkTestingOutputWindow::DisplayErrorText(const char* text)
{
  this->NumberOfLoggedMessages++;
  this->NumberOfLoggedErrorMessages++;
  Superclass::DisplayErrorText(text);
}

//-------------------------------------------------------
void vtkTestingOutputWindow::DisplayWarningText(const char* text)
{
  this->NumberOfLoggedMessages++;
  this->NumberOfLoggedWarningMessages++;
  Superclass::DisplayWarningText(text);
}

//-------------------------------------------------------
void vtkTestingOutputWindow::DisplayGenericWarningText(const char* text)
{
  this->NumberOfLoggedMessages++;
  this->NumberOfLoggedWarningMessages++;
  Superclass::DisplayGenericWarningText(text);
}

//-------------------------------------------------------
void vtkTestingOutputWindow::DisplayDebugText(const char* text)
{
  this->NumberOfLoggedMessages++;
  Superclass::DisplayDebugText(text);
} 

//-------------------------------------------------------
void vtkTestingOutputWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NumberOfLoggedWarningMessages: " << this->NumberOfLoggedWarningMessages << endl;
  os << indent << "NumberOfLoggesErrorMessages: " << this->NumberOfLoggedErrorMessages << endl;
} 

//-------------------------------------------------------
void vtkTestingOutputWindow::ResetNumberOfLoggedMessages()
{
  this->SetNumberOfLoggedWarningMessages(0);
  this->SetNumberOfLoggedErrorMessages(0);
  this->SetNumberOfLoggedMessages(0);
}

//-------------------------------------------------------
int vtkTestingOutputWindow::GetNumberOfLoggedWarningErrorMessages()
{
  return this->GetNumberOfLoggedWarningMessages() + this->GetNumberOfLoggedErrorMessages();
}
