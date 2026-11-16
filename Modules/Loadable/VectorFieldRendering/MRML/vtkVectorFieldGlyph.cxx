/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// MRML includes
#include "vtkVectorFieldGlyph.h"
#include "vtkVectorFieldSampler.h"

// vtkAddon includes
#include <vtkAddonMathUtilities.h>

// VTK includes
#include <vtkArrowSource.h>
#include <vtkCellArray.h>
#include <vtkConeSource.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkGeometryFilter.h>
#include <vtkGlyph3DMapper.h>
#include <vtkGlyphSource2D.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkLineSource.h>
#include <vtkMaskPoints.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
const char DirectionalScaleArrayName[] = "GlyphDirectionalScale";
const char ProjectedArrayNameSuffix[] = " projected";

//----------------------------------------------------------------------------
/// Rotation that takes +X onto a direction, as vtkGlyph3DMapper (DIRECTION mode) and
/// vtkGlyph3D compute it: a 180-degree rotation about the bisector of +X and the direction.
void GetDirectionRotation(const double direction[3], double rotation[3][3])
{
  vtkMath::Identity3x3(rotation);
  if (direction[1] == 0.0 && direction[2] == 0.0)
  {
    if (direction[0] < 0.0)
    {
      // 180-degree rotation about Y
      rotation[0][0] = -1.0;
      rotation[2][2] = -1.0;
    }
    return;
  }
  double length = vtkMath::Norm(direction);
  double axis[3] = { (direction[0] + length) / 2.0, direction[1] / 2.0, direction[2] / 2.0 };
  vtkMath::Normalize(axis);
  // 180-degree rotation about a unit axis: 2 a a^T - I
  for (int row = 0; row < 3; ++row)
  {
    for (int column = 0; column < 3; ++column)
    {
      rotation[row][column] = 2.0 * axis[row] * axis[column] - (row == column ? 1.0 : 0.0);
    }
  }
}
} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVectorFieldGlyph);

//----------------------------------------------------------------------------
vtkVectorFieldGlyph::vtkVectorFieldGlyph()
{
  this->SetNumberOfOutputPorts(3);
  this->MaskPoints->GenerateVerticesOff();
  this->MaskPoints->RandomModeOn();
  this->ConeSource->SetHeight(1.0);
  vtkNew<vtkTransform> cylinderToX;
  cylinderToX->RotateZ(90.0);
  this->CylinderRotation->SetTransform(cylinderToX);
  this->CylinderRotation->SetInputConnection(this->CylinderSource->GetOutputPort());
  this->FlatSource->SetScale(1.0);
  this->FlatSource->SetFilled(0);
  this->FlatSourceShift->SetInputConnection(this->FlatSource->GetOutputPort());
}

//----------------------------------------------------------------------------
vtkVectorFieldGlyph::~vtkVectorFieldGlyph() = default;

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ThresholdEnabled: " << (this->ThresholdEnabled ? "On" : "Off") << "\n";
  os << indent << "ThresholdArrayName: " << this->ThresholdArrayName << "\n";
  os << indent << "ThresholdComponent: " << this->ThresholdComponent << "\n";
  os << indent << "ThresholdRange: " << this->ThresholdRange[0] << " " << this->ThresholdRange[1] << "\n";
  os << indent << "RegionArrayName: " << this->RegionArrayName << "\n";
  os << indent << "SubsetMode: " << this->SubsetMode << "\n";
  os << indent << "MaximumNumberOfPoints: " << this->MaximumNumberOfPoints << "\n";
  os << indent << "SubsetSpacing: " << this->SubsetSpacing << "\n";
  os << indent << "RandomSeed: " << this->RandomSeed << "\n";
  os << indent << "OrientationArrayName: " << this->OrientationArrayName << "\n";
  os << indent << "ScaleArrayName: " << this->ScaleArrayName << "\n";
  os << indent << "ScaleMode: " << this->ScaleMode << "\n";
  os << indent << "ScaleFactor: " << this->ScaleFactor << "\n";
  os << indent << "DirectionalScale: " << (this->DirectionalScale ? "On" : "Off") << "\n";
  os << indent << "GlyphType: " << this->GlyphType << "\n";
  os << indent << "Resolution: " << this->Resolution << "\n";
  os << indent << "TipLength: " << this->TipLength << "\n";
  os << indent << "TipRadius: " << this->TipRadius << "\n";
  os << indent << "ShaftRadius: " << this->ShaftRadius << "\n";
  os << indent << "Flat: " << (this->Flat ? "On" : "Off") << "\n";
  os << indent << "GlyphPlane: " << (this->GlyphPlane ? "set" : "(none)") << "\n";
  os << indent << "ProjectToPlane: " << (this->ProjectToPlane ? "On" : "Off") << "\n";
  os << indent << "GenerateGeometry: " << (this->GenerateGeometry ? "On" : "Off") << "\n";
}

