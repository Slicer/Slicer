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
#include <vtkTestingOutputWindow.h>

class vtkLoggingMacroTester : public vtkObject
{
public:
  static vtkLoggingMacroTester *New();
  vtkTypeMacro(vtkLoggingMacroTester,vtkObject);
  void TestInfo()
    {
    vtkInfoMacro("This is an info message printed from inside a VTK object");
    }
  void TestWarning()
    {
    vtkWarningMacro("This is a warning message printed from inside a VTK object");
    }
  void TestError()
    {
    vtkErrorMacro("This is a error message printed from inside a VTK object");
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
  TESTING_OUTPUT_INIT();

  vtkNew<vtkLoggingMacroTester> testerClass;

  // Non-error messages

  TESTING_OUTPUT_RESET();
  vtkInfoWithObjectMacro(testerClass, "This is an info message printed on behalf of a VTK object");
  vtkInfoWithoutObjectMacro("This is an info message printed without having a pointer to a related VTK object");
  testerClass->TestInfo();
  TESTING_OUTPUT_ASSERT_MESSAGES(3);
  TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);

  // Warning messages
  
  TESTING_OUTPUT_RESET();
  testerClass->TestWarning();
  testerClass->TestWarning();
  TESTING_OUTPUT_ASSERT_WARNINGS(2);
  TESTING_OUTPUT_ASSERT_ERRORS(0);
  TESTING_OUTPUT_ASSERT_WARNINGS_MINIMUM(1);

  TESTING_OUTPUT_RESET();
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  testerClass->TestWarning();
  testerClass->TestWarning();
  TESTING_OUTPUT_ASSERT_WARNINGS_END();

  // Error messages

  TESTING_OUTPUT_RESET();
  testerClass->TestError();
  testerClass->TestError();
  testerClass->TestError();
  TESTING_OUTPUT_ASSERT_WARNINGS(0);
  TESTING_OUTPUT_ASSERT_ERRORS(3);
  TESTING_OUTPUT_ASSERT_ERRORS_MINIMUM(1);

  TESTING_OUTPUT_RESET();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  testerClass->TestError();
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  
  return EXIT_SUCCESS;
}
