/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkMRMLAccuratePicker.h"
#include "vtkMRMLRayCastMapper.h"

// VTK includes
#include <vtkAbstractCellLocator.h>
#include <vtkActor.h>
#include <vtkCellTypes.h>
#include <vtkDataSet.h>
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropCollection.h>
#include <vtkRenderer.h>
#include <vtkStaticCellLocator.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

// STD includes
#include <algorithm>
#include <set>

vtkStandardNewMacro(vtkMRMLAccuratePicker);

//----------------------------------------------------------------------------
vtkMRMLAccuratePicker::vtkMRMLAccuratePicker() = default;

//----------------------------------------------------------------------------
vtkMRMLAccuratePicker::~vtkMRMLAccuratePicker() = default;

//----------------------------------------------------------------------------
void vtkMRMLAccuratePicker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MinimumCellCountToIndex: " << this->MinimumCellCountToIndex << "\n";
  os << indent << "Cached locators: " << this->LocatorsBySurface.size() << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLAccuratePicker::UpdateLocators(vtkRenderer* renderer)
{
  this->RemoveAllLocators();
  if (!renderer)
  {
    this->LocatorsBySurface.clear();
    return;
  }

  std::set<vtkPolyData*> shownSurfaces;
  vtkPropCollection* props = renderer->GetViewProps();
  vtkCollectionSimpleIterator propIterator;
  props->InitTraversal(propIterator);
  for (vtkProp* prop = props->GetNextProp(propIterator); prop != nullptr; prop = props->GetNextProp(propIterator))
  {
    if (!prop->GetPickable() || !prop->GetVisibility())
    {
      continue;
    }
    vtkActor* actor = vtkActor::SafeDownCast(prop);
    if (!actor)
    {
      continue;
    }
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    if (!mapper)
    {
      continue;
    }
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(mapper->GetInput());
    if (!polyData || polyData->GetNumberOfCells() < this->MinimumCellCountToIndex)
    {
      continue;
    }

    shownSurfaces.insert(polyData);
    CachedLocator& cached = this->LocatorsBySurface[polyData];
    if (!cached.Locator)
    {
      vtkNew<vtkStaticCellLocator> locator;
      locator->SetDataSet(polyData);
      cached.Locator = locator;
      cached.BuildMTime = 0;
    }
    // Build once, and rebuild only when the surface itself changes, so repeated
    // picks over an unchanging surface pay the build cost at most once.
    if (cached.BuildMTime != polyData->GetMTime())
    {
      cached.Locator->BuildLocator();
      cached.BuildMTime = polyData->GetMTime();
    }
    this->AddLocator(cached.Locator);
  }

  // Release locators for surfaces that are no longer shown (a cached locator
  // holds a reference to its poly data).
  for (auto it = this->LocatorsBySurface.begin(); it != this->LocatorsBySurface.end();)
  {
    if (shownSurfaces.find(it->first) == shownSurfaces.end())
    {
      it = this->LocatorsBySurface.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

//----------------------------------------------------------------------------
int vtkMRMLAccuratePicker::Pick(double selectionX, double selectionY, double selectionZ, vtkRenderer* renderer)
{
  this->UpdateLocators(renderer);
  return this->Superclass::Pick(selectionX, selectionY, selectionZ, renderer);
}

//----------------------------------------------------------------------------
double vtkMRMLAccuratePicker::IntersectActorWithLine(const double p1[3], const double p2[3], double t1, double t2, double tol, vtkProp3D* prop, vtkMapper* m)
{
  vtkMapper* mapper = m;
  vtkMRMLRayCastMapper* rayCastMapper = vtkMRMLRayCastMapper::SafeDownCast(mapper);
  if (!rayCastMapper)
  {
    return this->Superclass::IntersectActorWithLine(p1, p2, t1, t2, tol, prop, mapper);
  }
  double t = 0.0;
  double position[3] = { 0.0, 0.0, 0.0 };
  double normal[3] = { 0.0, 0.0, 1.0 };
  if (!rayCastMapper->IntersectWithRay(p1, p2, t1, t2, t, position, normal) || t < t1 || t > t2)
  {
    return VTK_DOUBLE_MAX;
  }
  if (t < this->GlobalTMin)
  {
    // Same outputs as for cells, except there is no cell
    this->Mapper = mapper;
    this->DataSet = nullptr;
    this->CellId = -1;
    this->SubId = -1;
    this->PointId = -1;
    std::copy_n(position, 3, this->MapperPosition);
    std::copy_n(normal, 3, this->MapperNormal);
  }
  return t;
}

//----------------------------------------------------------------------------
namespace
{
/// Output of vtkCellPicker::IntersectDataSetWithLine(), so that searches with
/// different tolerances can be compared.
struct CellIntersection
{
  vtkAbstractCellLocator* Locator{ nullptr };
  vtkIdType CellId{ -1 };
  int SubId{ -1 };
  double T{ VTK_DOUBLE_MAX };
  double PDist{ VTK_DOUBLE_MAX };
  double XYZ[3]{ 0.0, 0.0, 0.0 };
  double PCoords[3]{ 0.0, 0.0, 0.0 };

  CellIntersection(vtkIdType cellId, int subId, double t, double pDist, const double xyz[3], const double pcoords[3])
    : CellId(cellId)
    , SubId(subId)
    , T(t)
    , PDist(pDist)
  {
    std::copy_n(xyz, 3, this->XYZ);
    std::copy_n(pcoords, 3, this->PCoords);
  }

  void CopyTo(vtkAbstractCellLocator*& locator, vtkIdType& cellId, int& subId, double& t, double& pDist, double xyz[3], double pcoords[3]) const
  {
    locator = this->Locator;
    cellId = this->CellId;
    subId = this->SubId;
    t = this->T;
    pDist = this->PDist;
    std::copy_n(this->XYZ, 3, xyz);
    std::copy_n(this->PCoords, 3, pcoords);
  }
};

/// Find out whether the data set has cells that a ray can hit (surface or
/// volumetric cells), and cells that can only be picked within the pick
/// tolerance (vertices and lines).
void GetKindsOfCells(vtkDataSet* dataSet, bool& hasSurfaceOrVolumeCells, bool& hasVertexOrLineCells)
{
  hasSurfaceOrVolumeCells = false;
  hasVertexOrLineCells = false;
  if (vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataSet))
  {
    hasSurfaceOrVolumeCells = polyData->GetNumberOfPolys() > 0 || polyData->GetNumberOfStrips() > 0;
    hasVertexOrLineCells = polyData->GetNumberOfVerts() > 0 || polyData->GetNumberOfLines() > 0;
    return;
  }
  if (vtkUnstructuredGrid* grid = vtkUnstructuredGrid::SafeDownCast(dataSet))
  {
    // The grid caches its distinct cell types
    vtkUnsignedCharArray* cellTypes = grid->GetDistinctCellTypesArray();
    for (vtkIdType typeIndex = 0; cellTypes && typeIndex < cellTypes->GetNumberOfTuples(); ++typeIndex)
    {
      if (vtkCellTypes::GetDimension(cellTypes->GetValue(typeIndex)) < 2)
      {
        hasVertexOrLineCells = true;
      }
      else
      {
        hasSurfaceOrVolumeCells = true;
      }
    }
    return;
  }
  // Other data sets (images, structured grids) have cells of a single type
  if (dataSet->GetNumberOfCells() > 0)
  {
    const bool vertexOrLineCells = vtkCellTypes::GetDimension(static_cast<unsigned char>(dataSet->GetCellType(0))) < 2;
    hasVertexOrLineCells = vertexOrLineCells;
    hasSurfaceOrVolumeCells = !vertexOrLineCells;
  }
}
} // namespace

//----------------------------------------------------------------------------
bool vtkMRMLAccuratePicker::IntersectDataSetWithLine(vtkDataSet* dataSet,
                                                     const double p1[3],
                                                     const double p2[3],
                                                     double t1,
                                                     double t2,
                                                     double tol,
                                                     vtkAbstractCellLocator*& locator,
                                                     vtkIdType& cellId,
                                                     int& subId,
                                                     double& tMin,
                                                     double& pDistMin,
                                                     double xyz[3],
                                                     double minPCoords[3])
{
  bool hasSurfaceOrVolumeCells = false;
  bool hasVertexOrLineCells = false;
  GetKindsOfCells(dataSet, hasSurfaceOrVolumeCells, hasVertexOrLineCells);
  if (!hasSurfaceOrVolumeCells)
  {
    // Vertices and lines cannot be hit without the pick tolerance
    return this->Superclass::IntersectDataSetWithLine(dataSet, p1, p2, t1, t2, tol, locator, cellId, subId, tMin, pDistMin, xyz, minPCoords);
  }

  // Each search starts from the closest intersection found so far (in other data sets)
  const CellIntersection closestSoFar(cellId, subId, tMin, pDistMin, xyz, minPCoords);

  // Look for a surface or volumetric cell that the ray hits. The tolerance is not
  // zero so that a ray that passes exactly through an edge or a vertex is not
  // missed due to rounding.
  CellIntersection hit = closestSoFar;
  const bool foundHit = this->Superclass::IntersectDataSetWithLine(dataSet, p1, p2, t1, t2, tol * 1e-6, hit.Locator, hit.CellId, hit.SubId, hit.T, hit.PDist, hit.XYZ, hit.PCoords);
  if (foundHit && !hasVertexOrLineCells)
  {
    hit.CopyTo(locator, cellId, subId, tMin, pDistMin, xyz, minPCoords);
    return true;
  }

  // Look for a cell within the pick tolerance: a vertex or line, which may be in
  // front of the hit cell, or a surface cell if the ray misses the surface (for
  // example, when it passes just outside the silhouette of the surface).
  CellIntersection withinTolerance = closestSoFar;
  const bool foundWithinTolerance = this->Superclass::IntersectDataSetWithLine(dataSet,
                                                                               p1,
                                                                               p2,
                                                                               t1,
                                                                               t2,
                                                                               tol,
                                                                               withinTolerance.Locator,
                                                                               withinTolerance.CellId,
                                                                               withinTolerance.SubId,
                                                                               withinTolerance.T,
                                                                               withinTolerance.PDist,
                                                                               withinTolerance.XYZ,
                                                                               withinTolerance.PCoords);
  if (!foundHit && !foundWithinTolerance)
  {
    return false;
  }
  const bool foundVertexOrLineInFront =
    foundWithinTolerance && withinTolerance.T < hit.T && vtkCellTypes::GetDimension(static_cast<unsigned char>(dataSet->GetCellType(withinTolerance.CellId))) < 2;
  if (!foundHit || foundVertexOrLineInFront)
  {
    withinTolerance.CopyTo(locator, cellId, subId, tMin, pDistMin, xyz, minPCoords);
    return true;
  }
  hit.CopyTo(locator, cellId, subId, tMin, pDistMin, xyz, minPCoords);
  // With a locator, vtkCellPicker uses the cell that the last search left in
  // vtkCellPicker::Cell, which is not the hit cell. Without a locator, it gets the
  // picked cell from the data set.
  locator = nullptr;
  return true;
}
