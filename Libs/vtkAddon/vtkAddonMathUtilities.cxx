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

#include <sstream>
#include <vector>
#include <cstdlib>
#include <cmath>

#include <vtkAddonMathUtilities.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtksys/RegularExpression.hxx>
#include <vtkLoggingMacros.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkAddonMathUtilities);

//----------------------------------------------------------------------------
vtkAddonMathUtilities::vtkAddonMathUtilities()
= default;

//----------------------------------------------------------------------------
vtkAddonMathUtilities::~vtkAddonMathUtilities()
= default;

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
  if (!m1 || !m2)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::MatrixAreEqual: invalid input matrix");
    return false;
    }
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
  if (!m1 || !m2)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::MatrixAreEqual: invalid input matrix");
    return false;
    }
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
  if (!m1 || !m2)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::MatrixAreEqual: invalid input matrix");
    return false;
    }
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
void vtkAddonMathUtilities::GetOrientationMatrixColumn(vtkMatrix4x4* m, int columnIndex,
                                      double columnVector[3])
{
  if (!m)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::GetMatrixColumn: invalid matrix");
    return;
    }
  for (int rowIndex = 0; rowIndex < 3; ++rowIndex)
    {
    columnVector[rowIndex] = m->GetElement(rowIndex, columnIndex);
    }
}

//----------------------------------------------------------------------------
void vtkAddonMathUtilities::SetOrientationMatrixColumn(vtkMatrix4x4* m, int columnIndex,
                                      double columnVector[3])
{
  if (!m)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::GetMatrixColumn: invalid matrix");
    return;
    }
  for (int rowIndex = 0; rowIndex < 3; ++rowIndex)
    {
    m->SetElement(rowIndex, columnIndex, columnVector[rowIndex]);
    }
}

//----------------------------------------------------------------------------
void vtkAddonMathUtilities::GetOrientationMatrix(vtkMatrix4x4* source,
                                                 vtkMatrix3x3* dest)
{
  if (!source || !dest)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::GetOrientationMatrix: invalid source or destination matrix");
    return;
    }
  for (int rowIndex = 0; rowIndex < 3; ++rowIndex)
    {
    for (int columnIndex = 0; columnIndex < 3; ++columnIndex)
      {
      dest->SetElement(rowIndex, columnIndex, source->GetElement(rowIndex, columnIndex));
      }
    }
}

//----------------------------------------------------------------------------
void vtkAddonMathUtilities::SetOrientationMatrix(vtkMatrix3x3* source,
                                                 vtkMatrix4x4* dest)
{
  if (!source || !dest)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::SetOrientationMatrix: invalid source or destination matrix");
    return;
    }
  for (int rowIndex = 0; rowIndex < 3; ++rowIndex)
    {
    for (int columnIndex = 0; columnIndex < 3; ++columnIndex)
      {
      dest->SetElement(rowIndex, columnIndex, source->GetElement(rowIndex, columnIndex));
      }
    }
}

//----------------------------------------------------------------------------
void vtkAddonMathUtilities::NormalizeColumns(vtkMatrix3x3 *m, double scale[3])
{
  if (!m)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::NormalizeColumns: invalid input matrix");
    return;
    }
  for (int col = 0; col < 3; col++)
    {
    double len = 0;
    for (int row = 0; row < 3; row++)
      {
      len += m->GetElement(row, col) * m->GetElement(row, col);
      }
    len = sqrt(len);
    scale[col] = len;
    for (int row = 0; row < 3; row++)
      {
      m->SetElement(row, col,  m->GetElement(row, col)/len);
      }
    }
}

//----------------------------------------------------------------------------
void vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(vtkMatrix4x4 *m, double scale[3])
{
  if (!m)
    {
    vtkGenericWarningMacro("vtkAddonMathUtilities::NormalizeOrientationMatrixColumns: invalid input matrix");
    return;
    }
  for (int col = 0; col < 3; col++)
    {
    double len = 0;
    for (int row = 0; row < 3; row++)
      {
      len += m->GetElement(row, col) * m->GetElement(row, col);
      }
    len = sqrt(len);
    scale[col] = len;
    for (int row = 0; row < 3; row++)
      {
      m->SetElement(row, col,  m->GetElement(row, col)/len);
      }
    }
}

//----------------------------------------------------------------------------
std::string vtkAddonMathUtilities::ToString(const vtkMatrix4x4* mat, const std::string delimiter, const std::string rowDelimiter)
{
  if (!mat)
    {
    return "";
    }

  std::stringstream ss;
  for (int i = 0; i < 4; i++)
    {
    for (int j = 0; j < 4; j++)
      {
      ss << mat->GetElement(i, j);
      ss << delimiter;
      }
    ss << rowDelimiter;
    }

  return ss.str();
}

//----------------------------------------------------------------------------
bool vtkAddonMathUtilities::FromString(vtkMatrix4x4* mat, const std::string& str, const std::string delimiterExp)
{
  if (!mat)
    {
    return false;
    }

  // Parse the string using the regular expression
  vtksys::RegularExpression delimiterRegex( delimiterExp );

  // Convert each string token into a double and put into vector
  char* end;
  std::string remainString = str;
  std::vector<double> elements;
  while(!remainString.empty())
    {
    bool separatorFound = delimiterRegex.find(remainString);
    std::string::size_type tokenStartIndex = remainString.length();
    std::string::size_type tokenEndIndex = remainString.length();
    if (separatorFound)
      {
      tokenStartIndex = delimiterRegex.start(0);
      tokenEndIndex = delimiterRegex.end(0);
      }

    std::string valString = remainString.substr(0, tokenStartIndex);
    remainString = remainString.substr(tokenEndIndex);
    if (valString.empty()) // Handle back-to-back delimiters
      {
      continue;
      }

    // strtod is much faster (about 2x on some computers) than string stream
    // based string->number conversion
    double val = std::strtod(valString.c_str(), &end);
    if (*end != 0) // Parsing failed due to non-numeric character
      {
      return false;
      }

    elements.push_back(val);
    }

  // Ensure the matrix is 1x1, 2x2, 3x3, or 4x4
  if (elements.size() != 1
    && elements.size() != 4
    && elements.size() != 9
    && elements.size() != 16)
    {
    return false;
    }
  int dimension = std::sqrt(elements.size()) + 0.5; // Since conversion to int just truncates

  // Put into matrix
  int linearIndex = 0;
  for (int row = 0; row < dimension; row++)
    {
    for (int col = 0; col < dimension; col++)
      {
      mat->SetElement(row, col, elements.at(linearIndex));
      linearIndex++;
      }
    }

  return true;
}
