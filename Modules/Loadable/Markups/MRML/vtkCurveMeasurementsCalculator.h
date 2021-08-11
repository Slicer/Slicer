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

class vtkCallbackCommand;

/// Filter that generates curves between points of an input polydata
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkCurveMeasurementsCalculator : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCurveMeasurementsCalculator, vtkPolyDataAlgorithm);
  static vtkCurveMeasurementsCalculator* New();

  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /// Set/Get measurement collection.
  /// The measurements that are enabled and contain control point data are used for interpolation
  void SetMeasurements(vtkCollection* measurements);
  vtkCollection* GetMeasurements();
  //@}

  //@{
  /// This indicates whether the curve loops back in on itself,
  /// connecting the last point back to the first point (disabled by default).
  vtkSetMacro(CurveIsClosed, bool);
  vtkGetMacro(CurveIsClosed, bool);
  vtkBooleanMacro(CurveIsClosed, bool);
  //@}

  //@{
  /// Set/Get flag determining whether to calculate curvature
  vtkSetMacro(CalculateCurvature, bool);
  vtkGetMacro(CalculateCurvature, bool);
  vtkBooleanMacro(CalculateCurvature, bool);
  //@}

  /// Get name of mean curvature measurement
  static const char* GetMeanCurvatureName() { return "curvature mean"; };
  /// Get name of max curvature measurement
  static const char* GetMaxCurvatureName() { return "curvature max"; };

  vtkMTimeType GetMTime() override;

  vtkGetMacro(CurvatureUnits, std::string);
  vtkSetMacro(CurvatureUnits, std::string);

  /// Store interpolated values of inputValues in interpolatedValues,
  /// using indices pedigreeIdsArray.
  /// pedigreeIdsValueScale is applied to values of pedigreeIdsArray, which can be used
  /// for converting between indices of curve points and curve control points.
  static bool InterpolateArray(vtkDoubleArray* inputValues, vtkDoubleArray* interpolatedValues,
    vtkDoubleArray* pedigreeIdsArray, double pedigreeIdsValueScale=1.0);

protected:
  bool CalculatePolyDataCurvature(vtkPolyData* polyData);
  bool InterpolateControlPointMeasurementToPolyData(vtkPolyData* outputPolyData);

  /// Callback function observing data array modified events.
  /// If a data array to interpolate is modified, then the interpolation needs to be re-run.
  static void OnControlPointArrayModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

protected:
  /// Measurement list from the markups node for derived measurements (such as interpolation)
  vtkWeakPointer<vtkCollection> Measurements;

  /// Flag indicating whether the current curve is closed
  bool CurveIsClosed{false};

  /// Flag determining whether the filter should calculate curvature
  bool CalculateCurvature{false};

  /// Command handling data array modified events
  vtkCallbackCommand* ControlPointArrayModifiedCallbackCommand;
  /// List of observed control point arrays (for removal of observations)
  vtkCollection* ObservedControlPointArrays;

  std::string CurvatureUnits;

protected:
  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

protected:
  vtkCurveMeasurementsCalculator();
  ~vtkCurveMeasurementsCalculator() override;
  vtkCurveMeasurementsCalculator(const vtkCurveMeasurementsCalculator&) = delete;
  void operator=(const vtkCurveMeasurementsCalculator&) = delete;
};

#endif
