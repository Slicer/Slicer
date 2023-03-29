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

// Markups MRML includes
#include "vtkCurveMeasurementsCalculator.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMeasurement.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkParallelTransportFrame.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTriangleFilter.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkCurveMeasurementsCalculator);

//------------------------------------------------------------------------------
vtkCurveMeasurementsCalculator::vtkCurveMeasurementsCalculator()
{
  this->SetNumberOfInputPorts(1);

  this->ControlPointArrayModifiedCallbackCommand = vtkCallbackCommand::New();
  this->ControlPointArrayModifiedCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->ControlPointArrayModifiedCallbackCommand->SetCallback( vtkCurveMeasurementsCalculator::OnControlPointArrayModified );

  this->ObservedControlPointArrays = vtkCollection::New();

  // timestamps for input and output are the same, initially
  this->Modified();
}

//------------------------------------------------------------------------------
vtkCurveMeasurementsCalculator::~vtkCurveMeasurementsCalculator()
{
  // Remove observations before deleting control point array callback and observed arrays collection
  for (int idx=0; idx<this->ObservedControlPointArrays->GetNumberOfItems(); ++idx)
    {
    vtkDoubleArray* observedArray = vtkDoubleArray::SafeDownCast(this->ObservedControlPointArrays->GetItemAsObject(idx));
    if (observedArray)
      {
      observedArray->RemoveObserver(this->ControlPointArrayModifiedCallbackCommand);
      }
    }

  if (this->ControlPointArrayModifiedCallbackCommand)
    {
    this->ControlPointArrayModifiedCallbackCommand->SetClientData(nullptr);
    this->ControlPointArrayModifiedCallbackCommand->Delete();
    this->ControlPointArrayModifiedCallbackCommand = nullptr;
    }

  if (this->ObservedControlPointArrays)
    {
    this->ObservedControlPointArrays->Delete();
    this->ObservedControlPointArrays = nullptr;
    }
}

//------------------------------------------------------------------------------
void vtkCurveMeasurementsCalculator::PrintSelf(std::ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "CalculateCurvature: " << this->CalculateCurvature << std::endl;
}

//----------------------------------------------------------------------------------
void vtkCurveMeasurementsCalculator::SetInputMarkupsMRMLNode(vtkMRMLMarkupsNode* node)
{
  if (this->InputMarkupsMRMLNode == node)
    {
    return;
    }
  this->InputMarkupsMRMLNode = node;
  this->Modified();
}

//----------------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkCurveMeasurementsCalculator::GetInputMarkupsMRMLNode()
{
  return this->InputMarkupsMRMLNode.GetPointer();
}

//----------------------------------------------------------------------------
int vtkCurveMeasurementsCalculator::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    }
  else
    {
    vtkErrorMacro("Cannot set input info for port " << port);
    return 0;
    }

  return 1;
}

