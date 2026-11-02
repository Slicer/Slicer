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

#ifndef __vtkMeshDisplacementTransform_h
#define __vtkMeshDisplacementTransform_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWarpTransform.h>

class vtkDataArray;
class vtkPointSet;
class vtkStaticCellLocator;
class vtkStaticPointLocator;

/// \brief Transform that moves each position by the displacement (3-component point data) of a mesh, without fitting.
///
/// A mesh point moves by its own vector without a cell search, so transforming the mesh itself equals vtkWarpVector.
/// Other positions move by the vector interpolated in the containing cell (as vtkProbeFilter); outside cells (e.g. around
/// a surface mesh or point cloud) they do not move. The inverse is iterative (vtkWarpTransform): reliable if the field does not fold.
class VTK_MRML_EXPORT vtkMeshDisplacementTransform : public vtkWarpTransform
{
public:
  static vtkMeshDisplacementTransform* New();
  vtkTypeMacro(vtkMeshDisplacementTransform, vtkWarpTransform);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Make another transform of the same type.
  vtkAbstractTransform* MakeTransform() override;

  /// The modification time includes that of the input mesh, so that editing the mesh is noticed.
  vtkMTimeType GetMTime() override;

  ///@{
  /// The mesh whose points carry the displacements. The mesh is not modified.
  virtual void SetInputPointSet(vtkPointSet* pointSet);
  vtkGetObjectMacro(InputPointSet, vtkPointSet);
  ///@}

  ///@{
  /// Name of the 3-component point data array that holds the displacements. If it is not set,
  /// the active vectors are used.
  vtkSetStringMacro(DisplacementArrayName);
  vtkGetStringMacro(DisplacementArrayName);
  ///@}

  ///@{
  /// Multiplies the displacements. Default is 1.
  vtkSetMacro(DisplacementScale, double);
  vtkGetMacro(DisplacementScale, double);
  ///@}

  /// Displacement at a mesh point or interpolated in the containing cell, times DisplacementScale. Update() must have been
  /// called. Returns false, and a zero displacement, if the position is not at a point or in a cell.
  bool GetDisplacement(const double position[3], double displacement[3]);

protected:
  vtkMeshDisplacementTransform();
  ~vtkMeshDisplacementTransform() override;

  void InternalUpdate() override;
  void InternalDeepCopy(vtkAbstractTransform* transform) override;

  void ForwardTransformPoint(const float in[3], float out[3]) override;
  void ForwardTransformPoint(const double in[3], double out[3]) override;
  void ForwardTransformDerivative(const float in[3], float out[3], float derivative[3][3]) override;
  void ForwardTransformDerivative(const double in[3], double out[3], double derivative[3][3]) override;

  vtkPointSet* InputPointSet{ nullptr };
  char* DisplacementArrayName{ nullptr };
  double DisplacementScale{ 1.0 };

  // Set up by InternalUpdate()
  vtkSmartPointer<vtkDataArray> Displacements;
  vtkSmartPointer<vtkStaticPointLocator> PointLocator;
  vtkSmartPointer<vtkStaticCellLocator> CellLocator;
  /// Squared distance within which a position is the same as a mesh point
  double PointTolerance2{ 0.0 };
  /// Squared distance within which a position is inside a cell
  double CellTolerance2{ 0.0 };
  /// Step of the finite differences that the derivative is computed with
  double DerivativeStep{ 1.0 };
  /// Largest number of points in a cell of the mesh, for the interpolation weights
  int MaximumCellSize{ 0 };

private:
  vtkMeshDisplacementTransform(const vtkMeshDisplacementTransform&) = delete;
  void operator=(const vtkMeshDisplacementTransform&) = delete;
};

#endif
