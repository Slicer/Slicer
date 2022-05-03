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

#ifndef __vtkMRMLMeasurementAngle_h
#define __vtkMRMLMeasurementAngle_h

// MRML includes
#include "vtkMRMLMeasurement.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"

/// \brief Measurement class calculating angle
/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMeasurementAngle : public vtkMRMLMeasurement
{
public:
  static vtkMRMLMeasurementAngle *New();
  vtkTypeMacro(vtkMRMLMeasurementAngle, vtkMRMLMeasurement);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new instance of this measurement type.
  VTK_NEWINSTANCE
  vtkMRMLMeasurement* CreateInstance() const override { return vtkMRMLMeasurementAngle::New(); }

  /// Calculate length of \sa InputMRMLNode curve markup node and store the result internally
  void Compute() override;

protected:
  vtkMRMLMeasurementAngle();
  ~vtkMRMLMeasurementAngle() override;
  vtkMRMLMeasurementAngle(const vtkMRMLMeasurementAngle&);
  void operator=(const vtkMRMLMeasurementAngle&);
};

#endif
