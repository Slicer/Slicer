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
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>


using namespace vtkAddonTestingUtilities;

//----------------------------------------------------------------------------
int AreMatrixEqual_4x4_4x4_Test();
int AreMatrixEqual_4x4_3x3_Test();
int AreMatrixEqual_3x3_4x4_Test();
int AreMatrixEqual_3x3_3x3_Test();

//----------------------------------------------------------------------------
int vtkAddonMathUtilitiesTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  CHECK_INT(AreMatrixEqual_4x4_4x4_Test(), EXIT_SUCCESS);
  CHECK_INT(AreMatrixEqual_4x4_3x3_Test(), EXIT_SUCCESS);
  CHECK_INT(AreMatrixEqual_3x3_4x4_Test(), EXIT_SUCCESS);
  CHECK_INT(AreMatrixEqual_3x3_3x3_Test(), EXIT_SUCCESS);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
template< typename M1_TYPE, typename M2_TYPE>
int AreMatrixEqual_Test(int size_m1, int size_m2)
{
  vtkNew<M1_TYPE> m1;
  vtkNew<M2_TYPE> m2;
  vtkNew<M1_TYPE> m3;
  for (int i = 0; i < size_m1; i++)
    {
    for (int j = 0; j < size_m1; j++)
      {
      m1->SetElement(i, j, i);
      m3->SetElement(i, j, i * j);
      }
    }

  for (int i = 0; i < size_m2; i++)
    {
    for (int j = 0; j < size_m2; j++)
      {
      m2->SetElement(i, j, i);
      }
    }

  CHECK_BOOL( vtkAddonMathUtilities::MatrixAreEqual(m1.GetPointer(), m2.GetPointer()), true);
  CHECK_BOOL( vtkAddonMathUtilities::MatrixAreEqual(m1.GetPointer(), m3.GetPointer()), false);

  double tolerance = 1e-3;
  CHECK_BOOL( vtkAddonMathUtilities::MatrixAreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), true);

  m1->SetElement(0, 0 , 1e-4);
  CHECK_BOOL( vtkAddonMathUtilities::MatrixAreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), true);

  m1->SetElement(0, 0 , -1e-4);
  CHECK_BOOL( vtkAddonMathUtilities::MatrixAreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), true);

  m1->SetElement(0, 0 , 5e-3);
  CHECK_BOOL( vtkAddonMathUtilities::MatrixAreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), false);

  m1->SetElement(0, 0 , -5e-3);
  CHECK_BOOL( vtkAddonMathUtilities::MatrixAreEqual(m1.GetPointer(), m2.GetPointer(), tolerance), false);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int AreMatrixEqual_4x4_4x4_Test()
{
  return AreMatrixEqual_Test<vtkMatrix4x4, vtkMatrix4x4>(4, 4);
}

//----------------------------------------------------------------------------
int AreMatrixEqual_4x4_3x3_Test()
{
  return AreMatrixEqual_Test<vtkMatrix4x4, vtkMatrix3x3>(4, 3);
}

//----------------------------------------------------------------------------
int AreMatrixEqual_3x3_4x4_Test()
{
  return AreMatrixEqual_Test<vtkMatrix3x3, vtkMatrix4x4>(3, 4);
}

//----------------------------------------------------------------------------
int AreMatrixEqual_3x3_3x3_Test()
{
  return AreMatrixEqual_Test<vtkMatrix3x3, vtkMatrix3x3>(3, 3);
}