//----------------------------------------------------------------------------
const char* vtkVectorFieldGlyph::GetDirectionalScaleArrayName()
{
  return DirectionalScaleArrayName;
}

//----------------------------------------------------------------------------
const char* vtkVectorFieldGlyph::GetProjectedArrayNameSuffix()
{
  return ProjectedArrayNameSuffix;
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::SetThresholdArrayName(const std::string& name)
{
  if (this->ThresholdArrayName == name)
  {
    return;
  }
  this->ThresholdArrayName = name;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::SetRegionArrayName(const std::string& name)
{
  if (this->RegionArrayName == name)
  {
    return;
  }
  this->RegionArrayName = name;
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVectorFieldGlyph::OutputUsesMapperSettings()
{
  return (this->GenerateGeometry || this->DirectionalScale || this->ProjectToPlane);
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::SetOrientationArrayName(const std::string& name)
{
  if (this->OrientationArrayName == name)
  {
    return;
  }
  this->OrientationArrayName = name;
  if (this->OutputUsesMapperSettings())
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::SetScaleArrayName(const std::string& name)
{
  if (this->ScaleArrayName == name)
  {
    return;
  }
  this->ScaleArrayName = name;
  if (this->OutputUsesMapperSettings())
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::SetScaleMode(int scaleMode)
{
  scaleMode = std::max(static_cast<int>(ScaleNone), std::min(scaleMode, static_cast<int>(Scale_Last) - 1));
  if (this->ScaleMode == scaleMode)
  {
    return;
  }
  this->ScaleMode = scaleMode;
  if (this->OutputUsesMapperSettings())
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::SetScaleFactor(double scaleFactor)
{
  if (this->ScaleFactor == scaleFactor)
  {
    return;
  }
  this->ScaleFactor = scaleFactor;
  if (this->OutputUsesMapperSettings())
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::SetGlyphPlane(vtkMatrix4x4* planeToInput)
{
  if (!planeToInput)
  {
    if (this->GlyphPlane)
    {
      this->GlyphPlane = nullptr;
      this->Modified();
    }
    return;
  }
  if (!this->GlyphPlane)
  {
    this->GlyphPlane = vtkSmartPointer<vtkMatrix4x4>::New();
  }
  else if (vtkAddonMathUtilities::MatrixAreEqual(this->GlyphPlane, planeToInput, 0.0))
  {
    return;
  }
  this->GlyphPlane->DeepCopy(planeToInput);
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkVectorFieldGlyph::GetGlyphPlane()
{
  return this->GlyphPlane;
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::CopySettings(vtkVectorFieldGlyph* source)
{
  if (!source || source == this)
  {
    return;
  }
  this->SetThresholdEnabled(source->ThresholdEnabled);
  this->SetThresholdArrayName(source->ThresholdArrayName);
  this->SetThresholdComponent(source->ThresholdComponent);
  this->SetThresholdRange(source->ThresholdRange);
  this->SetRegionArrayName(source->RegionArrayName);
  this->SetSubsetMode(source->SubsetMode);
  this->SetMaximumNumberOfPoints(source->MaximumNumberOfPoints);
  this->SetSubsetSpacing(source->SubsetSpacing);
  this->SetRandomSeed(source->RandomSeed);
  this->SetGenerateGeometry(source->GenerateGeometry);
  this->SetDirectionalScale(source->DirectionalScale);
  this->SetProjectToPlane(source->ProjectToPlane);
  this->SetOrientationArrayName(source->OrientationArrayName);
  this->SetScaleArrayName(source->ScaleArrayName);
  this->SetScaleMode(source->ScaleMode);
  this->SetScaleFactor(source->ScaleFactor);
  this->SetGlyphType(source->GlyphType);
  this->SetResolution(source->Resolution);
  this->SetTipLength(source->TipLength);
  this->SetTipRadius(source->TipRadius);
  this->SetShaftRadius(source->ShaftRadius);
  this->SetFlat(source->Flat);
  this->SetGlyphPlane(source->GlyphPlane);
}

//----------------------------------------------------------------------------
std::string vtkVectorFieldGlyph::GetRenderedOrientationArrayName()
{
  if (this->OrientationArrayName.empty())
  {
    return "";
  }
  if (this->ProjectToPlane && this->GlyphPlane)
  {
    return this->OrientationArrayName + ProjectedArrayNameSuffix;
  }
  return this->OrientationArrayName;
}

//----------------------------------------------------------------------------
std::string vtkVectorFieldGlyph::GetRenderedScaleArrayName()
{
  // Glyphs that are scaled by the vectors that orient them are as long as the part of the
  // vectors that they show
  if (!this->ScaleArrayName.empty() && this->ScaleArrayName == this->OrientationArrayName)
  {
    return this->GetRenderedOrientationArrayName();
  }
  return this->ScaleArrayName;
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::GetRenderedScale(std::string& scaleArrayName, int& scaleMode, double& scaleFactor)
{
  if (this->DirectionalScale && !this->Flat && !this->ScaleArrayName.empty())
  {
    // Stretched along the glyph's own axis only, by the per-axis scale that the glyph points
    // carry, which already includes the scale factor
    scaleArrayName = DirectionalScaleArrayName;
    scaleMode = ScaleByComponents;
    scaleFactor = 1.0;
    return;
  }
  scaleFactor = this->ScaleFactor;
  if (this->ScaleArrayName.empty() || this->ScaleMode == ScaleNone)
  {
    scaleArrayName.clear();
    scaleMode = ScaleNone;
    return;
  }
  scaleArrayName = this->GetRenderedScaleArrayName();
  scaleMode = ((this->ScaleMode == ScaleByComponents && !this->Flat) ? ScaleByComponents : ScaleByMagnitude);
}

//----------------------------------------------------------------------------
bool vtkVectorFieldGlyph::GetGlyphPlaneNormal(double normal[3])
{
  if (!this->GlyphPlane)
  {
    return false;
  }
  double xAxis[3] = { this->GlyphPlane->GetElement(0, 0), this->GlyphPlane->GetElement(1, 0), this->GlyphPlane->GetElement(2, 0) };
  double yAxis[3] = { this->GlyphPlane->GetElement(0, 1), this->GlyphPlane->GetElement(1, 1), this->GlyphPlane->GetElement(2, 1) };
  vtkMath::Cross(xAxis, yAxis, normal);
  return (vtkMath::Normalize(normal) > 0.0);
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::ConfigureMapper(vtkGlyph3DMapper* mapper)
{
  if (!mapper)
  {
    return;
  }
  mapper->SetSourceConnection(this->GetOutputPort(GlyphShapePort));

  std::string orientationArrayName = this->GetRenderedOrientationArrayName();
  if (!orientationArrayName.empty())
  {
    mapper->OrientOn();
    mapper->SetOrientationModeToDirection();
    mapper->SetOrientationArray(orientationArrayName.c_str());
  }
  else
  {
    mapper->OrientOff();
  }

  std::string scaleArrayName;
  int scaleMode = ScaleNone;
  double scaleFactor = 1.0;
  this->GetRenderedScale(scaleArrayName, scaleMode, scaleFactor);
  mapper->ScalingOn();
  mapper->SetScaleFactor(scaleFactor);
  if (scaleMode == ScaleNone)
  {
    mapper->SetScaleModeToNoDataScaling();
    return;
  }
  mapper->SetScaleArray(scaleArrayName.c_str());
  mapper->SetScaleMode(scaleMode == ScaleByComponents ? vtkGlyph3DMapper::SCALE_BY_COMPONENTS : vtkGlyph3DMapper::SCALE_BY_MAGNITUDE);
}

//----------------------------------------------------------------------------
int vtkVectorFieldGlyph::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  // The shape can be had without the points
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int vtkVectorFieldGlyph::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkDataSet* input = vtkDataSet::GetData(inputVector[0]);
  vtkPolyData* glyphPoints = vtkPolyData::GetData(outputVector, GlyphPointsPort);
  vtkPolyData* shape = vtkPolyData::GetData(outputVector, GlyphShapePort);
  vtkPolyData* geometry = vtkPolyData::GetData(outputVector, GlyphGeometryPort);
  if (!glyphPoints || !shape || !geometry)
  {
    return 0;
  }
  glyphPoints->Initialize();
  shape->Initialize();
  geometry->Initialize();

  this->BuildGlyphShape(shape);
  if (!input || input->GetNumberOfPoints() == 0)
  {
    return 1;
  }
  this->SelectGlyphPoints(input, glyphPoints);
  this->AddGlyphArrays(glyphPoints);
  if (this->GenerateGeometry)
  {
    this->BuildGlyphGeometry(glyphPoints, shape, geometry);
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::SelectGlyphPoints(vtkDataSet* input, vtkPolyData* glyphPoints)
{
  if (this->SubsetMode == SubsetRandomSurface || this->SubsetMode == SubsetRandomVolume)
  {
    // The subset is spread over the input cells, which thresholding drops, so it is taken first. Volumetric meshes
    // typically have only 3D cells, so a surface subset needs the boundary surface extracted.
    vtkSmartPointer<vtkDataSet> cells = input;
    if (this->SubsetMode == SubsetRandomSurface)
    {
      this->SurfaceFilter->SetInputData(input);
      this->SurfaceFilter->Update();
      vtkSmartPointer<vtkPolyData> surface = vtkSmartPointer<vtkPolyData>::New();
      surface->ShallowCopy(this->SurfaceFilter->GetOutput());
      this->SurfaceFilter->SetInputData(nullptr);
      cells = surface;
    }
    vtkNew<vtkPolyData> subset;
    this->TakeRandomSubset(cells, this->SubsetMode, subset);
    this->ExtractShownPoints(subset, glyphPoints);
    return;
  }
  // Otherwise the thresholds go first, so that the subset is taken from the points that are
  // shown
  if (this->SubsetMode == SubsetSpaced || this->SubsetMode == SubsetSpacedSurface)
  {
    vtkSmartPointer<vtkDataSet> points = input;
    if (this->SubsetMode == SubsetSpacedSurface)
    {
      this->SurfaceFilter->SetInputData(input);
      this->SurfaceFilter->Update();
      // Points without cells have no surface, they are all there is
      if (this->SurfaceFilter->GetOutput()->GetNumberOfPoints() > 0)
      {
        vtkSmartPointer<vtkPolyData> surface = vtkSmartPointer<vtkPolyData>::New();
        surface->ShallowCopy(this->SurfaceFilter->GetOutput());
        points = surface;
      }
      this->SurfaceFilter->SetInputData(nullptr);
    }
    vtkNew<vtkPolyData> shownPoints;
    this->ExtractShownPoints(points, shownPoints);
    this->TakeSpacedSubset(shownPoints, glyphPoints);
    return;
  }
  if (this->SubsetMode == SubsetRandomBounds)
  {
    vtkNew<vtkPolyData> shownPoints;
    this->ExtractShownPoints(input, shownPoints);
    this->TakeRandomSubset(shownPoints, SubsetRandomBounds, glyphPoints);
    return;
  }
  this->ExtractShownPoints(input, glyphPoints);
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::ExtractShownPoints(vtkDataSet* dataSet, vtkPolyData* shownPoints)
{
  vtkPointData* pointData = dataSet->GetPointData();
  vtkDataArray* thresholdArray = nullptr;
  if (this->ThresholdEnabled && !this->ThresholdArrayName.empty())
  {
    thresholdArray = pointData->GetArray(this->ThresholdArrayName.c_str());
  }
  vtkDataArray* regionArray = (this->RegionArrayName.empty() ? nullptr : pointData->GetArray(this->RegionArrayName.c_str()));

  vtkIdType numberOfPoints = dataSet->GetNumberOfPoints();
  vtkPointSet* pointSet = vtkPointSet::SafeDownCast(dataSet);
  if (!thresholdArray && !regionArray && pointSet && pointSet->GetPoints())
  {
    // Every point
    shownPoints->SetPoints(pointSet->GetPoints());
    shownPoints->GetPointData()->PassData(pointData);
    return;
  }

  vtkNew<vtkIdList> shownPointIds;
  shownPointIds->Allocate(numberOfPoints);
  for (vtkIdType pointId = 0; pointId < numberOfPoints; ++pointId)
  {
    if (thresholdArray)
    {
      double value = vtkVectorFieldSampler::GetComponentOrMagnitude(thresholdArray, pointId, this->ThresholdComponent);
      if (value < this->ThresholdRange[0] || value > this->ThresholdRange[1])
      {
        continue;
      }
    }
    if (regionArray && regionArray->GetComponent(pointId, 0) < 0.5)
    {
      continue;
    }
    shownPointIds->InsertNextId(pointId);
  }

  vtkIdType numberOfShownPoints = shownPointIds->GetNumberOfIds();
  vtkNew<vtkPoints> points;
  if (pointSet && pointSet->GetPoints())
  {
    points->SetDataType(pointSet->GetPoints()->GetDataType());
  }
  points->SetNumberOfPoints(numberOfShownPoints);
  vtkPointData* shownPointData = shownPoints->GetPointData();
  shownPointData->CopyAllocate(pointData, numberOfShownPoints);
  for (vtkIdType shownIndex = 0; shownIndex < numberOfShownPoints; ++shownIndex)
  {
    vtkIdType pointId = shownPointIds->GetId(shownIndex);
    points->SetPoint(shownIndex, dataSet->GetPoint(pointId));
    shownPointData->CopyData(pointData, pointId, shownIndex);
  }
  shownPoints->SetPoints(points);
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::TakeRandomSubset(vtkDataSet* dataSet, int spread, vtkPolyData* subset)
{
  if (dataSet->GetNumberOfPoints() == 0)
  {
    return;
  }
  // A random subset of the points, spread evenly over their bounding box, the surface, or
  // the volume. The generator is seeded on every run, so the same points give the same subset.
  this->MaskPoints->SetRandomModeType(spread == SubsetRandomSurface  ? vtkMaskPoints::UNIFORM_SPATIAL_SURFACE
                                      : spread == SubsetRandomVolume ? vtkMaskPoints::UNIFORM_SPATIAL_VOLUME
                                                                     : vtkMaskPoints::UNIFORM_SPATIAL_BOUNDS);
  this->MaskPoints->SetMaximumNumberOfPoints(this->MaximumNumberOfPoints);
  this->MaskPoints->SetRandomSeed(this->RandomSeed);
  this->MaskPoints->SetInputData(dataSet);
  this->MaskPoints->Update();
  subset->ShallowCopy(this->MaskPoints->GetOutput());
  this->MaskPoints->SetInputData(nullptr);
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::TakeSpacedSubset(vtkPolyData* points, vtkPolyData* subset)
{
  vtkNew<vtkIdList> pointIds;
  vtkVectorFieldSampler::GetSpacedPointIds(points->GetPoints(), this->SubsetSpacing, pointIds);
  vtkIdType numberOfPoints = pointIds->GetNumberOfIds();
  vtkNew<vtkPoints> subsetPoints;
  if (points->GetPoints())
  {
    subsetPoints->SetDataType(points->GetPoints()->GetDataType());
  }
  subsetPoints->SetNumberOfPoints(numberOfPoints);
  subset->GetPointData()->CopyAllocate(points->GetPointData(), numberOfPoints);
  for (vtkIdType index = 0; index < numberOfPoints; ++index)
  {
    subsetPoints->SetPoint(index, points->GetPoint(pointIds->GetId(index)));
    subset->GetPointData()->CopyData(points->GetPointData(), pointIds->GetId(index), index);
  }
  subset->SetPoints(subsetPoints);
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::AddGlyphArrays(vtkPolyData* glyphPoints)
{
  vtkPointData* pointData = glyphPoints->GetPointData();
  vtkIdType numberOfPoints = glyphPoints->GetNumberOfPoints();

  // The mapper scales per axis only from an array, so store (length * ScaleFactor, 1, 1). The length comes from the
  // scale array, which is not always the orientation array.
  vtkDataArray* scaleArray = (this->ScaleArrayName.empty() ? nullptr : pointData->GetArray(this->ScaleArrayName.c_str()));
  if (this->DirectionalScale && !this->Flat && scaleArray)
  {
    vtkNew<vtkDoubleArray> directionalScale;
    directionalScale->SetName(DirectionalScaleArrayName);
    directionalScale->SetNumberOfComponents(3);
    directionalScale->SetNumberOfTuples(numberOfPoints);
    for (vtkIdType pointId = 0; pointId < numberOfPoints; ++pointId)
    {
      double length = vtkVectorFieldSampler::GetComponentOrMagnitude(scaleArray, pointId, scaleArray->GetNumberOfComponents() == 1 ? 0 : -1);
      directionalScale->SetTuple3(pointId, this->ScaleFactor * length, 1.0, 1.0);
    }
    pointData->AddArray(directionalScale);
  }

  // Projected vectors go into an array of their own, so the glyphs are still colored by the true magnitude.
  double normal[3] = { 0.0, 0.0, 0.0 };
  vtkDataArray* orientationArray = (this->OrientationArrayName.empty() ? nullptr : pointData->GetArray(this->OrientationArrayName.c_str()));
  if (this->ProjectToPlane && orientationArray && orientationArray->GetNumberOfComponents() == 3 && this->GetGlyphPlaneNormal(normal))
  {
    vtkSmartPointer<vtkDataArray> projected = vtkSmartPointer<vtkDataArray>::Take(orientationArray->NewInstance());
    projected->SetName((this->OrientationArrayName + ProjectedArrayNameSuffix).c_str());
    projected->SetNumberOfComponents(3);
    projected->SetNumberOfTuples(numberOfPoints);
    for (vtkIdType pointId = 0; pointId < numberOfPoints; ++pointId)
    {
      double vector[3] = { 0.0, 0.0, 0.0 };
      orientationArray->GetTuple(pointId, vector);
      double outOfPlaneComponent = vtkMath::Dot(vector, normal);
      for (int axis = 0; axis < 3; ++axis)
      {
        vector[axis] -= outOfPlaneComponent * normal[axis];
      }
      projected->SetTuple(pointId, vector);
    }
    pointData->AddArray(projected);
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::BuildGlyphShape(vtkPolyData* shape)
{
  vtkAlgorithm* source = nullptr;
  if (this->Flat)
  {
    // Outlines, which stay crisp at any zoom level and do not hide the image underneath
    this->FlatSource->SetResolution(this->Resolution);
    this->FlatSource->SetTipLength(this->TipLength);
    bool directional = false;
    switch (this->GlyphType)
    {
      case GlyphCone:
        this->FlatSource->SetGlyphTypeToEdgeArrow();
        directional = true;
        break;
      case GlyphBox: this->FlatSource->SetGlyphTypeToSquare(); break;
      case GlyphCylinder:
      case GlyphLine: this->FlatSource->SetGlyphTypeToDash(); break;
      case GlyphSphere: this->FlatSource->SetGlyphTypeToCircle(); break;
      case GlyphArrow:
      default:
        this->FlatSource->SetGlyphTypeToArrow();
        directional = true;
        break;
    }
    // vtkGlyphSource2D centers the glyph on the point; directional glyphs are more readable
    // when they start at the point and grow along the vector
    vtkNew<vtkTransform> shift;
    if (directional)
    {
      shift->Translate(0.5, 0.0, 0.0);
    }
    this->FlatSourceShift->SetTransform(shift);
    source = this->FlatSourceShift;
  }
  else
  {
    switch (this->GlyphType)
    {
      case GlyphCone:
        this->ConeSource->SetResolution(this->Resolution);
        this->ConeSource->SetRadius(this->TipRadius);
        source = this->ConeSource;
        break;
      case GlyphBox: source = this->CubeSource; break;
      case GlyphCylinder:
        this->CylinderSource->SetResolution(this->Resolution);
        source = this->CylinderRotation;
        break;
      case GlyphLine: source = this->LineSource; break;
      case GlyphSphere:
        this->SphereSource->SetThetaResolution(this->Resolution);
        this->SphereSource->SetPhiResolution(this->Resolution);
        source = this->SphereSource;
        break;
      case GlyphArrow:
      default:
        this->ArrowSource->SetTipResolution(this->Resolution);
        this->ArrowSource->SetShaftResolution(this->Resolution);
        this->ArrowSource->SetTipLength(this->TipLength);
        this->ArrowSource->SetTipRadius(this->TipRadius);
        this->ArrowSource->SetShaftRadius(this->ShaftRadius);
        source = this->ArrowSource;
        break;
    }
  }
  // The sources are kept, so a shape is only generated again when its settings change
  source->Update();
  shape->ShallowCopy(source->GetOutputDataObject(0));
}

//----------------------------------------------------------------------------
void vtkVectorFieldGlyph::BuildGlyphGeometry(vtkPolyData* glyphPoints, vtkPolyData* shape, vtkPolyData* geometry)
{
  vtkIdType numberOfGlyphs = glyphPoints->GetNumberOfPoints();
  vtkPoints* shapePoints = shape->GetPoints();
  if (numberOfGlyphs == 0 || !shapePoints || shapePoints->GetNumberOfPoints() == 0)
  {
    return;
  }
  vtkPointData* glyphPointData = glyphPoints->GetPointData();
  std::string orientationArrayName = this->GetRenderedOrientationArrayName();
  vtkDataArray* orientationArray = (orientationArrayName.empty() ? nullptr : glyphPointData->GetArray(orientationArrayName.c_str()));
  if (orientationArray && orientationArray->GetNumberOfComponents() != 3)
  {
    orientationArray = nullptr;
  }
  // The same scale as ConfigureMapper() gives the mapper
  std::string scaleArrayName;
  int scaleMode = ScaleNone;
  double scaleFactor = 1.0;
  this->GetRenderedScale(scaleArrayName, scaleMode, scaleFactor);
  vtkDataArray* scaleArray = (scaleMode == ScaleNone ? nullptr : glyphPointData->GetArray(scaleArrayName.c_str()));
  if (scaleMode == ScaleByComponents && scaleArray && scaleArray->GetNumberOfComponents() != 3)
  {
    // As the mapper does, which reports it
    scaleMode = ScaleNone;
  }

  // Flat glyphs are placed in the coordinates of the glyph plane, where the shape lies in the
  // XY plane and stays there when it is rotated about the plane normal, and then moved back
  vtkNew<vtkMatrix4x4> planeToInput;
  vtkNew<vtkMatrix4x4> inputToPlane;
  bool inPlane = (this->Flat && this->GlyphPlane);
  if (inPlane)
  {
    // An orthonormal frame, so that the glyphs keep their size in the coordinates of the input
    double axes[3][3];
    for (int axis = 0; axis < 3; ++axis)
    {
      for (int row = 0; row < 3; ++row)
      {
        axes[axis][row] = this->GlyphPlane->GetElement(row, axis);
      }
    }
    vtkMath::Cross(axes[0], axes[1], axes[2]);
    vtkMath::Cross(axes[2], axes[0], axes[1]);
    for (int axis = 0; axis < 3; ++axis)
    {
      vtkMath::Normalize(axes[axis]);
      for (int row = 0; row < 3; ++row)
      {
        planeToInput->SetElement(row, axis, axes[axis][row]);
      }
      planeToInput->SetElement(axis, 3, this->GlyphPlane->GetElement(axis, 3));
    }
    vtkMatrix4x4::Invert(planeToInput, inputToPlane);
  }

  vtkIdType numberOfShapePoints = shapePoints->GetNumberOfPoints();
  vtkNew<vtkPoints> points;
  points->SetDataType(VTK_DOUBLE);
  points->SetNumberOfPoints(numberOfGlyphs * numberOfShapePoints);
  vtkPointData* geometryPointData = geometry->GetPointData();
  geometryPointData->CopyAllocate(glyphPointData, numberOfGlyphs * numberOfShapePoints);
  vtkNew<vtkCellArray> verts;
  vtkNew<vtkCellArray> lines;
  vtkNew<vtkCellArray> polys;
  vtkNew<vtkCellArray> strips;
  vtkCellArray* shapeCells[4] = { shape->GetVerts(), shape->GetLines(), shape->GetPolys(), shape->GetStrips() };
  vtkCellArray* geometryCells[4] = { verts, lines, polys, strips };
  std::vector<vtkIdType> cellPointIds;

  for (vtkIdType glyphIndex = 0; glyphIndex < numberOfGlyphs; ++glyphIndex)
  {
    // Scale, as vtkGlyph3DMapper computes it
    double scale[3] = { 1.0, 1.0, 1.0 };
    if (scaleArray && scaleMode == ScaleByComponents)
    {
      scaleArray->GetTuple(glyphIndex, scale);
    }
    else if (scaleArray && scaleMode == ScaleByMagnitude)
    {
      scale[0] = scale[1] = scale[2] = vtkVectorFieldSampler::GetComponentOrMagnitude(scaleArray, glyphIndex, -1);
    }
    for (int axis = 0; axis < 3; ++axis)
    {
      scale[axis] *= scaleFactor;
      if (scale[axis] == 0.0)
      {
        scale[axis] = 1.0e-10;
      }
    }

    // Position and direction, in the plane of flat glyphs
    double position[4] = { 0.0, 0.0, 0.0, 1.0 };
    glyphPoints->GetPoint(glyphIndex, position);
    double direction[4] = { 1.0, 0.0, 0.0, 0.0 };
    if (orientationArray)
    {
      orientationArray->GetTuple(glyphIndex, direction);
      direction[3] = 0.0;
    }
    if (inPlane)
    {
      inputToPlane->MultiplyPoint(position, position);
      inputToPlane->MultiplyPoint(direction, direction);
    }
    double rotation[3][3];
    if (orientationArray)
    {
      GetDirectionRotation(direction, rotation);
    }
    else
    {
      vtkMath::Identity3x3(rotation);
    }

    vtkIdType firstPointId = glyphIndex * numberOfShapePoints;
    for (vtkIdType shapePointIndex = 0; shapePointIndex < numberOfShapePoints; ++shapePointIndex)
    {
      double shapePoint[3] = { 0.0, 0.0, 0.0 };
      shapePoints->GetPoint(shapePointIndex, shapePoint);
      double scaled[3] = { shapePoint[0] * scale[0], shapePoint[1] * scale[1], shapePoint[2] * scale[2] };
      double placed[4] = { 0.0, 0.0, 0.0, 1.0 };
      vtkMath::Multiply3x3(rotation, scaled, placed);
      for (int axis = 0; axis < 3; ++axis)
      {
        placed[axis] += position[axis];
      }
      if (inPlane)
      {
        planeToInput->MultiplyPoint(placed, placed);
      }
      points->SetPoint(firstPointId + shapePointIndex, placed);
      geometryPointData->CopyData(glyphPointData, glyphIndex, firstPointId + shapePointIndex);
    }

    // The cells of the shape, on the points of this glyph
    for (int cellType = 0; cellType < 4; ++cellType)
    {
      vtkCellArray* cells = shapeCells[cellType];
      if (!cells || cells->GetNumberOfCells() == 0)
      {
        continue;
      }
      vtkIdType numberOfCellPoints = 0;
      const vtkIdType* shapeCellPointIds = nullptr;
      for (cells->InitTraversal(); cells->GetNextCell(numberOfCellPoints, shapeCellPointIds);)
      {
        cellPointIds.resize(numberOfCellPoints);
        for (vtkIdType cellPointIndex = 0; cellPointIndex < numberOfCellPoints; ++cellPointIndex)
        {
          cellPointIds[cellPointIndex] = firstPointId + shapeCellPointIds[cellPointIndex];
        }
        geometryCells[cellType]->InsertNextCell(numberOfCellPoints, cellPointIds.data());
      }
    }
  }

  geometry->SetPoints(points);
  geometry->SetVerts(verts);
  geometry->SetLines(lines);
  geometry->SetPolys(polys);
  geometry->SetStrips(strips);
}
