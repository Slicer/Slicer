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

#include "vtkMRMLMarkupsCurveNode.h"

// MRML includes
#include "vtkCurveGenerator.h"
#include "vtkCurveMeasurementsCalculator.h"
#include "vtkEventBroker.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMeasurementLength.h"
#include "vtkMRMLStaticMeasurement.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLUnitNode.h"
#include "vtkProjectMarkupsCurvePointsFilter.h"
#include "vtkSlicerDijkstraGraphGeodesicPath.h"

// VTK includes
#include <vtkArrayCalculator.h>
#include <vtkAssignAttribute.h>
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCellLocator.h>
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkGenericCell.h>
#include <vtkLine.h>
#include <vtkMathUtilities.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkOBBTree.h>
#include <vtkObjectFactory.h>
#include <vtkParallelTransportFrame.h>
#include <vtkPassThrough.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkStringArray.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsCurveNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsCurveNode::vtkMRMLMarkupsCurveNode()
{
  // Set RequiredNumberOfControlPoints to a very high number to remain
  // in place mode after placing a curve point.
  this->RequiredNumberOfControlPoints = 1e6;

  this->CleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();

  this->TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  this->TriangleFilter->SetInputConnection(this->CleanFilter->GetOutputPort());

  this->SurfaceToLocalTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->SurfaceToLocalTransformer->SetTransform(vtkNew<vtkGeneralTransform>());
  this->SurfaceToLocalTransformer->SetInputConnection(this->TriangleFilter->GetOutputPort());

  this->SurfaceScalarCalculator = vtkSmartPointer<vtkArrayCalculator>::New();
  this->SurfaceScalarCalculator->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());
  this->SurfaceScalarCalculator->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
  this->SurfaceScalarCalculator->SetAttributeTypeToPointData();
  this->SurfaceScalarCalculator->SetResultArrayName("weights");
  this->SurfaceScalarCalculator->SetResultArrayType(VTK_FLOAT);
  this->SetSurfaceDistanceWeightingFunction("activeScalar");

  this->SurfaceScalarPassThroughFilter = vtkSmartPointer<vtkPassThrough>::New();
  this->SurfaceScalarPassThroughFilter->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());

  this->CurveGenerator->SetCurveTypeToCardinalSpline();
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(10);
  this->CurveGenerator->SetSurfaceCostFunctionType(vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE);

  this->CurvePolyToWorldTransformer->SetInputConnection(this->CurveGenerator->GetOutputPort());

  this->ProjectPointsFilter = vtkSmartPointer<vtkProjectMarkupsCurvePointsFilter>::New();
  this->ProjectPointsFilter->SetInputCurveNode(this);
  this->ProjectPointsFilter->SetInputConnection(this->CurvePolyToWorldTransformer->GetOutputPort());

  vtkNew<vtkIntArray> events;
  events->InsertNextTuple1(vtkCommand::ModifiedEvent);
  events->InsertNextTuple1(vtkMRMLModelNode::MeshModifiedEvent);
  events->InsertNextTuple1(vtkMRMLTransformableNode::TransformModifiedEvent);
  this->AddNodeReferenceRole(this->GetSurfaceConstraintNodeReferenceRole(), this->GetSurfaceConstraintNodeReferenceMRMLAttributeName(), events);

  this->CurveCoordinateSystemGeneratorWorld->SetInputConnection(this->ProjectPointsFilter->GetOutputPort());

  this->CurveMeasurementsCalculator = vtkSmartPointer<vtkCurveMeasurementsCalculator>::New();
  this->CurveMeasurementsCalculator->SetInputMarkupsMRMLNode(this);
  this->CurveMeasurementsCalculator->SetInputConnection(this->CurveCoordinateSystemGeneratorWorld->GetOutputPort());
  this->CurveMeasurementsCalculator->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);

  this->WorldOutput = vtkSmartPointer<vtkPassThrough>::New();
  this->WorldOutput->SetInputConnection(this->CurveMeasurementsCalculator->GetOutputPort());


  this->ScalarDisplayAssignAttribute = vtkSmartPointer<vtkAssignAttribute>::New();

  this->ShortestDistanceSurfaceActiveScalar = "";

  // Setup measurements calculated for this markup type
  vtkNew<vtkMRMLMeasurementLength> lengthMeasurement;
  lengthMeasurement->SetEnabled(false); // Length measurement is off by default to only show curve name
  lengthMeasurement->SetName("length");
  lengthMeasurement->SetInputMRMLNode(this);
  this->Measurements->AddItem(lengthMeasurement);

  vtkEventBroker* broker = vtkEventBroker::GetInstance();

  vtkNew<vtkMRMLStaticMeasurement> curvatureMeanMeasurement;
  curvatureMeanMeasurement->SetName(this->CurveMeasurementsCalculator->GetMeanCurvatureName());
  curvatureMeanMeasurement->SetEnabled(false); // Curvature calculation is off by default
  this->Measurements->AddItem(curvatureMeanMeasurement);

  vtkNew<vtkMRMLStaticMeasurement> curvatureMaxMeasurement;
  curvatureMaxMeasurement->SetName(this->CurveMeasurementsCalculator->GetMaxCurvatureName());
  curvatureMaxMeasurement->SetEnabled(false); // Curvature calculation is off by default
  this->Measurements->AddItem(curvatureMaxMeasurement);

  this->CurvatureMeasurementModifiedCallbackCommand = vtkCallbackCommand::New();
  this->CurvatureMeasurementModifiedCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->CurvatureMeasurementModifiedCallbackCommand->SetCallback( vtkMRMLMarkupsCurveNode::OnCurvatureMeasurementEnabledModified );
  broker->AddObservation(curvatureMeanMeasurement, vtkCommand::ModifiedEvent, this, this->CurvatureMeasurementModifiedCallbackCommand);
  broker->AddObservation(curvatureMaxMeasurement, vtkCommand::ModifiedEvent, this, this->CurvatureMeasurementModifiedCallbackCommand);

  vtkNew<vtkMRMLStaticMeasurement> torsionMeanMeasurement;
  torsionMeanMeasurement->SetName(this->CurveMeasurementsCalculator->GetMeanTorsionName());
  torsionMeanMeasurement->SetEnabled(false); // Torsion calculation is off by default
  this->Measurements->AddItem(torsionMeanMeasurement);

  vtkNew<vtkMRMLStaticMeasurement> torsionMaxMeasurement;
  torsionMaxMeasurement->SetName(this->CurveMeasurementsCalculator->GetMaxTorsionName());
  torsionMaxMeasurement->SetEnabled(false); // Torsion calculation is off by default
  this->Measurements->AddItem(torsionMaxMeasurement);

  this->TorsionMeasurementModifiedCallbackCommand = vtkCallbackCommand::New();
  this->TorsionMeasurementModifiedCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->TorsionMeasurementModifiedCallbackCommand->SetCallback( vtkMRMLMarkupsCurveNode::OnTorsionMeasurementEnabledModified );
  broker->AddObservation(torsionMeanMeasurement, vtkCommand::ModifiedEvent, this, this->TorsionMeasurementModifiedCallbackCommand);
  broker->AddObservation(torsionMaxMeasurement, vtkCommand::ModifiedEvent, this, this->TorsionMeasurementModifiedCallbackCommand);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsCurveNode::~vtkMRMLMarkupsCurveNode()
{
  if (this->CurvatureMeasurementModifiedCallbackCommand)
    {
    this->CurvatureMeasurementModifiedCallbackCommand->SetClientData(nullptr);
    this->CurvatureMeasurementModifiedCallbackCommand->Delete();
    this->CurvatureMeasurementModifiedCallbackCommand = nullptr;
    }
  if (this->TorsionMeasurementModifiedCallbackCommand)
    {
    this->TorsionMeasurementModifiedCallbackCommand->SetClientData(nullptr);
    this->TorsionMeasurementModifiedCallbackCommand->Delete();
    this->TorsionMeasurementModifiedCallbackCommand = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(curveType, CurveType);
  vtkMRMLWriteXMLIntMacro(numberOfPointsPerInterpolatingSegment, NumberOfPointsPerInterpolatingSegment);
  vtkMRMLWriteXMLEnumMacro(surfaceCostFunctionType, SurfaceCostFunctionType);
  vtkMRMLWriteXMLStringMacro(surfaceDistanceWeightingFunction, SurfaceDistanceWeightingFunction);
  vtkMRMLWriteXMLFloatMacro(surfaceConstraintMaximumSearchRadiusTolerance, SurfaceConstraintMaximumSearchRadiusTolerance);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(curveType, CurveType);
  vtkMRMLReadXMLIntMacro(numberOfPointsPerInterpolatingSegment, NumberOfPointsPerInterpolatingSegment);
  vtkMRMLReadXMLEnumMacro(surfaceCostFunctionType, SurfaceCostFunctionType);
  vtkMRMLReadXMLStringMacro(surfaceDistanceWeightingFunction, SurfaceDistanceWeightingFunction);
  vtkMRMLReadXMLFloatMacro(surfaceConstraintMaximumSearchRadiusTolerance, SurfaceConstraintMaximumSearchRadiusTolerance);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(CurveType);
  vtkMRMLCopyIntMacro(NumberOfPointsPerInterpolatingSegment);
  vtkMRMLCopyEnumMacro(SurfaceCostFunctionType);
  vtkMRMLCopyStringMacro(SurfaceDistanceWeightingFunction);
  vtkMRMLCopyFloatMacro(SurfaceConstraintMaximumSearchRadiusTolerance);
  vtkMRMLCopyEndMacro();

  // Add observers on the measurements computed by the curve measurement calculator. The measurements
  // that were observed were removed in the base class call and need to re-add them in order to work properly.
  vtkEventBroker* broker = vtkEventBroker::GetInstance();

  for (int index = 0; index < this->Measurements->GetNumberOfItems(); ++index)
    {
    vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(this->Measurements->GetItemAsObject(index));
    if ( currentMeasurement->GetName() == this->CurveMeasurementsCalculator->GetMeanCurvatureName()
      || currentMeasurement->GetName() == this->CurveMeasurementsCalculator->GetMaxCurvatureName() )
      {
      if (!broker->GetObservationExist(currentMeasurement, vtkCommand::ModifiedEvent, this, this->CurvatureMeasurementModifiedCallbackCommand))
        {
        broker->AddObservation(currentMeasurement, vtkCommand::ModifiedEvent, this, this->CurvatureMeasurementModifiedCallbackCommand);
        currentMeasurement->Modified();
        }
      }
    if ( currentMeasurement->GetName() == this->CurveMeasurementsCalculator->GetMeanTorsionName()
      || currentMeasurement->GetName() == this->CurveMeasurementsCalculator->GetMaxTorsionName() )
      {
      if (!broker->GetObservationExist(currentMeasurement, vtkCommand::ModifiedEvent, this, this->TorsionMeasurementModifiedCallbackCommand))
        {
        broker->AddObservation(currentMeasurement, vtkCommand::ModifiedEvent, this, this->TorsionMeasurementModifiedCallbackCommand);
        currentMeasurement->Modified();
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(CurveType);
  vtkMRMLPrintIntMacro(NumberOfPointsPerInterpolatingSegment);
  vtkMRMLPrintEnumMacro(SurfaceCostFunctionType);
  vtkMRMLPrintStringMacro(SurfaceDistanceWeightingFunction);
  vtkMRMLPrintFloatMacro(SurfaceConstraintMaximumSearchRadiusTolerance);
  vtkMRMLPrintEndMacro();
}

//---------------------------------------------------------------------------
vtkPoints* vtkMRMLMarkupsCurveNode::GetCurvePointsWorld()
{
  vtkPolyData* curvePoly = this->GetCurveWorld();
  if (!curvePoly)
    {
    return nullptr;
    }
  return curvePoly->GetPoints();
}

//---------------------------------------------------------------------------
vtkPolyData* vtkMRMLMarkupsCurveNode::GetCurveWorld()
{
  if (this->GetNumberOfControlPoints() < 1)
    {
    return nullptr;
    }
  this->WorldOutput->Update();
  auto* curvePolyDataWorld = vtkPolyData::SafeDownCast(this->WorldOutput->GetOutput());
  this->TransformedCurvePolyLocator->SetDataSet(curvePolyDataWorld);
  return curvePolyDataWorld;
}

//----------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLMarkupsCurveNode::GetCurveWorldConnection()
{
  return this->WorldOutput->GetOutputPort();
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsCurveNode::GetCurveLength(vtkPoints* curvePoints, bool closedCurve,
  vtkIdType startCurvePointIndex /*=0*/, vtkIdType numberOfCurvePoints /*=-1*/)
{
  if (!curvePoints || curvePoints->GetNumberOfPoints() < 2)
    {
    return 0.0;
    }
  if (startCurvePointIndex < 0)
    {
    vtkGenericWarningMacro("Invalid startCurvePointIndex=" << startCurvePointIndex << ", using 0 instead");
    startCurvePointIndex = 0;
    }
  vtkIdType lastCurvePointIndex = curvePoints->GetNumberOfPoints()-1;
  if (numberOfCurvePoints >= 0 && startCurvePointIndex + numberOfCurvePoints - 1 < lastCurvePointIndex)
    {
    lastCurvePointIndex = startCurvePointIndex + numberOfCurvePoints - 1;
    }

  double length = 0.0;
  double previousPoint[3] = { 0.0 };
  double nextPoint[3] = { 0.0 };
  curvePoints->GetPoint(startCurvePointIndex, previousPoint);
  for (vtkIdType curvePointIndex = startCurvePointIndex + 1; curvePointIndex <= lastCurvePointIndex; curvePointIndex++)
    {
    curvePoints->GetPoint(curvePointIndex, nextPoint);
    length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
    previousPoint[0] = nextPoint[0];
    previousPoint[1] = nextPoint[1];
    previousPoint[2] = nextPoint[2];
    }
  // Add length of closing segment
  if (closedCurve && (numberOfCurvePoints < 0 || numberOfCurvePoints >= curvePoints->GetNumberOfPoints()))
    {
    curvePoints->GetPoint(0, nextPoint);
    length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
    }
  return length;
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsCurveNode::GetCurveLengthWorld(
  vtkIdType startCurvePointIndex /*=0*/, vtkIdType numberOfCurvePoints /*=-1*/)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  return vtkMRMLMarkupsCurveNode::GetCurveLength(points, this->CurveClosed,
    startCurvePointIndex, numberOfCurvePoints);
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsCurveNode::GetCurveLengthBetweenStartEndPointsWorld(vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex)
{
  if (startCurvePointIndex <= endCurvePointIndex)
    {
    return this->GetCurveLengthWorld(startCurvePointIndex, endCurvePointIndex - startCurvePointIndex + 1);
    }
  else
    {
    // wrap around
    return this->GetCurveLengthWorld(0, endCurvePointIndex + 1) + this->GetCurveLengthWorld(startCurvePointIndex, -1);
    }
}
//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::SetControlPointLabels(vtkStringArray* labels, vtkPoints* points)
{
  return this->SetControlPointLabelsWorld(labels, points);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::ConstrainPointsToSurface(vtkPoints* originalPoints, vtkPoints* normalVectors, vtkPolyData* surfacePolydata,
  vtkPoints* surfacePoints, double maximumSearchRadiusTolerance)
{
  // Convert normals from vtkPoints (the legacy interface) to vtkDoubleArray
  vtkNew<vtkDoubleArray> normalVectorsAsArray;
  normalVectorsAsArray->SetNumberOfComponents(3);
  for (vtkIdType i = 0; i < normalVectors->GetNumberOfPoints(); ++i)
    {
    normalVectorsAsArray->InsertNextTuple(normalVectors->GetPoint(i));
    }

  return vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurface(originalPoints, normalVectorsAsArray, surfacePolydata,
    surfacePoints, maximumSearchRadiusTolerance);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::ResampleCurveWorld(double controlPointDistance)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points || points->GetNumberOfPoints() < 2)
    {
    return;
    }

  vtkNew<vtkPoints> interpolatedPoints;
  vtkNew<vtkDoubleArray> pedigreeIdsArray;
  vtkMRMLMarkupsCurveNode::ResamplePoints(points, interpolatedPoints, controlPointDistance, this->CurveClosed, pedigreeIdsArray);
  vtkMRMLMarkupsCurveNode::ResampleStaticControlPointMeasurements(this->Measurements, pedigreeIdsArray,
    this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment(), this->CurveClosed);

  vtkNew<vtkPoints> originalPoints;
  this->GetControlPointPositionsWorld(originalPoints);
  vtkNew<vtkStringArray> originalLabels;
  this->GetControlPointLabels(originalLabels);

  this->SetControlPointPositionsWorld(interpolatedPoints);
  this->SetControlPointLabelsWorld(originalLabels, originalPoints);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::ResampleStaticControlPointMeasurements(vtkCollection* measurements,
  vtkDoubleArray* curvePointsPedigreeIdsArray, int curvePointsPerControlPoint, bool closedCurve)
{
  if (!measurements || !curvePointsPedigreeIdsArray)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::ResampleCurveWorld: Invalid inputs");
    return false;
    }

  bool success = true;
  for (int index = 0; index < measurements->GetNumberOfItems(); ++index)
    {
    vtkMRMLStaticMeasurement* currentMeasurement = vtkMRMLStaticMeasurement::SafeDownCast(measurements->GetItemAsObject(index));
    if (!currentMeasurement)
      {
      continue;
      }
    vtkDoubleArray* controlPointValues = currentMeasurement->GetControlPointValues();
    if (!controlPointValues || controlPointValues->GetNumberOfTuples() < 2)
      {
      // no need to interpolate
      continue;
      }
    if (controlPointValues->GetNumberOfComponents() != 1)
      {
      vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::ResampleCurveWorld: "
        << "Only the interpolation of single component control point measurements is implemented");
      success = false;
      continue;
      }
    vtkNew<vtkDoubleArray> interpolatedMeasurement;
    interpolatedMeasurement->SetName(controlPointValues->GetName());
    vtkCurveMeasurementsCalculator::InterpolateArray(controlPointValues, closedCurve,
      interpolatedMeasurement, curvePointsPedigreeIdsArray, 1.0/curvePointsPerControlPoint);
    controlPointValues->DeepCopy(interpolatedMeasurement);
    }

  return success;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::ResamplePoints(vtkPoints* originalPoints, vtkPoints* sampledPoints,
  double samplingDistance, bool closedCurve, vtkDoubleArray* pedigreeIdsArray/*=nullptr*/)
{
  if (!originalPoints || !sampledPoints || samplingDistance <= 0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::ResamplePoints failed: invalid inputs");
    return false;
    }

  if (pedigreeIdsArray)
    {
    pedigreeIdsArray->Initialize();
    }

  if (originalPoints->GetNumberOfPoints() < 2)
    {
    sampledPoints->DeepCopy(originalPoints);
    if (pedigreeIdsArray)
      {
      pedigreeIdsArray->InsertNextValue(0.0);
      pedigreeIdsArray->InsertNextValue(1.0);
      }
    return true;
    }

  double distanceFromLastSampledPoint = 0;
  double remainingSegmentLength = 0;
  double previousCurvePoint[3] = { 0.0 };
  originalPoints->GetPoint(0, previousCurvePoint);
  sampledPoints->Reset();
  sampledPoints->InsertNextPoint(previousCurvePoint);
  if (pedigreeIdsArray)
    {
    pedigreeIdsArray->Initialize();
    pedigreeIdsArray->InsertNextValue(0.0);
    }
  vtkIdType numberOfOriginalPoints = originalPoints->GetNumberOfPoints();
  bool addClosingSegment = closedCurve; // for closed curves, add a closing segment that connects last and first points
  double* currentCurvePoint = nullptr;
  for (vtkIdType originalPointIndex = 0; originalPointIndex < numberOfOriginalPoints || addClosingSegment; originalPointIndex++)
    {
    if (originalPointIndex >= numberOfOriginalPoints)
      {
      // this is the closing segment
      addClosingSegment = false;
      currentCurvePoint = originalPoints->GetPoint(0);
      }
    else
      {
      currentCurvePoint = originalPoints->GetPoint(originalPointIndex);
      }

    double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(currentCurvePoint, previousCurvePoint));
    if (segmentLength <= 0.0)
      {
      continue;
      }
    remainingSegmentLength = distanceFromLastSampledPoint + segmentLength;
    if (remainingSegmentLength >= samplingDistance)
      {
      double segmentDirectionVector[3] =
        {
        (currentCurvePoint[0] - previousCurvePoint[0]) / segmentLength,
        (currentCurvePoint[1] - previousCurvePoint[1]) / segmentLength,
        (currentCurvePoint[2] - previousCurvePoint[2]) / segmentLength
        };
      // distance of new sampled point from previous curve point
      double distanceFromLastInterpolatedPoint = samplingDistance - distanceFromLastSampledPoint;
      while (remainingSegmentLength >= samplingDistance)
        {
        double newSampledPoint[3] =
          {
          previousCurvePoint[0] + segmentDirectionVector[0] * distanceFromLastInterpolatedPoint,
          previousCurvePoint[1] + segmentDirectionVector[1] * distanceFromLastInterpolatedPoint,
          previousCurvePoint[2] + segmentDirectionVector[2] * distanceFromLastInterpolatedPoint
          };
        sampledPoints->InsertNextPoint(newSampledPoint);
        distanceFromLastSampledPoint = 0;
        distanceFromLastInterpolatedPoint += samplingDistance;
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->InsertNextValue(originalPointIndex + distanceFromLastInterpolatedPoint/samplingDistance);
          }
        remainingSegmentLength -= samplingDistance;
        }
      distanceFromLastSampledPoint = remainingSegmentLength;
      }
    else
      {
      distanceFromLastSampledPoint += segmentLength;
      }
    previousCurvePoint[0] = currentCurvePoint[0];
    previousCurvePoint[1] = currentCurvePoint[1];
    previousCurvePoint[2] = currentCurvePoint[2];
    }

  // Make sure the resampled curve has the same size as the original
  // but avoid having very long or very short line segments at the end.
  if (closedCurve)
    {
    // Closed curve
    // Ideally, remainingSegmentLength would be equal to samplingDistance.
    if (remainingSegmentLength < samplingDistance * 0.5)
      {
      // last segment would be too short, so remove the last point and adjust position of second last point
      double lastPointPosition[3] = { 0.0 };
      vtkIdType lastPointOriginalPointIndex = 0;
      if (vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, lastPointOriginalPointIndex,
        0, -(2.0*samplingDistance+remainingSegmentLength)/2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetNumberOfValues(pedigreeIdsArray->GetNumberOfValues() - 1);
          pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, lastPointOriginalPointIndex);
          }
        }
      else
        {
        // something went wrong, we could not add a point, therefore just remove the last point
        sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetNumberOfValues(pedigreeIdsArray->GetNumberOfValues() - 1);
          }
        }
      }
    else
      {
      // last segment is only slightly shorter than the sampling distance
      // so just adjust the position of the last point
      double lastPointPosition[3] = { 0.0 };
      vtkIdType lastPointOriginalPointIndex = 0;
      if (vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, lastPointOriginalPointIndex,
        0, -(samplingDistance+remainingSegmentLength)/2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, lastPointOriginalPointIndex);
          }
        }
      }
    }
  else
    {
    // Open curve
    // Ideally, remainingSegmentLength would be equal to 0.
    if (remainingSegmentLength > samplingDistance * 0.5)
      {
      // last segment would be much longer than the sampling distance, so add an extra point
      double secondLastPointPosition[3] = { 0.0 };
      vtkIdType secondLastPointOriginalPointIndex = 0;
      if (vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(secondLastPointPosition, secondLastPointOriginalPointIndex,
        originalPoints->GetNumberOfPoints() - 1, -(samplingDistance+remainingSegmentLength) / 2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, secondLastPointPosition);
        sampledPoints->InsertNextPoint(originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, secondLastPointOriginalPointIndex);
          pedigreeIdsArray->InsertNextValue(originalPoints->GetNumberOfPoints() - 1);
          }
        }
      else
        {
        // something went wrong, we could not add a point, therefore just adjust the last point position
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
          originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
        if (pedigreeIdsArray)
          {
          pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, originalPoints->GetNumberOfPoints() - 1);
          }
        }
      }
    else
      {
      // last segment is only slightly longer than the sampling distance
      // so we just adjust the position of last point
      sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
        originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
      if (pedigreeIdsArray)
        {
        pedigreeIdsArray->SetValue(pedigreeIdsArray->GetNumberOfValues() - 1, originalPoints->GetNumberOfPoints() - 1);
        }
      }
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(double foundCurvePosition[3], vtkIdType& foundClosestPointIndex,
  vtkIdType startCurvePointId, double distanceFromStartPoint, vtkPoints* curvePoints, bool closedCurve)
{
  vtkIdType numberOfCurvePoints = (curvePoints != nullptr ? curvePoints->GetNumberOfPoints() : 0);
  if (numberOfCurvePoints == 0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetPositionAlongCurve failed: invalid input points");
    foundClosestPointIndex = -1;
    return false;
    }
  if (startCurvePointId < 0 || startCurvePointId >= numberOfCurvePoints)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetPositionAlongCurve failed: startCurvePointId is out of range");
    foundClosestPointIndex = -1;
    return false;
    }
  if (numberOfCurvePoints == 1 || distanceFromStartPoint == 0)
    {
    curvePoints->GetPoint(startCurvePointId, foundCurvePosition);
    foundClosestPointIndex = startCurvePointId;
    if (distanceFromStartPoint > 0.0)
      {
      vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetPositionAlongCurve failed: non-zero distance"
        " is requested but only 1 point is available");
      return false;
      }
    else
      {
      return true;
      }
    }
  vtkIdType idIncrement = (distanceFromStartPoint > 0 ? 1 : -1);
  double remainingDistanceFromStartPoint = abs(distanceFromStartPoint);
  double previousPoint[3] = { 0.0 };
  curvePoints->GetPoint(startCurvePointId, previousPoint);
  vtkIdType pointId = startCurvePointId;
  bool curveConfirmedToBeNonZeroLength = false;
  double lastSegmentLength = 0;
  while (true)
    {
    pointId += idIncrement;

    // if reach the end then wrap around for closed curve, terminate search for open curve
    if (pointId < 0 || pointId >= numberOfCurvePoints)
      {
      if (closedCurve)
        {
        if (!curveConfirmedToBeNonZeroLength)
          {
          if (vtkMRMLMarkupsCurveNode::GetCurveLength(curvePoints, closedCurve) == 0.0)
            {
            foundClosestPointIndex = -1;
            return false;
            }
          curveConfirmedToBeNonZeroLength = true;
          }
        pointId = (pointId < 0 ? numberOfCurvePoints : -1);
        continue;
        }
      else
        {
        // reached end of curve before getting at the requested distance
        // return closest
        foundClosestPointIndex = (pointId < 0 ? 0 : numberOfCurvePoints - 1);
        curvePoints->GetPoint(foundClosestPointIndex, foundCurvePosition);
        return false;
        }
      }

    // determine how much closer we are now
    double* nextPoint = curvePoints->GetPoint(pointId);
    lastSegmentLength = sqrt(vtkMath::Distance2BetweenPoints(nextPoint, previousPoint));
    remainingDistanceFromStartPoint -= lastSegmentLength;

    if (remainingDistanceFromStartPoint <= 0)
      {
      // reached the requested distance (and probably a bit more)
      for (int i=0; i<3; i++)
        {
        foundCurvePosition[i] = nextPoint[i] +
          remainingDistanceFromStartPoint * (nextPoint[i] - previousPoint[i]) / lastSegmentLength;
        }
      if (fabs(remainingDistanceFromStartPoint) <= fabs(remainingDistanceFromStartPoint + lastSegmentLength))
        {
        foundClosestPointIndex = pointId;
        }
      else
        {
        foundClosestPointIndex = pointId-1;
        }
      break;
      }

    previousPoint[0] = nextPoint[0];
    previousPoint[1] = nextPoint[1];
    previousPoint[2] = nextPoint[2];
    }
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPointsWorld(vtkPoints* sampledPoints,
  double samplingDistance, vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex)
{
  if (!sampledPoints || samplingDistance <= 0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPoints failed: invalid inputs");
    return false;
    }
  vtkPoints* allPoints = this->GetCurvePointsWorld();
  if (!allPoints)
    {
    return false;
    }
  if (startCurvePointIndex < 0 || endCurvePointIndex >= allPoints->GetNumberOfPoints())
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPoints failed: invalid inputs ("
    << "requested " << startCurvePointIndex << ".." << endCurvePointIndex << " range, but there are "
    << allPoints->GetNumberOfPoints() << " curve points)");
    return false;
    }
  vtkNew<vtkPoints> points;
  if (startCurvePointIndex <= endCurvePointIndex)
    {
    points->InsertPoints(0, endCurvePointIndex - startCurvePointIndex + 1, startCurvePointIndex, allPoints);
    }
  else
    {
    // wrap around
    vtkNew<vtkPoints> points;
    points->InsertPoints(0, allPoints->GetNumberOfPoints() - startCurvePointIndex, startCurvePointIndex, allPoints);
    points->InsertPoints(points->GetNumberOfPoints(), endCurvePointIndex + 1, 0, allPoints);
    }
  return vtkMRMLMarkupsCurveNode::ResamplePoints(points, sampledPoints, samplingDistance, this->CurveClosed);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetClosestCurvePointIndexToPositionWorld(const double posWorld[3])
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points)
    {
    return -1;
    }
  this->TransformedCurvePolyLocator->Update(); // or ->BuildLocator()?
  return this->TransformedCurvePolyLocator->FindClosestPoint(posWorld);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetCurvePointIndexFromControlPointIndex(int controlPointIndex)
{
  if (this->CurveGenerator->IsInterpolatingCurve())
    {
    return controlPointIndex * this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment();
    }
  else
    {
    double controlPointPositionWorld[3] = { 0.0 };
    this->GetNthControlPointPositionWorld(controlPointIndex, controlPointPositionWorld);
    return GetClosestCurvePointIndexToPositionWorld(controlPointPositionWorld);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetCurveDirectionAtPointIndexWorld(vtkIdType curvePointIndex, double directionVectorWorld[3])
{
  vtkNew<vtkMatrix4x4> curvePointToWorld;
  if (!this->GetCurvePointToWorldTransformAtPointIndex(curvePointIndex, curvePointToWorld))
    {
    return false;
    }
  directionVectorWorld[0] = curvePointToWorld->GetElement(0, 2);
  directionVectorWorld[1] = curvePointToWorld->GetElement(1, 2);
  directionVectorWorld[2] = curvePointToWorld->GetElement(2, 2);
  return true;
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetFarthestCurvePointIndexToPositionWorld(const double posWorld[3])
{
  return vtkMRMLMarkupsCurveNode::GetFarthestCurvePointIndexToPosition(this->GetCurvePointsWorld(), posWorld);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetFarthestCurvePointIndexToPosition(vtkPoints* points, const double pos[3])
{
  if (!points || points->GetNumberOfPoints()<1)
    {
    return false;
    }

  double farthestPoint[3] = { 0.0 };
  points->GetPoint(0, farthestPoint);
  double farthestPointDistance2 = vtkMath::Distance2BetweenPoints(pos, farthestPoint);
  vtkIdType farthestPointId = 0;

  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  for (vtkIdType pointIndex = 1; pointIndex < numberOfPoints; pointIndex++)
    {
    double* nextPoint = points->GetPoint(pointIndex);
    double nextPointDistance2 = vtkMath::Distance2BetweenPoints(pos, nextPoint);
    if (nextPointDistance2 > farthestPointDistance2)
      {
      farthestPoint[0] = nextPoint[0];
      farthestPoint[1] = nextPoint[1];
      farthestPoint[2] = nextPoint[2];
      farthestPointDistance2 = nextPointDistance2;
      farthestPointId = pointIndex;
      }
    }

  return farthestPointId;
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetCurvePointIndexAlongCurveWorld(vtkIdType startCurvePointId, double distanceFromStartPoint)
{
  return vtkMRMLMarkupsCurveNode::GetCurvePointIndexAlongCurve(this->GetCurvePointsWorld(), startCurvePointId, distanceFromStartPoint, this->CurveClosed);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetCurvePointIndexAlongCurve(vtkPoints* points, vtkIdType startCurvePointId, double distanceFromStartPoint, bool curveClosed)
{
  double foundCurvePosition[3] = { 0.0 };
  vtkIdType foundClosestPointIndex = -1;
  vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(foundCurvePosition, foundClosestPointIndex,
    startCurvePointId, distanceFromStartPoint, points, curveClosed);
  return foundClosestPointIndex;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetPositionAlongCurveWorld(double foundCurvePosition[3], vtkIdType startCurvePointId, double distanceFromStartPoint)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  vtkIdType foundClosestPointIndex = -1;
  return vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(foundCurvePosition, foundClosestPointIndex,
    startCurvePointId, distanceFromStartPoint, points, this->CurveClosed);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetPointsOnPlaneWorld(vtkPlane* plane, vtkPoints* intersectionPoints)
{
  if (!intersectionPoints)
    {
    return false;
    }
  intersectionPoints->Reset();
  if (!plane)
    {
    return false;
    }
  this->CurveGenerator->Update();
  vtkPolyData* curvePoly = this->GetCurveWorld();
  if (!curvePoly)
    {
    return true;
    }

  vtkNew<vtkCutter> cutEdges;
  cutEdges->SetInputData(curvePoly);
  cutEdges->SetCutFunction(plane);
  cutEdges->GenerateCutScalarsOff();
  cutEdges->SetValue(0, 0);
  cutEdges->Update();
  if (!cutEdges->GetOutput())
    {
    return true;
    }
  vtkPoints* points = cutEdges->GetOutput()->GetPoints();
  if (!points)
    {
    return true;
    }
  intersectionPoints->DeepCopy(points);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex(vtkIdType curvePointIndex, vtkMatrix4x4* curvePointToWorld)
{
  if (!curvePointToWorld)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex failed: Invalid curvePointToWorld");
    return false;
    }
  this->CurveGenerator->Update();
  this->CurveCoordinateSystemGeneratorWorld->Update();
  vtkPolyData* curvePoly = this->CurveCoordinateSystemGeneratorWorld->GetOutput();
  if (!curvePoly)
    {
    return false;
    }
  vtkIdType n = curvePoly->GetNumberOfPoints();
  if (curvePointIndex < 0 || curvePointIndex >= n)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex failed: Invalid curvePointIndex "
      << curvePointIndex << " (number of curve points: " << n << ")");
    return false;
    }
  curvePointToWorld->Identity();
  vtkPointData* pointData = curvePoly->GetPointData();
  if (!pointData)
    {
    return false;
    }
  vtkDoubleArray* normals = vtkDoubleArray::SafeDownCast(
    pointData->GetAbstractArray(this->CurveCoordinateSystemGeneratorWorld->GetNormalsArrayName()));
  vtkDoubleArray* binormals = vtkDoubleArray::SafeDownCast(
    pointData->GetAbstractArray(this->CurveCoordinateSystemGeneratorWorld->GetBinormalsArrayName()));
  vtkDoubleArray* tangents = vtkDoubleArray::SafeDownCast(
    pointData->GetAbstractArray(this->CurveCoordinateSystemGeneratorWorld->GetTangentsArrayName()));
  if (!tangents || !normals || !binormals)
    {
    return false;
    }
  double* normal = normals->GetTuple3(curvePointIndex);
  double* binormal = binormals->GetTuple3(curvePointIndex);
  double* tangent = tangents->GetTuple3(curvePointIndex);
  double* position = curvePoly->GetPoint(curvePointIndex);
  for (int row=0; row<3; row++)
    {
    curvePointToWorld->SetElement(row, 0, normal[row]);
    curvePointToWorld->SetElement(row, 1, binormal[row]);
    curvePointToWorld->SetElement(row, 2, tangent[row]);
    curvePointToWorld->SetElement(row, 3, position[row]);
    }
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetCurveType()
{
  return this->CurveGenerator->GetCurveType();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveType(int type)
{
  this->CurveGenerator->SetCurveType(type);
}

//-----------------------------------------------------------
const char* vtkMRMLMarkupsCurveNode::GetCurveTypeAsString(int id)
{
  return this->CurveGenerator->GetCurveTypeAsString(id);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetCurveTypeFromString(const char* name)
{
  return this->CurveGenerator->GetCurveTypeFromString(name);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToLinear()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToCardinalSpline()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToKochanekSpline()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToPolynomial()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToShortestDistanceOnSurface(vtkMRMLModelNode* modelNode)
{
  MRMLNodeModifyBlocker blocker(this);
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE);
  if (modelNode)
    {
    this->SetAndObserveSurfaceConstraintNode(modelNode);
    }
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetNumberOfPointsPerInterpolatingSegment()
{
  return this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetNumberOfPointsPerInterpolatingSegment(int pointsPerSegment)
{
  if (pointsPerSegment < 1)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::SetNumberOfPointsPerInterpolatingSegment failed: minimum value is 1, attempted to set value " << pointsPerSegment);
    return;
    }
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(pointsPerSegment);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetClosestPointPositionAlongCurveWorld(const double posWorld[3], double closestPos[3])
{
  return vtkMRMLMarkupsCurveNode::GetClosestPointPositionAlongCurve(this->GetCurvePointsWorld(), posWorld, closestPos, this->TransformedCurvePolyLocator);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetClosestPointPositionAlongCurve(vtkPoints* points,
  const double pos[3], double closestPos[3], vtkPointLocator* pointLocator/*=nullptr*/)
{
  if (!points || points->GetNumberOfPoints() < 1)
    {
    return -1;
    }
  if (points->GetNumberOfPoints() == 1)
    {
    points->GetPoint(0, closestPos);
    return -1;
    }

  // Find closest curve point
  vtkIdType closestCurvePointIndex = 0;
  double closestCurvePoint[3] = { 0.0 };
  double closestDistance2 = -1.0;
  if (pointLocator)
    {
    closestCurvePointIndex = pointLocator->FindClosestPoint(pos);
    if (closestCurvePointIndex < 0)
      {
      return -1;
      }
    points->GetPoint(closestCurvePointIndex, closestCurvePoint);
    closestDistance2 = vtkMath::Distance2BetweenPoints(pos, closestPos);
    }
  else
    {
    closestDistance2 = vtkMath::Distance2BetweenPoints(pos, points->GetPoint(0));
    vtkIdType numberOfPoints = points->GetNumberOfPoints();
    for (vtkIdType pointId = 1; pointId < numberOfPoints; pointId++)
      {
      double distance2 = vtkMath::Distance2BetweenPoints(pos, points->GetPoint(pointId));
      if (distance2 < closestDistance2)
        {
        closestCurvePointIndex = pointId;
        closestDistance2 = distance2;
        }
      }
    points->GetPoint(closestCurvePointIndex, closestCurvePoint);
    }
  closestPos[0] = closestCurvePoint[0];
  closestPos[1] = closestCurvePoint[1];
  closestPos[2] = closestCurvePoint[2];
  vtkIdType lineIndex = closestCurvePointIndex;

  // See if we can find any points closer along the curve
  double relativePositionAlongLine = -1.0; // between 0.0-1.0 if between the endpoints of the line segment
  double otherPoint[3] = { 0.0 };
  double closestPointOnLine[3] = { 0.0 };
  if (closestCurvePointIndex - 1 >= 0)
    {
    points->GetPoint(closestCurvePointIndex - 1, otherPoint);
    double distance2 = vtkLine::DistanceToLine(pos, closestCurvePoint, otherPoint, relativePositionAlongLine, closestPointOnLine);
    if (distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      closestPos[0] = closestPointOnLine[0];
      closestPos[1] = closestPointOnLine[1];
      closestPos[2] = closestPointOnLine[2];
      lineIndex = closestCurvePointIndex - 1;
      }
    }
  if (closestCurvePointIndex + 1 < points->GetNumberOfPoints())
    {
    points->GetPoint(closestCurvePointIndex + 1, otherPoint);
    double distance2 = vtkLine::DistanceToLine(pos, closestCurvePoint, otherPoint, relativePositionAlongLine, closestPointOnLine);
    if (distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      closestPos[0] = closestPointOnLine[0];
      closestPos[1] = closestPointOnLine[1];
      closestPos[2] = closestPointOnLine[2];
      lineIndex = closestCurvePointIndex;
      }
    }
  return lineIndex;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::UpdateMeasurementsInternal()
{
  // Execute curve measurements calculator (curvature, interpolate control point measurements
  // and store the results in the curve poly data points as scalars for visualization)
  if (this->CurveMeasurementsCalculator && this->GetNumberOfControlPoints() > 1)
    {
    // Update curvature unit (only do it if a curve measurement is enabled)
    vtkMRMLMeasurement* curvatureMeanMeasurement = this->GetMeasurement(this->CurveMeasurementsCalculator->GetMeanCurvatureName());
    vtkMRMLMeasurement* curvatureMaxMeasurement = this->GetMeasurement(this->CurveMeasurementsCalculator->GetMaxCurvatureName());
    vtkMRMLMeasurement* torsionAvgMeasurement = this->GetMeasurement(this->CurveMeasurementsCalculator->GetMeanTorsionName());
    vtkMRMLMeasurement* torsionMaxMeasurement = this->GetMeasurement(this->CurveMeasurementsCalculator->GetMaxTorsionName());
    if ( (curvatureMeanMeasurement && curvatureMeanMeasurement->GetEnabled())
      || (curvatureMaxMeasurement && curvatureMaxMeasurement->GetEnabled())
      || (torsionAvgMeasurement && torsionAvgMeasurement->GetEnabled())
      || (torsionMaxMeasurement && torsionMaxMeasurement->GetEnabled()))
      {
      std::string inverseLengthUnit = "mm-1";
      vtkMRMLUnitNode* lengthUnitNode = this->GetUnitNode("length");
      if (lengthUnitNode && lengthUnitNode->GetSuffix())
        {
        inverseLengthUnit = std::string(lengthUnitNode->GetSuffix()) + "-1";
        }
      this->CurveMeasurementsCalculator->SetCurvatureUnits(inverseLengthUnit);
      this->CurveMeasurementsCalculator->SetTorsionUnits(inverseLengthUnit);
      }
    this->CurveMeasurementsCalculator->Update();
    }

  Superclass::UpdateMeasurementsInternal();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::ProcessMRMLEvents(vtkObject* caller,
                                             unsigned long event,
                                             void* callData)
{
  if (event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    this->OnSurfaceModelTransformChanged();
    }
  else if (caller == this->SurfaceScalarCalculator.GetPointer())
    {
    this->UpdateAllMeasurements();
    int n = -1;
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    this->StorableModifiedTime.Modified();
    }
  else if (caller == this->CurveGenerator.GetPointer())
    {
    int surfaceCostFunctionType = this->CurveGenerator->GetSurfaceCostFunctionType();
    // Change the pass through filter input depending on if we need the scalar values.
    // Trying to run SurfaceScalarCalculator without an active scalar will result in an error message.
    if (surfaceCostFunctionType == vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE)
      {
      this->SurfaceScalarPassThroughFilter->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());
      }
    else
      {
      this->SurfaceScalarPassThroughFilter->SetInputConnection(this->SurfaceScalarCalculator->GetOutputPort());
      }
    }
  else if (caller == this->CurveMeasurementsCalculator.GetPointer())
    {
    vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
    if (displayNode)
      {
      this->InvokeEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, displayNode);
      }
    }

  if (caller == this->GetNodeReference(this->GetSurfaceConstraintNodeReferenceRole()))
    {
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnNodeReferenceAdded(vtkMRMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetSurfaceConstraintNodeReferenceRole()) == 0 ||
      strcmp(reference->GetReferenceRole(), this->TransformNodeReferenceRole) == 0)
    {
    this->OnSurfaceModelTransformChanged();
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::OnNodeReferenceAdded(reference);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnNodeReferenceModified(vtkMRMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetSurfaceConstraintNodeReferenceRole()) == 0 ||
      strcmp(reference->GetReferenceRole(), this->TransformNodeReferenceRole) == 0)
    {
    this->OnSurfaceModelTransformChanged();
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::OnNodeReferenceModified(reference);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnNodeReferenceRemoved(vtkMRMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetSurfaceConstraintNodeReferenceRole()) == 0 ||
      strcmp(reference->GetReferenceRole(), this->TransformNodeReferenceRole) == 0)
    {
    this->OnSurfaceModelTransformChanged();
    this->OnSurfaceModelNodeChanged();
    }
  Superclass::OnNodeReferenceRemoved(reference);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetAndObserveSurfaceConstraintNode(vtkMRMLModelNode* modelNode)
{
  this->SetAndObserveNodeReferenceID(this->GetSurfaceConstraintNodeReferenceRole(), modelNode ? modelNode->GetID() : nullptr);
}

//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLMarkupsCurveNode::GetSurfaceConstraintNode()
{
  return vtkMRMLModelNode::SafeDownCast(this->GetNodeReference(this->GetSurfaceConstraintNodeReferenceRole()));
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetSurfaceCostFunctionType()
{
  return this->CurveGenerator->GetSurfaceCostFunctionType();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetSurfaceCostFunctionType(int surfaceCostFunctionType)
{
  this->CurveGenerator->SetSurfaceCostFunctionType(surfaceCostFunctionType);
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsCurveNode::GetSurfaceCostFunctionTypeAsString(int surfaceCostFunctionType)
{
  return vtkSlicerDijkstraGraphGeodesicPath::GetCostFunctionTypeAsString(surfaceCostFunctionType);
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetSurfaceCostFunctionTypeFromString(const char* surfaceCostFunctionTypeName)
{
  return vtkSlicerDijkstraGraphGeodesicPath::GetCostFunctionTypeFromString(surfaceCostFunctionTypeName);
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsCurveNode::GetSurfaceDistanceWeightingFunction()
{
  return this->SurfaceScalarCalculator->GetFunction();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetSurfaceDistanceWeightingFunction(const char* function)
{
  const char* currentFunction = this->SurfaceScalarCalculator->GetFunction();
  if ((currentFunction && function && strcmp(this->SurfaceScalarCalculator->GetFunction(), function) == 0) ||
    (currentFunction == nullptr && function == nullptr))
    {
    return;
    }
  this->SurfaceScalarCalculator->SetFunction(function);
  this->UpdateSurfaceScalarVariables();
  this->UpdateAllMeasurements();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnSurfaceModelNodeChanged()
{
  this->UpdateSurfaceScalarVariables();

  vtkMRMLModelNode* modelNode = this->GetSurfaceConstraintNode();
  if (modelNode)
    {
    this->CleanFilter->SetInputConnection(modelNode->GetPolyDataConnection());
    this->CurveGenerator->SetInputConnection(1, this->SurfaceScalarPassThroughFilter->GetOutputPort());
    }
  else
    {
    this->CleanFilter->RemoveAllInputs();
    this->CurveGenerator->RemoveInputConnection(1, this->SurfaceScalarPassThroughFilter->GetOutputPort());
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnSurfaceModelTransformChanged()
{
  vtkMRMLModelNode* modelNode = this->GetSurfaceConstraintNode();
  if (!modelNode)
    {
    return;
    }

  vtkSmartPointer<vtkGeneralTransform> surfaceToLocalTransform = vtkGeneralTransform::SafeDownCast(
    this->SurfaceToLocalTransformer->GetTransform());
  if (!surfaceToLocalTransform)
    {
    surfaceToLocalTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    this->SurfaceToLocalTransformer->SetTransform(surfaceToLocalTransform);
    }

  vtkMRMLTransformNode::GetTransformBetweenNodes(modelNode->GetParentTransformNode(), this->GetParentTransformNode(), surfaceToLocalTransform);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::UpdateSurfaceScalarVariables()
{
  vtkMRMLModelNode* modelNode = this->GetSurfaceConstraintNode();
  if (!modelNode)
    {
    return;
    }

  vtkPolyData* polyData = modelNode->GetPolyData();
  if (!polyData)
    {
    return;
    }

  vtkPointData* pointData = polyData->GetPointData();
  if (!pointData)
    {
    return;
    }

  const char* activeScalarName = modelNode->GetActivePointScalarName(vtkDataSetAttributes::SCALARS);
  bool activeScalarChanged = false;
  if (!activeScalarName && this->ShortestDistanceSurfaceActiveScalar)
    {
    activeScalarChanged = true;
    }
  else if (activeScalarName && !this->ShortestDistanceSurfaceActiveScalar)
    {
    activeScalarChanged = true;
    }
  else if (activeScalarName && this->ShortestDistanceSurfaceActiveScalar && strcmp(activeScalarName, this->ShortestDistanceSurfaceActiveScalar) != 0)
    {
    activeScalarChanged = true;
    }
  this->ShortestDistanceSurfaceActiveScalar = activeScalarName;

  int numberOfArraysInMesh = pointData->GetNumberOfArrays();
  int numberOfArraysInCalculator = this->SurfaceScalarCalculator->GetNumberOfScalarArrays();
  if (!activeScalarChanged && numberOfArraysInMesh + 1 == numberOfArraysInCalculator)
    {
    return;
    }

  this->SurfaceScalarCalculator->RemoveAllVariables();
  for (int i = -1; i < numberOfArraysInMesh; ++i)
    {
    const char* variableName = "activeScalar";
    vtkDataArray* array = nullptr;
    if (i >= 0)
      {
      array = pointData->GetArray(i);
      variableName = array->GetName();
      }
    else
      {
      if (!activeScalarName)
        {
        continue;
        }
      array = pointData->GetArray(activeScalarName);
      }

    if (!array)
      {
      vtkWarningMacro("UpdateSurfaceScalarVariables: Could not get array " << i);
      continue;
      }

    this->SurfaceScalarCalculator->AddScalarVariable(variableName, array->GetName());
    }

  // Changing the variables doesn't invoke modified, so we need to invoke it here.
  this->SurfaceScalarCalculator->Modified();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::UpdateAssignedAttribute()
{
  // TODO: This method only works well if there is a single display node
  // (or all display nodes use the same scalar display settings).
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  this->ScalarDisplayAssignAttribute->Assign(
    displayNode->GetActiveScalarName(),
    vtkDataSetAttributes::SCALARS,
    displayNode->GetActiveAttributeLocation() >= 0 ? displayNode->GetActiveAttributeLocation() : vtkAssignAttribute::POINT_DATA);

  // Connect assign attributes filter if scalar visibility is on
  if (displayNode->GetScalarVisibility())
    {
    this->ScalarDisplayAssignAttribute->SetInputConnection(this->CurveMeasurementsCalculator->GetOutputPort());
    this->WorldOutput->SetInputConnection(this->ScalarDisplayAssignAttribute->GetOutputPort());
    }
  else
    {
    this->ScalarDisplayAssignAttribute->RemoveAllInputConnections(0);
    this->WorldOutput->SetInputConnection(this->CurveMeasurementsCalculator->GetOutputPort());
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnCurvatureMeasurementEnabledModified(
  vtkObject* caller, unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkMRMLMarkupsCurveNode* self = reinterpret_cast<vtkMRMLMarkupsCurveNode*>(clientData);
  vtkMRMLStaticMeasurement* measurement = reinterpret_cast<vtkMRMLStaticMeasurement*>(caller);
  if (!self || !measurement)
    {
    return;
    }

  if (!measurement->GetEnabled())
    {
    // measurement is disabled
    measurement->ClearValue();
    if (!self->CurveMeasurementsCalculator->GetCalculateCurvature())
      {
      // no need to compute and it was not computed, nothing to do
      return;
      }
    // Disable curve measurement calculator if no curvature metric is needed anymore
    bool isCurvatureComputationNeeded = false;
    for (int index = 0; index < self->Measurements->GetNumberOfItems(); ++index)
      {
      vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(self->Measurements->GetItemAsObject(index));
      if (currentMeasurement->GetEnabled()
        && (currentMeasurement->GetName() == self->CurveMeasurementsCalculator->GetMeanCurvatureName()
          || currentMeasurement->GetName() == self->CurveMeasurementsCalculator->GetMaxCurvatureName()))
        {
        isCurvatureComputationNeeded = true;
        break;
        }
      }
    if (!isCurvatureComputationNeeded)
      {
      self->CurveMeasurementsCalculator->SetCalculateCurvature(false);
      self->CurveMeasurementsCalculator->Update();
      }
    return;
    }

  // measurement is enabled
  if (self->CurveMeasurementsCalculator->GetCalculateCurvature() && measurement->GetValueDefined())
    {
    // measurement was already on, nothing to do
    return;
    }

  // trigger a recompute
  self->CurveMeasurementsCalculator->SetCalculateCurvature(true);
  self->CurveMeasurementsCalculator->Update();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnTorsionMeasurementEnabledModified(
  vtkObject* caller, unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkMRMLMarkupsCurveNode* self = reinterpret_cast<vtkMRMLMarkupsCurveNode*>(clientData);
  vtkMRMLStaticMeasurement* measurement = reinterpret_cast<vtkMRMLStaticMeasurement*>(caller);
  if (!self || !measurement)
    {
    return;
    }

  if (!measurement->GetEnabled())
    {
    // measurement is disabled
    measurement->ClearValue();
    if (!self->CurveMeasurementsCalculator->GetCalculateTorsion())
      {
      // no need to compute and it was not computed, nothing to do
      return;
      }
    // Disable curve measurement calculator if no torsion metric is needed anymore
    bool isTorsionComputationNeeded = false;
    for (int index = 0; index < self->Measurements->GetNumberOfItems(); ++index)
      {
      vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(self->Measurements->GetItemAsObject(index));
      if (currentMeasurement->GetEnabled()
        && (currentMeasurement->GetName() == self->CurveMeasurementsCalculator->GetMeanTorsionName()
          || currentMeasurement->GetName() == self->CurveMeasurementsCalculator->GetMaxTorsionName()))
        {
        isTorsionComputationNeeded = true;
        break;
        }
      }
    if (!isTorsionComputationNeeded)
      {
      self->CurveMeasurementsCalculator->SetCalculateTorsion(false);
      self->CurveMeasurementsCalculator->Update();
      }
    return;
    }

  // measurement is enabled
  if (self->CurveMeasurementsCalculator->GetCalculateTorsion() && measurement->GetValueDefined())
    {
    // measurement was already on, nothing to do
    return;
    }

  // trigger a recompute
  self->CurveMeasurementsCalculator->SetCalculateTorsion(true);
  self->CurveMeasurementsCalculator->Update();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetSurfaceConstraintMaximumSearchRadiusTolerance(double tolerance)
{
  if (vtkMathUtilities::FuzzyCompare<double>(this->GetSurfaceConstraintMaximumSearchRadiusTolerance(), tolerance))
    {
    return;
    }
  this->ProjectPointsFilter->SetMaximumSearchRadiusTolerance(tolerance);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsCurveNode::GetSurfaceConstraintMaximumSearchRadiusTolerance() const
{
  return this->ProjectPointsFilter->GetMaximumSearchRadiusTolerance();
}