//------------------------------------------------------------------------------
int vtkCurveMeasurementsCalculator::RequestData(
  vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData* inputPolyData = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!this->InputMarkupsMRMLNode)
    {
    return 1;
    }

  if (!inputPolyData ||
      inputPolyData->GetNumberOfPoints() == 0 ||
      inputPolyData->GetNumberOfLines() == 0)
    {
    // Clear values
    for (int index = 0; index < this->InputMarkupsMRMLNode->GetNumberOfMeasurements(); ++index)
      {
      vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(this->InputMarkupsMRMLNode->GetNthMeasurement(index));
      if (!currentMeasurement || currentMeasurement->GetName().empty() || !currentMeasurement->GetEnabled())
        {
        continue;
        }
      if (currentMeasurement->GetName() == this->GetMeanCurvatureName())
        {
        currentMeasurement->ClearValue(vtkMRMLMeasurement::InsufficientInput);
        }
      else if (currentMeasurement->GetName() == this->GetMaxCurvatureName())
        {
        currentMeasurement->ClearValue(vtkMRMLMeasurement::InsufficientInput);
        }
      else if (currentMeasurement->GetName() == this->GetMeanTorsionName())
        {
        currentMeasurement->ClearValue(vtkMRMLMeasurement::InsufficientInput);
        }
      else if (currentMeasurement->GetName() == this->GetMaxTorsionName())
        {
        currentMeasurement->ClearValue(vtkMRMLMeasurement::InsufficientInput);
        }
      }

    return 1;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* outputPolyData = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  outputPolyData->CopyStructure(inputPolyData);
  outputPolyData->GetPointData()->PassData(inputPolyData->GetPointData());
  outputPolyData->GetCellData()->PassData(inputPolyData->GetCellData());
  outputPolyData->GetFieldData()->PassData(inputPolyData->GetFieldData());

  int returnValue = 1;
  if (this->CalculateCurvature)
    {
    if (!this->CalculatePolyDataCurvature(outputPolyData))
      {
      vtkErrorMacro("Failed to calculate curve markup curvature");
      returnValue = 0;
      }
    }
  else
    {
    outputPolyData->GetPointData()->RemoveArray(this->GetCurvatureArrayName());
    }

  if (this->CalculateTorsion)
    {
    if (!this->CalculatePolyDataTorsion(outputPolyData))
      {
      vtkErrorMacro("Failed to calculate curve markup torsion");
      returnValue = 0;
      }
    }
  else
    {
    outputPolyData->GetPointData()->RemoveArray(this->GetTorsionArrayName());
    }

  // Go through measurements, and interpolate those that contain control point data and are enabled
  this->InterpolateControlPointMeasurementToPolyData(outputPolyData);

  outputPolyData->Squeeze();
  return returnValue;
}

