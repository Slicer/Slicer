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

#ifndef __vtkCurveMeasurementsCalculator_h
#define __vtkCurveMeasurementsCalculator_h

// VTK includes
#include <vtkCollection.h>
#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>
#include <vtkWeakPointer.h>

// Export
#include "vtkSlicerMarkupsModuleMRMLExport.h"

/// Filter that generates curves between points of an input polydata
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkCurveMeasurementsCalculator : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCurveMeasurementsCalculator, vtkPolyDataAlgorithm);
  static vtkCurveMeasurementsCalculator* New();

  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetMeasurements(vtkCollection* measurements);
  vtkCollection* GetMeasurements();

  /// TODO:
  vtkSetMacro(CalculateCurvature, bool);
  vtkGetMacro(CalculateCurvature, bool);
  vtkBooleanMacro(CalculateCurvature, bool);

  /// TODO:
  vtkSetMacro(InterpolateControlPointMeasurement, bool);
  vtkGetMacro(InterpolateControlPointMeasurement, bool);
  vtkBooleanMacro(InterpolateControlPointMeasurement, bool);

protected:
  bool CalculatePolyDataCurvature(vtkPolyData* polyData);

protected:
  vtkWeakPointer<vtkCollection> Measurements;
  bool CalculateCurvature;
  bool InterpolateControlPointMeasurement;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

protected:
  vtkCurveMeasurementsCalculator();
  ~vtkCurveMeasurementsCalculator() override;
  vtkCurveMeasurementsCalculator(const vtkCurveMeasurementsCalculator&) = delete;
  void operator=(const vtkCurveMeasurementsCalculator&) = delete;
};

#endif
