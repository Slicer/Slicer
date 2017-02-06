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

// std includes
#include <sstream>

// vtkAddon includes
#include "vtkAddonMathUtilities.h"
#include "vtkAddonTestingMacros.h"

// vtk includes
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTransform.h>


using namespace vtkAddonTestingUtilities;

//----------------------------------------------------------------------------
int AreMatrixEqual_4x4_4x4_Test();
int AreMatrixEqual_4x4_3x3_Test();
int AreMatrixEqual_3x3_4x4_Test();
int AreMatrixEqual_3x3_3x3_Test();
int GetOrientationMatrixTest();
int SetOrientationMatrixTest();
int NormalizeColumnsTest();
int NormalizeOrientationMatrixColumnsTest();
int ToString_Test();
int FromString_Test();

//----------------------------------------------------------------------------
int vtkAddonMathUtilitiesTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  CHECK_EXIT_SUCCESS(AreMatrixEqual_4x4_4x4_Test());
  CHECK_EXIT_SUCCESS(AreMatrixEqual_4x4_3x3_Test());
  CHECK_EXIT_SUCCESS(AreMatrixEqual_3x3_4x4_Test());
  CHECK_EXIT_SUCCESS(AreMatrixEqual_3x3_3x3_Test());
  CHECK_EXIT_SUCCESS(GetOrientationMatrixTest());
  CHECK_EXIT_SUCCESS(SetOrientationMatrixTest());
  CHECK_EXIT_SUCCESS(NormalizeColumnsTest());
  CHECK_EXIT_SUCCESS(NormalizeOrientationMatrixColumnsTest());
  CHECK_EXIT_SUCCESS(ToString_Test());
  CHECK_EXIT_SUCCESS(FromString_Test());
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