//------------------------------------------------------------------------------
bool vtkCurveMeasurementsCalculator::CalculatePolyDataCurvature(vtkPolyData* polyData)
{
  if (polyData == nullptr)
    {
    return false;
    }
  if (polyData->GetNumberOfPoints() == 0 || polyData->GetNumberOfLines() == 0)
    {
    return false;
    }
  if (this->InputMarkupsMRMLNode == nullptr)
    {
    vtkErrorMacro("Failed to find input markups node");
    return false;
    }

  // Note: This algorithm has been ported from CurveMaker and further improved
  //       https://github.com/tokjun/CurveMaker/blob/master/CurveMaker/CurveMaker.py

  vtkPoints* points = polyData->GetPoints();
  vtkCellArray* lines = polyData->GetLines();
  vtkNew<vtkIdList> linePoints;

  lines->GetCell(0, linePoints);
  vtkIdType numberOfPoints = // Last point in closed curve line is the first point
    (this->CurveIsClosed ? linePoints->GetNumberOfIds()-1 : linePoints->GetNumberOfIds());
  if (numberOfPoints < 3)
    {
    vtkErrorMacro("Number of curve points too low for curvature calculation (" << numberOfPoints << "), at least 3 needed");
    return false;
    }

  // Initialize curvature array
  vtkSmartPointer<vtkDoubleArray> curvatureValues = vtkDoubleArray::SafeDownCast(
    polyData->GetPointData()->GetArray(this->GetCurvatureArrayName()));
  if (curvatureValues == nullptr)
    {
    curvatureValues = vtkSmartPointer<vtkDoubleArray>::New();
    }
  curvatureValues->Initialize();
  curvatureValues->SetName(this->GetCurvatureArrayName());
  curvatureValues->SetNumberOfComponents(1);
  curvatureValues->SetNumberOfTuples(numberOfPoints);
  curvatureValues->Reset();
  curvatureValues->FillComponent(0,0.0);

  // Initialize curvature variables
  double minKappa = 0.0;
  double maxKappa = 0.0;
  double meanKappa = 0.0; // Mean is weighted by the length of each segment

  // Get values for first point
  double prevPoint[3] = {points->GetPoint(linePoints->GetId(0))[0], points->GetPoint(linePoints->GetId(0))[1], points->GetPoint(linePoints->GetId(0))[2]}; // pp
  double* currPoint = points->GetPoint(linePoints->GetId(1)); // p
  double diffVector[3] = {currPoint[0]-prevPoint[0], currPoint[1]-prevPoint[1], currPoint[2]-prevPoint[2]};
  double diffNorm = sqrt(diffVector[0]*diffVector[0] + diffVector[1]*diffVector[1] + diffVector[2]*diffVector[2]); // ds
  double normDiffVector[3] = {0.0, 0.0, 0.0}; // T
  double prevNormDiffVector[3] = {diffVector[0]/diffNorm, diffVector[1]/diffNorm, diffVector[2]/diffNorm}; // pT
  double meanPoint[3] = {0.0, 0.0, 0.0}; // m
  double prevMeanPoint[3] = {currPoint[0], currPoint[1], currPoint[2]}; // pm (Skip first point)
  double kappa = 0.0;
  double currentLength = 0.0;
  double length = 0.0;

  // The curvature for the first cell is 0.0 for open curves
  curvatureValues->InsertValue(linePoints->GetId(0), 0.0);

  prevPoint[0] = currPoint[0]; prevPoint[1] = currPoint[1]; prevPoint[2] = currPoint[2];
  for (vtkIdType idx=1; idx<numberOfPoints-1; ++idx)
    {
    currPoint = points->GetPoint(linePoints->GetId(idx+1));

    diffVector[0] = currPoint[0]-prevPoint[0];
    diffVector[1] = currPoint[1]-prevPoint[1];
    diffVector[2] = currPoint[2]-prevPoint[2];
    diffNorm = sqrt(diffVector[0]*diffVector[0] + diffVector[1]*diffVector[1] + diffVector[2]*diffVector[2]);

    normDiffVector[0] = diffVector[0] / diffNorm;
    normDiffVector[1] = diffVector[1] / diffNorm;
    normDiffVector[2] = diffVector[2] / diffNorm;

    // Local curvature
    kappa = sqrt( (normDiffVector[0]-prevNormDiffVector[0])*(normDiffVector[0]-prevNormDiffVector[0])
                + (normDiffVector[1]-prevNormDiffVector[1])*(normDiffVector[1]-prevNormDiffVector[1])
                + (normDiffVector[2]-prevNormDiffVector[2])*(normDiffVector[2]-prevNormDiffVector[2]) )
            / diffNorm;
    curvatureValues->InsertValue(linePoints->GetId(idx), kappa);

    // Statistics
    meanPoint[0] = (currPoint[0]+prevPoint[0]) / 2.0;
    meanPoint[1] = (currPoint[1]+prevPoint[1]) / 2.0;
    meanPoint[2] = (currPoint[2]+prevPoint[2]) / 2.0;

    currentLength = sqrt( (meanPoint[0]-prevMeanPoint[0])*(meanPoint[0]-prevMeanPoint[0])
                        + (meanPoint[1]-prevMeanPoint[1])*(meanPoint[1]-prevMeanPoint[1])
                        + (meanPoint[2]-prevMeanPoint[2])*(meanPoint[2]-prevMeanPoint[2]) );
    if (kappa < minKappa)
      {
      minKappa = kappa;
      }
    else if (kappa > maxKappa)
      {
      maxKappa = kappa;
      }
    meanKappa += kappa * currentLength; // weighted mean
    length += currentLength;

    // Propagate current values to previous
    for (int i=0; i<3; ++i)
      {
      prevPoint[i] = currPoint[i];
      prevMeanPoint[i] = meanPoint[i];
      prevNormDiffVector[i] = normDiffVector[i];
      }
    } // For each line point

  if (!this->CurveIsClosed)
    {
    // The curvature for the last cell by definition is 0.0 for open curves
    curvatureValues->InsertValue(linePoints->GetId(numberOfPoints-1), 0.0);
    }
  else
    {
    // Use the adjacent values for closed curve instead of the singular values
    curvatureValues->SetComponent(linePoints->GetId(0), 0,
      curvatureValues->GetValue(linePoints->GetId(1)));
    curvatureValues->InsertValue(linePoints->GetId(numberOfPoints-1),
      curvatureValues->GetValue(linePoints->GetId(numberOfPoints-2)));
    }

  currentLength = sqrt( (prevPoint[0]-prevMeanPoint[0])*(prevPoint[0]-prevMeanPoint[0])
                      + (prevPoint[1]-prevMeanPoint[1])*(prevPoint[1]-prevMeanPoint[1])
                      + (prevPoint[2]-prevMeanPoint[2])*(prevPoint[2]-prevMeanPoint[2]) );
  length += currentLength;
  if (length > 0.0)
    {
    meanKappa = meanKappa / length;
    }
  else
    {
    meanKappa = 0.0;
    }

  // Set mean and max curvature to measurements
  // Calculate and set interpolated control point measurements in poly data
  for (int index=0; index<this->InputMarkupsMRMLNode->GetNumberOfMeasurements(); ++index)
    {
    vtkMRMLMeasurement* currentMeasurement = this->InputMarkupsMRMLNode->GetNthMeasurement(index);
    if (!currentMeasurement || currentMeasurement->GetName().empty() || !currentMeasurement->GetEnabled())
      {
      continue;
      }
    if (currentMeasurement->GetName() == this->GetMeanCurvatureName())
      {
      currentMeasurement->SetDisplayValue(meanKappa, this->CurvatureUnits.c_str());
      currentMeasurement->Compute(); // Have the measurement set the computation result to OK
      }
    else if (currentMeasurement->GetName() == this->GetMaxCurvatureName())
      {
      currentMeasurement->SetDisplayValue(maxKappa, this->CurvatureUnits.c_str());
      currentMeasurement->Compute(); // Have the measurement set the computation result to OK
      }
    }

  // Set curvature array to output
  polyData->GetPointData()->AddArray(curvatureValues);

  return true;
}

