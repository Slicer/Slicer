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

#ifndef __vtkMRMLMeasurementArea_h
#define __vtkMRMLMeasurementArea_h

// MRML includes
#include "vtkMRMLMeasurement.h"

// Markups includes
#include "vtkMRMLExport.h"

/// \brief Measurement class calculating area of a plane or enclosed by a closed curve
class VTK_MRML_EXPORT vtkMRMLMeasurementArea : public vtkMRMLMeasurement
{
public:
  static vtkMRMLMeasurementArea* New();
  vtkTypeMacro(vtkMRMLMeasurementArea, vtkMRMLMeasurement);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new instance of this measurement type.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLMeasurement* CreateInstance() const override { return vtkMRMLMeasurementArea::New(); }

  /// Calculate area of \sa InputMRMLNode plane or closed curve markup node and store the result internally
  void Compute() override;

protected:
  vtkMRMLMeasurementArea();
  ~vtkMRMLMeasurementArea() override;
  vtkMRMLMeasurementArea(const vtkMRMLMeasurementArea&);
  void operator=(const vtkMRMLMeasurementArea&);
};

#endif