//----------------------------------------------------------------------------
int GetOrientationMatrixTest()
{
  vtkNew<vtkMatrix4x4> m44;
  for (int ii = 0; ii < 4; ii++)
    {
    for (int jj = 0; jj < 4; jj++)
      {
      m44->SetElement(ii, jj, (1 + ii)*(1 + jj));
      }
    }

  vtkNew<vtkMatrix3x3> m33;

  vtkAddonMathUtilities::GetOrientationMatrix(m44.GetPointer(), m33.GetPointer());

  for (int ii = 0; ii < 3; ii++)
    {
    for (int jj = 0; jj < 3; jj++)
      {
      CHECK_DOUBLE(m33->GetElement(ii, jj), (1 + ii)*(1 + jj));
      }
    }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int SetOrientationMatrixTest()
{
  // Fill m44 matrix
  vtkNew<vtkMatrix4x4> m44;
  for (int ii = 0; ii < 4; ii++)
    {
    for (int jj = 0; jj < 4; jj++)
      {
      m44->SetElement(ii, jj, (1 + ii)*(1 + jj));
      }
    }

  // Fill m33 matrix (with different values compared to m44)
  vtkNew<vtkMatrix3x3> m33;
  for (int ii = 0; ii < 3; ii++)
    {
    for (int jj = 0; jj < 3; jj++)
      {
      m33->SetElement(ii, jj, 10 * m44->GetElement(ii, jj));
      }
    }

  // Save original m44 matrix
  vtkNew<vtkMatrix4x4> m44Original;
  m44Original->DeepCopy(m44.GetPointer());

  // Update m44 matrix orientation part
  vtkAddonMathUtilities::SetOrientationMatrix(m33.GetPointer(), m44.GetPointer());

  // Check if correct parts are modified
  for (int ii = 0; ii < 4; ii++)
    {
    for (int jj = 0; jj < 4; jj++)
      {
      if (ii < 3 && jj < 3)
        {
        // orientation part, must match m33
        CHECK_DOUBLE(m44->GetElement(ii, jj), m33->GetElement(ii, jj));
        }
      else
        {
        // non-orientation part, must match original m44
        CHECK_DOUBLE(m44->GetElement(ii, jj), m44Original->GetElement(ii, jj));
        }
      }
    }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int NormalizeColumnsTest()
{
  vtkNew<vtkTransform> transform;
  transform->RotateX(20);
  transform->RotateY(30);
  transform->RotateZ(40);
  transform->Translate(5, 10, 15);
  double scale[3] = { 3.5, 0.7, 6.2 };
  transform->Scale(scale);

  vtkNew<vtkMatrix4x4> m44;
  transform->GetMatrix(m44.GetPointer());

  vtkNew<vtkMatrix3x3> m;
  vtkAddonMathUtilities::GetOrientationMatrix(m44.GetPointer(), m.GetPointer());

  vtkNew<vtkMatrix3x3> mNormalized;
  vtkAddonMathUtilities::GetOrientationMatrix(m44.GetPointer(), mNormalized.GetPointer());

  double computedScale[3] = { 0 };
  vtkAddonMathUtilities::NormalizeColumns(mNormalized.GetPointer(), computedScale);

  // Check normalization and scale computation
  for (int col = 0; col < 3; col++)
    {
    CHECK_DOUBLE(scale[col], computedScale[col]);
    double columnNorm = sqrt(mNormalized->GetElement(0, col)*mNormalized->GetElement(0, col) +
      mNormalized->GetElement(1, col)*mNormalized->GetElement(1, col) +
      mNormalized->GetElement(2, col)*mNormalized->GetElement(2, col));
    CHECK_DOUBLE_TOLERANCE(columnNorm, 1.0, 1e-6);
    CHECK_DOUBLE_TOLERANCE(mNormalized->GetElement(0, col) * scale[col], m->GetElement(0, col), 1e-6);
    }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int NormalizeOrientationMatrixColumnsTest()
{
  vtkNew<vtkTransform> transform;
  transform->RotateX(20);
  transform->RotateY(30);
  transform->RotateZ(40);
  transform->Translate(5, 10, 15);
  double scale[3] = { 3.5, 0.7, 6.2 };
  transform->Scale(scale);

  vtkNew<vtkMatrix4x4> m;
  transform->GetMatrix(m.GetPointer());

  vtkNew<vtkMatrix4x4> mNormalized;
  transform->GetMatrix(mNormalized.GetPointer());

  double computedScale[3] = { 0 };
  vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(mNormalized.GetPointer(), computedScale);

  // Check normalization and scale computation
  for (int col = 0; col < 3; col++)
  {
    CHECK_DOUBLE(scale[col], computedScale[col]);
    double columnNorm = sqrt(mNormalized->GetElement(0, col)*mNormalized->GetElement(0, col) +
      mNormalized->GetElement(1, col)*mNormalized->GetElement(1, col) +
      mNormalized->GetElement(2, col)*mNormalized->GetElement(2, col));
    CHECK_DOUBLE_TOLERANCE(columnNorm, 1.0, 1e-6);
    CHECK_DOUBLE_TOLERANCE(mNormalized->GetElement(0, col) * scale[col], m->GetElement(0, col), 1e-6);
  }

  // Check if other parts are not modified
  for (int ii = 0; ii < 4; ii++)
  {
    for (int jj = 0; jj < 4; jj++)
    {
      if (ii < 3 && jj < 3)
      {
        // orientation part, already checked
        continue;
      }
      else
      {
        // non-orientation part, must match original
        CHECK_DOUBLE(mNormalized->GetElement(ii, jj), m->GetElement(ii, jj));
      }
    }
  }

  return EXIT_SUCCESS;
}
//----------------------------------------------------------------------------
int ToString_Test()
{
  vtkNew<vtkMatrix4x4> mat;
  std::stringstream ss;
  std::string delimiter = ",";
  std::string rowDelimiter = "\n";
  for (int ii = 0; ii < 4; ii++)
    {
    for (int jj = 0; jj < 4; jj++)
      {
      double val = (1 + ii)*(1 + jj);
      mat->SetElement(ii, jj, val);
      ss << val << delimiter;
      }
    ss << rowDelimiter;
    }

  std::string resultStr = vtkAddonMathUtilities::ToString(mat.GetPointer(), delimiter, rowDelimiter);

  CHECK_INT(resultStr.compare(ss.str()), 0);

  return EXIT_SUCCESS;
}

bool MatrixEqual(std::string matrixStr, vtkMatrix4x4* matrixExpected, int numberOfFilledRowsCols = 4)
{
  vtkNew<vtkMatrix4x4> matrixFromStr;

  // If less than 16 values are passed then not all rows/columns are filled
  // (e.g., 2x2, 3x3 matrix). Pre-fill the non-filled region to make the
  // matrices equal.
  for (int r = 0; r < 4; r++)
    {
    for (int c = 0; c < 4; c++)
      {
      if (r < numberOfFilledRowsCols && c < numberOfFilledRowsCols)
        {
        continue;
        }
      matrixFromStr->SetElement(r, c, matrixExpected->GetElement(r,c));
      }
    }

  vtkAddonMathUtilities::FromString(matrixFromStr.GetPointer(), matrixStr);
  return vtkAddonMathUtilities::MatrixAreEqual(matrixFromStr.GetPointer(), matrixExpected);
}

//----------------------------------------------------------------------------
int FromString_Test()
{
  vtkNew<vtkMatrix4x4> mat;
  mat->SetElement(0, 0, 11);
  mat->SetElement(0, 1, 22);
  mat->SetElement(0, 2, 33);
  mat->SetElement(0, 3, 44);
  mat->SetElement(1, 0, 55);
  mat->SetElement(1, 1, 66);
  mat->SetElement(1, 2, 77);
  mat->SetElement(1, 3, 88);
  mat->SetElement(2, 0, 99);
  mat->SetElement(2, 1, 1010);
  mat->SetElement(2, 2, 1111);
  mat->SetElement(2, 3, 1212);
  mat->SetElement(3, 0, 1313);
  mat->SetElement(3, 1, 1414);
  mat->SetElement(3, 2, 1515);
  mat->SetElement(3, 3, 1616);

  // Generic list style
  CHECK_BOOL(MatrixEqual("11,22,33,44,55,66,77,88,99,1010,1111,1212,1313,1414,1515,1616", mat.GetPointer()), true);
  CHECK_BOOL(MatrixEqual("11 22 33 44 55 66 77 88 99 1010 1111 1212 1313 1414 1515 1616", mat.GetPointer()), true);
  CHECK_BOOL(MatrixEqual("11 22 33 55 66 77 99 1010 1111", mat.GetPointer(), 3), true);
  CHECK_BOOL(MatrixEqual("11 22 55 66", mat.GetPointer(), 2), true);
  CHECK_BOOL(MatrixEqual("11", mat.GetPointer(), 1), true);
  // Python numpy array style
  CHECK_BOOL(MatrixEqual("[[11, 22, 33, 44], [55, 66, 77, 88], [99, 1010, 1111, 1212], [1313, 1414, 1515, 1616]]",
    mat.GetPointer()), true);
  CHECK_BOOL(MatrixEqual("[[11, 22, 33, 44],\n       [55, 66, 77, 88],\n       [99, 1010, 1111, 1212],\n"
    "[1313, 1414, 1515, 1616]]", mat.GetPointer()), true);
  // Matlab-style
  CHECK_BOOL(MatrixEqual("[11 22 33 44; 55 66 77 88 99; 1010 1111 1212; 1313 1414 1515 1616]", mat.GetPointer()), true);
  CHECK_BOOL(MatrixEqual("[11, 22, 33, 44; 55, 66, 77, 88, 99; 1010, 1111, 1212; 1313, 1414, 1515, 1616]", mat.GetPointer()), true);

  return EXIT_SUCCESS;
}
