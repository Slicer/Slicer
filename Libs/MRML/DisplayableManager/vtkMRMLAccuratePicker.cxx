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

// VTK includes
#include <vtkAbstractCellLocator.h>
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropCollection.h>
#include <vtkRenderer.h>
#include <vtkStaticCellLocator.h>

// STD includes
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
  // Vertices and lines cannot be hit without the pick tolerance, so there is no
  // point in looking for a cell that the ray hits if there are only those.
  vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataSet);
  const bool onlyVerticesOrLines = polyData && polyData->GetNumberOfPolys() == 0 && polyData->GetNumberOfStrips() == 0;
  if (!onlyVerticesOrLines)
  {
    // Look for a cell that the ray hits. The tolerance is not zero so that a ray
    // that passes exactly through an edge or a vertex is not missed due to rounding.
    const double hitTolerance = tol * 1e-6;
    if (this->Superclass::IntersectDataSetWithLine(dataSet, p1, p2, t1, t2, hitTolerance, locator, cellId, subId, tMin, pDistMin, xyz, minPCoords))
    {
      return true;
    }
  }
  return this->Superclass::IntersectDataSetWithLine(dataSet, p1, p2, t1, t2, tol, locator, cellId, subId, tMin, pDistMin, xyz, minPCoords);
}
