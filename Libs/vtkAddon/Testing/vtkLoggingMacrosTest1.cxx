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

// MRMLDisplayableManager includes
#include <vtkLoggingMacros.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>

class vtkLoggingMacroTester : public vtkObject
{
public:
  static vtkLoggingMacroTester *New();
  vtkTypeMacro(vtkLoggingMacroTester,vtkObject);
  void TestLogs()
    {
    vtkInfoMacro("This is an info message printed from inside a VTK object");
    }
protected:
  vtkLoggingMacroTester() {};
  ~vtkLoggingMacroTester() {};
  vtkLoggingMacroTester(const vtkLoggingMacroTester&);
  void operator=(const vtkLoggingMacroTester&);
};

vtkStandardNewMacro(vtkLoggingMacroTester); 

//----------------------------------------------------------------------------
int vtkLoggingMacrosTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkLoggingMacroTester> testerClass;
  vtkInfoWithObjectMacro(testerClass, "This is an info message printed on behalf of a VTK object");
  vtkInfoWithoutObjectMacro("This is an info message printed without having a pointer to a related VTK object");
  
  return EXIT_SUCCESS;
}
