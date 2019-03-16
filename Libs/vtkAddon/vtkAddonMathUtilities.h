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

/// \brief vtkAddonMathUtilities.
///
///

#ifndef __vtkAddonMathUtilities_h
#define __vtkAddonMathUtilities_h

#include <vtkAddon.h>
#include <vtkObject.h>

class vtkMatrix4x4;
class vtkMatrix3x3;

class VTK_ADDON_EXPORT vtkAddonMathUtilities : public vtkObject
{
public:
  static vtkAddonMathUtilities *New();
  vtkTypeMacro(vtkAddonMathUtilities,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static bool MatrixAreEqual(const vtkMatrix4x4* m1,
                             const vtkMatrix4x4* m2,
                             double tolerance = 1e-3);

  static bool MatrixAreEqual(const vtkMatrix4x4 *m1,
                             const vtkMatrix3x3 *m2,
                             double tolerance = 1e-3);

  static bool MatrixAreEqual(const vtkMatrix3x3 *m1,
                             const vtkMatrix4x4 *m2,
                             double tolerance = 1e-3);

  static bool MatrixAreEqual(const vtkMatrix3x3 *m1,
                             const vtkMatrix3x3 *m2,
                             double tolerance = 1e-3);

  /// Get matrix column as a vector
  static void GetOrientationMatrixColumn(vtkMatrix4x4* m, int columnIndex, double columnVector[4]);

  /// Set matrix column from a vector
  static void SetOrientationMatrixColumn(vtkMatrix4x4* m, int columnIndex, double columnVector[4]);

  /// Update orientation vectors of \a dest matrix with values from \a source
  /// matrix.
  static void GetOrientationMatrix(vtkMatrix4x4* source, vtkMatrix3x3* dest);

  /// Update orientation vectors of \a dest matrix with values from \a source
  /// matrix.
  static void SetOrientationMatrix(vtkMatrix3x3* source, vtkMatrix4x4* dest);

  /// Normalizes columns of m matrix in-place and return original column norms in scale vector
  static void NormalizeColumns(vtkMatrix3x3 *m, double scale[3]);

  /// Normalizes columns of top-left 3x3 corner of m matrix in-place and return original column norms in scale vector
  static void NormalizeOrientationMatrixColumns(vtkMatrix4x4 *m, double scale[3]);

  /// Convert a matrix to a string in row-major order
  static std::string ToString(const vtkMatrix4x4* mat, const std::string delimiter = " ", const std::string rowDelimiter = "");

  /// Convert a string in row-major order to a matrix
  static bool FromString(vtkMatrix4x4* mat, const std::string& str, const std::string delimiterExp = "(\\ |\\,|\\:|\\;|\t|\n|\\[|\\])");

protected:
  vtkAddonMathUtilities();
  ~vtkAddonMathUtilities() override;

private:
  vtkAddonMathUtilities(const vtkAddonMathUtilities&) = delete;
  void operator=(const vtkAddonMathUtilities&) = delete;
};

#endif
