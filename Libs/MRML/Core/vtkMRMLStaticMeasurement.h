/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLStaticMeasurement_h
#define __vtkMRMLStaticMeasurement_h

// MRML includes
#include "vtkMRMLMeasurement.h"

/// \brief Measurement class storing a constant measurement.
///
/// Typically all measurements calculate their own value from its input
/// MRML node. This class is to be able to store constant measurements.
///
class VTK_MRML_EXPORT vtkMRMLStaticMeasurement : public vtkMRMLMeasurement
{
public:
  static vtkMRMLStaticMeasurement* New();
  vtkTypeMacro(vtkMRMLStaticMeasurement, vtkMRMLMeasurement);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new instance of this measurement type.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLMeasurement* CreateInstance() const override { return vtkMRMLStaticMeasurement::New(); }

  /// Do nothing to compute the measurement as it is static
  void Compute() override;

protected:
  vtkMRMLStaticMeasurement();
  ~vtkMRMLStaticMeasurement() override;
  vtkMRMLStaticMeasurement(const vtkMRMLStaticMeasurement&);
  void operator=(const vtkMRMLStaticMeasurement&);
};

#endif