//------------------------------------------------------------------------------
bool vtkCurveMeasurementsCalculator::CalculatePolyDataTorsion(vtkPolyData* polyData)
{
  if (polyData == nullptr)
    {
    return false;
    }
  if (polyData->GetNumberOfPoints() == 0 || polyData->GetNumberOfLines() == 0)
    {
    return false;
    }
  if (this->InputMarkupsMRMLNode == nullptr)
    {
    vtkErrorMacro("Failed to find input markups node");
    return false;
    }

  // Note: This algorithm has been implemented based on the information at https://mathworld.wolfram.com/Torsion.html
  //       and based on the implementation of CalculatePolyDataCurvature

  // Get binormals and tangents arrays
  vtkParallelTransportFrame* curveCoordinateSystemGenerator = this->InputMarkupsMRMLNode->GetCurveCoordinateSystemGeneratorWorld();
  if (!curveCoordinateSystemGenerator)
    {
    vtkErrorMacro("Failed to access coordinate system generator");
    return false;
    }
  vtkPointData* pointData = polyData->GetPointData();
  vtkDoubleArray* tangents = vtkDoubleArray::SafeDownCast(
    pointData->GetAbstractArray(curveCoordinateSystemGenerator->GetTangentsArrayName()));
  vtkDoubleArray* binormals = vtkDoubleArray::SafeDownCast(
    pointData->GetAbstractArray(curveCoordinateSystemGenerator->GetBinormalsArrayName()));
  // Sanity checks
  vtkPolyData* curvePoly = curveCoordinateSystemGenerator->GetOutput();
  if (curvePoly->GetNumberOfPoints() != polyData->GetNumberOfPoints())
    {
    // Sanity check
    vtkErrorMacro("Number of points does not match between given poly data and the coordinate system generator poly data ("
      << curvePoly->GetNumberOfPoints() << " != " <<  polyData->GetNumberOfPoints() << ")");
    return false;
    }
  if (binormals->GetNumberOfTuples() != pointData->GetNumberOfTuples())
    {
    vtkErrorMacro("Number of data points does not match between coordinate system generator ("
      << binormals->GetNumberOfTuples() << ") and poly data point data (" << pointData->GetNumberOfTuples() << ")");
    return false;
    }

  vtkCellArray* lines = polyData->GetLines();
  vtkNew<vtkIdList> linePoints;

  lines->GetCell(0, linePoints);
  vtkIdType numberOfPoints = // Last point in closed curve line is the first point
    (this->CurveIsClosed ? linePoints->GetNumberOfIds()-1 : linePoints->GetNumberOfIds());
  if (numberOfPoints < 3)
    {
    vtkErrorMacro("Number of curve points too low for torsion calculation (" << numberOfPoints << "), at least 3 needed");
    return false;
    }

  // Initialize torsion array
  vtkSmartPointer<vtkDoubleArray> torsionArray = vtkDoubleArray::SafeDownCast(
    polyData->GetPointData()->GetArray(this->GetTorsionArrayName()));
  if (torsionArray == nullptr)
    {
    torsionArray = vtkSmartPointer<vtkDoubleArray>::New();
    }
  torsionArray->Initialize();
  torsionArray->SetName(this->GetTorsionArrayName());
  torsionArray->SetNumberOfComponents(1);
  torsionArray->SetNumberOfTuples(numberOfPoints);
  torsionArray->Reset();
  torsionArray->FillComponent(0,0.0);

  // Initialize curvature variables
  double minTorsion= 0.0;
  double maxTorsion = 0.0;
  double meanTorsion = 0.0;

  // Get values for first point
  double* binormal = binormals->GetTuple3(linePoints->GetId(1));
  double binormalNorm = sqrt(binormal[0]*binormal[0] + binormal[1]*binormal[1] + binormal[2]*binormal[2]);
  double normBinormal[3] = {0.0, 0.0, 0.0};
  double prevNormBinormal[3] = {binormal[0]/binormalNorm, binormal[1]/binormalNorm, binormal[2]/binormalNorm};
  double torsion = 0.0;
  double *tangent = nullptr;
  double currentLength = 0.0;
  double length = 0.0;

  // The torsion for the first cell is 0.0 for open curves
  torsionArray->InsertValue(linePoints->GetId(0), 0.0);

  for (vtkIdType idx=1; idx<numberOfPoints-1; ++idx)
    {
    binormal = binormals->GetTuple3(linePoints->GetId(idx+1));
    binormalNorm = sqrt(binormal[0]*binormal[0] + binormal[1]*binormal[1] + binormal[2]*binormal[2]);

    normBinormal[0] = binormal[0] / binormalNorm;
    normBinormal[1] = binormal[1] / binormalNorm;
    normBinormal[2] = binormal[2] / binormalNorm;

    // Local torsion
    torsion = sqrt( (normBinormal[0]-prevNormBinormal[0])*(normBinormal[0]-prevNormBinormal[0])
                  + (normBinormal[1]-prevNormBinormal[1])*(normBinormal[1]-prevNormBinormal[1])
                  + (normBinormal[2]-prevNormBinormal[2])*(normBinormal[2]-prevNormBinormal[2]) )
              / binormalNorm;
    torsionArray->InsertValue(linePoints->GetId(idx), torsion);

    // Statistics
    tangent = tangents->GetTuple3(linePoints->GetId(idx));
    currentLength = sqrt( tangent[0]*tangent[0] + tangent[1]*tangent[1] + tangent[2]*tangent[2] );
    if (torsion < minTorsion)
      {
      minTorsion = torsion;
      }
    else if (torsion > maxTorsion)
      {
      maxTorsion = torsion;
      }
    meanTorsion += torsion * currentLength; // weighted mean
    length += currentLength;

    // Propagate current values to previous
    for (int i=0; i<3; ++i)
      {
      prevNormBinormal[i] = normBinormal[i];
      }
    } // For each line point

  // Use the adjacent values for instead of the singular values
  if (!this->CurveIsClosed)
    {
    torsionArray->InsertValue(linePoints->GetId(numberOfPoints-1),
      torsionArray->GetValue(linePoints->GetId(numberOfPoints-2)));
    }
  else
    {
    torsionArray->SetComponent(linePoints->GetId(0), 0,
      torsionArray->GetValue(linePoints->GetId(1)));
    torsionArray->InsertValue(linePoints->GetId(numberOfPoints-1),
      torsionArray->GetValue(linePoints->GetId(numberOfPoints-2)));
    }

  tangent = tangents->GetTuple3(linePoints->GetId(numberOfPoints-1));
  currentLength = sqrt( tangent[0]*tangent[0] + tangent[1]*tangent[1] + tangent[2]*tangent[2] );
  length += currentLength;
  if (length > 0.0)
    {
    meanTorsion = meanTorsion / length;
    }
  else
    {
    meanTorsion = 0.0;
    }

  // Set mean and max torsion to measurements
  // Calculate and set interpolated control point measurements in poly data
  for (int index=0; index<this->InputMarkupsMRMLNode->GetNumberOfMeasurements(); ++index)
    {
    vtkMRMLMeasurement* currentMeasurement = this->InputMarkupsMRMLNode->GetNthMeasurement(index);
    if (!currentMeasurement || currentMeasurement->GetName().empty() || !currentMeasurement->GetEnabled())
      {
      continue;
      }
    if (currentMeasurement->GetName() == this->GetMeanTorsionName())
      {
      currentMeasurement->SetDisplayValue(meanTorsion, this->TorsionUnits.c_str());
      currentMeasurement->Compute(); // Have the measurement set the computation result to OK
      }
    else if (currentMeasurement->GetName() == this->GetMaxTorsionName())
      {
      currentMeasurement->SetDisplayValue(maxTorsion, this->TorsionUnits.c_str());
      currentMeasurement->Compute(); // Have the measurement set the computation result to OK
      }
    }

  // Set torsion array to output
  polyData->GetPointData()->AddArray(torsionArray);

  return true;
}

