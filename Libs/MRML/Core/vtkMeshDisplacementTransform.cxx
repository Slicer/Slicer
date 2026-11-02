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
#include "vtkMeshDisplacementTransform.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkGenericCell.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkStaticCellLocator.h>
#include <vtkStaticPointLocator.h>

// STD includes
#include <algorithm>
#include <cmath>
#include <vector>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMeshDisplacementTransform);
vtkCxxSetObjectMacro(vtkMeshDisplacementTransform, InputPointSet, vtkPointSet);

//----------------------------------------------------------------------------
vtkMeshDisplacementTransform::vtkMeshDisplacementTransform() = default;

//----------------------------------------------------------------------------
vtkMeshDisplacementTransform::~vtkMeshDisplacementTransform()
{
  this->SetInputPointSet(nullptr);
  this->SetDisplacementArrayName(nullptr);
}

//----------------------------------------------------------------------------
void vtkMeshDisplacementTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "InputPointSet: " << this->InputPointSet << "\n";
  os << indent << "DisplacementArrayName: " << (this->DisplacementArrayName ? this->DisplacementArrayName : "(active vectors)") << "\n";
  os << indent << "DisplacementScale: " << this->DisplacementScale << "\n";
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMeshDisplacementTransform::MakeTransform()
{
  return vtkMeshDisplacementTransform::New();
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMeshDisplacementTransform::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  if (this->InputPointSet)
  {
    mTime = std::max(mTime, this->InputPointSet->GetMTime());
  }
  return mTime;
}

//----------------------------------------------------------------------------
void vtkMeshDisplacementTransform::InternalDeepCopy(vtkAbstractTransform* transform)
{
  this->Superclass::InternalDeepCopy(transform);
  vtkMeshDisplacementTransform* meshTransform = static_cast<vtkMeshDisplacementTransform*>(transform);
  // The mesh is never modified by this class, so it is shared
  this->SetInputPointSet(meshTransform->GetInputPointSet());
  this->SetDisplacementArrayName(meshTransform->GetDisplacementArrayName());
  this->SetDisplacementScale(meshTransform->GetDisplacementScale());
}

//----------------------------------------------------------------------------
void vtkMeshDisplacementTransform::InternalUpdate()
{
  this->Displacements = nullptr;
  this->PointLocator = nullptr;
  this->CellLocator = nullptr;
  vtkPointSet* mesh = this->InputPointSet;
  if (!mesh || mesh->GetNumberOfPoints() == 0)
  {
    return;
  }
  vtkDataArray* displacements = (this->DisplacementArrayName && this->DisplacementArrayName[0] != '\0' ? mesh->GetPointData()->GetArray(this->DisplacementArrayName) //
                                                                                                       : mesh->GetPointData()->GetVectors());
  if (!displacements || displacements->GetNumberOfComponents() != 3)
  {
    return;
  }
  this->Displacements = displacements;

  // The diagonal of the bounding box of the mesh
  double diagonal = mesh->GetLength();
  if (diagonal <= 0.0)
  {
    diagonal = 1.0;
  }
  // A mesh point that is transformed has exactly the coordinates of the point; the tolerance
  // only absorbs rounding
  this->PointTolerance2 = (1e-9 * diagonal) * (1e-9 * diagonal);
  this->CellTolerance2 = (1e-6 * diagonal) * (1e-6 * diagonal);
  this->DerivativeStep = 1e-4 * diagonal;

  this->PointLocator = vtkSmartPointer<vtkStaticPointLocator>::New();
  this->PointLocator->SetDataSet(mesh);
  this->PointLocator->BuildLocator();
  if (mesh->GetNumberOfCells() > 0)
  {
    this->MaximumCellSize = mesh->GetMaxCellSize();
    this->CellLocator = vtkSmartPointer<vtkStaticCellLocator>::New();
    this->CellLocator->SetDataSet(mesh);
    this->CellLocator->BuildLocator();
  }
}

