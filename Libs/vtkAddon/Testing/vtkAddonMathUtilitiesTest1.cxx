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

// vtkAddon includes
#include "vtkAddonMathUtilities.h"
#include "vtkAddonTestingMacros.h"

// vtk includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>


using namespace vtkAddonTestingUtilities;

//----------------------------------------------------------------------------
int vtkAddonMathUtilitiesTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{

  vtkNew<vtkMatrix4x4> m1;
  vtkNew<vtkMatrix4x4> m2;
  vtkNew<vtkMatrix4x4> m3;
  for (int i = 0; i < 4; i++)
    {
    for (int j = 0; j < 4; j++)
      {  
      m1->SetElement(i, j, i);
      m3->SetElement(i, j, i * j);
      }
    }

  m2->DeepCopy(m1.GetPointer());

  CHECK_BOOL( vtkAddonMathUtilities::Matrix4x4AreEqual(m1.GetPointer(), m2.GetPointer()), true);
  CHECK_BOOL( vtkAddonMathUtilities::Matrix4x4AreEqual(m1.GetPointer(), m3.GetPointer()), false);

  double tolerance = 1e-3;
  CHECK_BOOL( vtkAddonMathUtilities::Matrix4x4AreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), true);

  m1->SetElement(0, 0 , 1e-4);
  CHECK_BOOL( vtkAddonMathUtilities::Matrix4x4AreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), true);

  m1->SetElement(0, 0 , -1e-4);
  CHECK_BOOL( vtkAddonMathUtilities::Matrix4x4AreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), true);

  m1->SetElement(0, 0 , 5e-3);
  CHECK_BOOL( vtkAddonMathUtilities::Matrix4x4AreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), false);

  m1->SetElement(0, 0 , -5e-3);
  CHECK_BOOL( vtkAddonMathUtilities::Matrix4x4AreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), false);


  return EXIT_SUCCESS;
}