//------------------------------------------------------------------------------
bool vtkCurveMeasurementsCalculator::InterpolateControlPointMeasurementToPolyData(vtkPolyData* outputPolyData)
{
  if (!this->InputMarkupsMRMLNode)
    {
    return false;
    }
  if (!outputPolyData)
    {
    return false;
    }
  vtkIdType numberOfPoints = outputPolyData->GetNumberOfPoints();
  if (numberOfPoints == 0 || outputPolyData->GetNumberOfLines() == 0)
    {
    return false;
    }
  vtkDoubleArray* pedigreeIdsArray = vtkDoubleArray::SafeDownCast(outputPolyData->GetPointData()->GetAbstractArray("PedigreeIDs"));
  if (!pedigreeIdsArray)
    {
    vtkErrorMacro("InterpolateControlPointMeasurementToPolyData: Missing PedigreeIDs array in the curve poly data");
    return false;
    }
  if (pedigreeIdsArray->GetNumberOfTuples() != numberOfPoints)
    {
    vtkErrorMacro("InterpolateControlPointMeasurementToPolyData: Size mismatch between PedigreeIDs array ("
      << pedigreeIdsArray->GetNumberOfTuples() << ") and polydata points (" << numberOfPoints << ")");
    return false;
    }

  // Calculate and set interpolated control point measurements in poly data
  for (int index=0; index<this->InputMarkupsMRMLNode->GetNumberOfMeasurements(); ++index)
    {
    vtkMRMLMeasurement* currentMeasurement = this->InputMarkupsMRMLNode->GetNthMeasurement(index);
    if (!currentMeasurement || !currentMeasurement->GetEnabled())
      {
      continue;
      }
    vtkDoubleArray* controlPointValues = currentMeasurement->GetControlPointValues();
    if (!controlPointValues || controlPointValues->GetNumberOfTuples() < 2)
      {
      continue;
      }
    if (controlPointValues->GetNumberOfComponents() != 1)
      {
      //TODO: Add support for more components
      vtkWarningMacro("InterpolateControlPointMeasurementToPolyData: Only the interpolation of single component control point measurements is implemented");
      return false;
      }

    // Observe control point data array. If it is modified, then interpolation needs to be re-run
    controlPointValues->AddObserver(vtkCommand::ModifiedEvent, this->ControlPointArrayModifiedCallbackCommand);
    vtkWeakPointer<vtkDoubleArray> controlPointArrayWeakPointer(controlPointValues);
    this->ObservedControlPointArrays->AddItem(controlPointArrayWeakPointer);

    vtkNew<vtkDoubleArray> interpolatedMeasurement;
    std::string arrayName = !currentMeasurement->GetName().empty() ? currentMeasurement->GetName() : "Unnamed";
    interpolatedMeasurement->SetName(arrayName.c_str());

    if (!vtkCurveMeasurementsCalculator::InterpolateArray(controlPointValues, this->CurveIsClosed, interpolatedMeasurement, pedigreeIdsArray, 1.0))
      {
      vtkErrorMacro("Failed to add " + arrayName + " measurement array to curve");
      continue;
      }

    outputPolyData->GetPointData()->AddArray(interpolatedMeasurement);
    }

  return true;
}

