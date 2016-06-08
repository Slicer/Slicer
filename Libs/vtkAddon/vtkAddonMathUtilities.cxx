/*=auto==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

===============================================================================auto=*/

#include <vtkAddonMathUtilities.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkAddonMathUtilities);

//----------------------------------------------------------------------------
vtkAddonMathUtilities::vtkAddonMathUtilities()
{
}

//----------------------------------------------------------------------------
vtkAddonMathUtilities::~vtkAddonMathUtilities()
{
}

//----------------------------------------------------------------------------
void vtkAddonMathUtilities::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);  
}

//----------------------------------------------------------------------------
bool vtkAddonMathUtilities::MatrixAreEqual(const vtkMatrix4x4* m1,
                                           const vtkMatrix4x4* m2,
                                           double tolerance)
{
  for (int i = 0; i < 4; i++)
    {
    for (int j = 0; j < 4; j++)
      {
      if ( fabs(m1->GetElement(i, j) - m2->GetElement(i, j)) >= tolerance )
        {
        return false;
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkAddonMathUtilities::MatrixAreEqual(const vtkMatrix4x4 *m1,
                                           const vtkMatrix3x3 *m2,
                                           double tolerance)
{
  for (int i = 0; i < 3; i++)
    {
    for (int j = 0; j < 3; j++)
      {
      if ( fabs(m1->GetElement(i, j) - m2->GetElement(i, j)) >= tolerance )
        {
        return false;
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkAddonMathUtilities::MatrixAreEqual(const vtkMatrix3x3 *m1,
                                           const vtkMatrix4x4 *m2,
                                           double tolerance)
{
  return MatrixAreEqual(m2, m1, tolerance);
}

//----------------------------------------------------------------------------
bool vtkAddonMathUtilities::MatrixAreEqual(const vtkMatrix3x3 *m1,
                                           const vtkMatrix3x3 *m2,
                                           double tolerance)
{
  for (int i = 0; i < 3; i++)
    {
    for (int j = 0; j < 3; j++)
      {
      if ( fabs(m1->GetElement(i, j) - m2->GetElement(i, j)) >= tolerance )
        {
        return false;
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkAddonMathUtilities::GetOrientationMatrix(vtkMatrix4x4* source,
                                                 vtkMatrix3x3* dest)
{
  if (!source || !dest)
    {
    return;
    }
  for (int ii = 0; ii < 3; ++ii)
    {
    for (int jj = 0; jj < 3; ++jj)
      {
      dest->SetElement(ii, jj, source->GetElement(ii, jj));
      }
    }
}