//----------------------------------------------------------------------------
bool vtkMeshDisplacementTransform::GetDisplacement(const double position[3], double displacement[3])
{
  displacement[0] = displacement[1] = displacement[2] = 0.0;
  vtkDataArray* displacements = this->Displacements;
  vtkPointSet* mesh = this->InputPointSet;
  if (!displacements || !mesh || !this->PointLocator)
  {
    return false;
  }

  // A point of the mesh: its own vector, as a warp would use it
  double distance2 = 0.0;
  vtkIdType pointId = this->PointLocator->FindClosestPointWithinRadius(std::sqrt(this->PointTolerance2), position, distance2);
  if (pointId >= 0)
  {
    displacements->GetTuple(pointId, displacement);
  }
  else
  {
    // Anywhere else: interpolated in the cell that contains the position. Transforms may be
    // evaluated from several threads at once, so the scratch space belongs to this call.
    if (!this->CellLocator)
    {
      return false;
    }
    vtkNew<vtkGenericCell> cell;
    std::vector<double> weights(static_cast<size_t>(std::max(1, this->MaximumCellSize)));
    int subId = 0;
    double pcoords[3] = { 0.0, 0.0, 0.0 };
    double x[3] = { position[0], position[1], position[2] };
    vtkIdType cellId = this->CellLocator->FindCell(x, this->CellTolerance2, cell, subId, pcoords, weights.data());
    if (cellId < 0)
    {
      return false;
    }
    vtkIdList* cellPointIds = cell->GetPointIds();
    for (vtkIdType cellPointIndex = 0; cellPointIndex < cellPointIds->GetNumberOfIds(); ++cellPointIndex)
    {
      double vector[3] = { 0.0, 0.0, 0.0 };
      displacements->GetTuple(cellPointIds->GetId(cellPointIndex), vector);
      for (int axis = 0; axis < 3; ++axis)
      {
        displacement[axis] += weights[cellPointIndex] * vector[axis];
      }
    }
  }
  for (int axis = 0; axis < 3; ++axis)
  {
    displacement[axis] *= this->DisplacementScale;
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkMeshDisplacementTransform::ForwardTransformPoint(const double in[3], double out[3])
{
  double displacement[3] = { 0.0, 0.0, 0.0 };
  this->GetDisplacement(in, displacement);
  out[0] = in[0] + displacement[0];
  out[1] = in[1] + displacement[1];
  out[2] = in[2] + displacement[2];
}

//----------------------------------------------------------------------------
void vtkMeshDisplacementTransform::ForwardTransformPoint(const float in[3], float out[3])
{
  double inDouble[3] = { in[0], in[1], in[2] };
  double outDouble[3] = { 0.0, 0.0, 0.0 };
  this->ForwardTransformPoint(inDouble, outDouble);
  out[0] = static_cast<float>(outDouble[0]);
  out[1] = static_cast<float>(outDouble[1]);
  out[2] = static_cast<float>(outDouble[2]);
}

//----------------------------------------------------------------------------
void vtkMeshDisplacementTransform::ForwardTransformDerivative(const double in[3], double out[3], double derivative[3][3])
{
  this->ForwardTransformPoint(in, out);
  // Central differences of the displacement; the field is only piecewise smooth, so this is
  // what the iterative inverse can use everywhere, also across cell boundaries
  double step = this->DerivativeStep;
  for (int column = 0; column < 3; ++column)
  {
    double before[3] = { in[0], in[1], in[2] };
    double after[3] = { in[0], in[1], in[2] };
    before[column] -= step;
    after[column] += step;
    double displacementBefore[3] = { 0.0, 0.0, 0.0 };
    double displacementAfter[3] = { 0.0, 0.0, 0.0 };
    this->GetDisplacement(before, displacementBefore);
    this->GetDisplacement(after, displacementAfter);
    for (int row = 0; row < 3; ++row)
    {
      derivative[row][column] = (row == column ? 1.0 : 0.0) + (displacementAfter[row] - displacementBefore[row]) / (2.0 * step);
    }
  }
}

//----------------------------------------------------------------------------
void vtkMeshDisplacementTransform::ForwardTransformDerivative(const float in[3], float out[3], float derivative[3][3])
{
  double inDouble[3] = { in[0], in[1], in[2] };
  double outDouble[3] = { 0.0, 0.0, 0.0 };
  double derivativeDouble[3][3];
  this->ForwardTransformDerivative(inDouble, outDouble, derivativeDouble);
  for (int row = 0; row < 3; ++row)
  {
    out[row] = static_cast<float>(outDouble[row]);
    for (int column = 0; column < 3; ++column)
    {
      derivative[row][column] = static_cast<float>(derivativeDouble[row][column]);
    }
  }
}