//------------------------------------------------------------------------------
bool vtkCurveMeasurementsCalculator::InterpolateArray(vtkDoubleArray* inputValues, bool closedCurve, vtkDoubleArray* interpolatedValues,
  vtkDoubleArray* pedigreeIdsArray, double pedigreeIdsValueScale/*=1.0*/)
{
  if (!inputValues || !interpolatedValues || !pedigreeIdsArray)
    {
    vtkGenericWarningMacro("vtkCurveMeasurementsCalculator::InterpolateArray: invalid input array");
    return false;
    }
  vtkIdType numberOfValues = pedigreeIdsArray->GetNumberOfValues();
  if (inputValues->GetNumberOfComponents() != 1)
    {
    //TODO: Add support for more components
    vtkGenericWarningMacro("vtkCurveMeasurementsCalculator::InterpolateArray: Only the interpolation of single component values is implemented");
    return false;
    }
  interpolatedValues->Reset(); // empty without reallocating memory
  if (numberOfValues < 1)
    {
    return true;
    }
  // Pedigree IDs wrap around if curve is closed. IDs of the last segment are between go up to the number of control points.
  vtkIdType lastValidControlPointIndex = (closedCurve ? inputValues->GetNumberOfTuples() : inputValues->GetNumberOfTuples() - 1);
  double pedigreeRange[2] = { 0.0, 0.0 };
  pedigreeIdsArray->GetValueRange(pedigreeRange);
  if (pedigreeRange[0] * pedigreeIdsValueScale < -VTK_DBL_EPSILON
    || pedigreeRange[1] * pedigreeIdsValueScale > lastValidControlPointIndex + VTK_DBL_EPSILON)
    {
    vtkGenericWarningMacro("vtkCurveMeasurementsCalculator::InterpolateArray: pedigreeIdsArray contain values between "
      << pedigreeRange[0] * pedigreeIdsValueScale << " and " << pedigreeRange[1] * pedigreeIdsValueScale << ", but there are only "
      << inputValues->GetNumberOfTuples() << " values in the input array");
    return false;
    }
  if (numberOfValues == 1)
    {
    interpolatedValues->InsertNextValue(inputValues->GetValue(0));
    return true;
    }
  // Perform interpolation on the control points measurement values in each enabled measurement
  for (vtkIdType pointIdx = 0; pointIdx < numberOfValues; ++pointIdx)
    {
    // Based on the pedigree IDs calculate the interpolated value for each point in the polydata
    double pedigreeID = pedigreeIdsArray->GetValue(pointIdx) * pedigreeIdsValueScale;
    vtkIdType controlPointIndex = vtkIdType(pedigreeID);
    double fractionValue = pedigreeID - controlPointIndex;
    double currentControlPointValue = inputValues->GetValue(controlPointIndex % inputValues->GetNumberOfTuples());
    if (fractionValue < VTK_DBL_EPSILON)
      {
      // Point corresponds to a control point
      interpolatedValues->InsertValue(pointIdx, currentControlPointValue);
      }
    else
      {
      // Need to interpolate
      double nextControlPointValue = inputValues->GetValue((controlPointIndex + 1) % inputValues->GetNumberOfTuples());
      double interpolatedValue = currentControlPointValue + fractionValue * (nextControlPointValue-currentControlPointValue);
      interpolatedValues->InsertValue(pointIdx, interpolatedValue);
      }
    }
  return true;
}

//---------------------------------------------------------------------------
void vtkCurveMeasurementsCalculator::OnControlPointArrayModified(
  vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkCurveMeasurementsCalculator* self = reinterpret_cast<vtkCurveMeasurementsCalculator*>(clientData);
  self->Modified();
}

//---------------------------------------------------------------------------
vtkMTimeType vtkCurveMeasurementsCalculator::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  // Modified time is the latest modified time of all measurements
  if (this->InputMarkupsMRMLNode)
    {
    for (int index=0; index<this->InputMarkupsMRMLNode->GetNumberOfMeasurements(); ++index)
      {
      vtkMRMLMeasurement* currentMeasurement = this->InputMarkupsMRMLNode->GetNthMeasurement(index);
      vtkMTimeType measurementMTime = currentMeasurement->GetMTime();
      if (measurementMTime > mTime)
        {
        mTime = measurementMTime;
        }
      }
    }
  return mTime;
}
