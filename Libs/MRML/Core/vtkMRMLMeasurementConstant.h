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

#ifndef __vtkMRMLMeasurementConstant_h
#define __vtkMRMLMeasurementConstant_h

// MRML includes
#include "vtkMRMLMeasurement.h"

/// \brief Measurement class storing a constant measurement.
///
/// Typically all measurements calculate their own value from its input
/// MRML node. This class is to be able to store constant measurements.
///
/// \ingroup Slicer_QtModules_Markups
class VTK_MRML_EXPORT vtkMRMLMeasurementConstant : public vtkMRMLMeasurement
{
public:
  static vtkMRMLMeasurementConstant *New();
  vtkTypeMacro(vtkMRMLMeasurementConstant, vtkMRMLMeasurement);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Do nothing to compute the measurement as it is static
  void Compute() override;

protected:
  vtkMRMLMeasurementConstant();
  ~vtkMRMLMeasurementConstant() override;
  vtkMRMLMeasurementConstant(const vtkMRMLMeasurementConstant&);
  void operator=(const vtkMRMLMeasurementConstant&);
};

#endif
