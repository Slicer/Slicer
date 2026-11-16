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

#ifndef __vtkVectorFieldGrid_h
#define __vtkVectorFieldGrid_h

// MRML includes
#include "vtkSlicerVectorFieldRenderingModuleMRMLExport.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>

// STD includes
#include <string>

class vtkAppendPolyData;
class vtkClipPolyData;
class vtkDataArray;
class vtkGeometryFilter;
class vtkPoints;
class vtkPointSet;
class vtkTubeFilter;

/// \brief Builds the grid that shows a vector field as a deformation, in 3D and in slice views.
///
/// Input: vtkVectorFieldSampler output (lattice points, lattice size in GetLatticeSizeArrayName() field data); the
/// deforming vectors are selected with SetInputArrayToProcess(0, ...), as in vtkWarpVector. Output port 0:
/// - deformed grid lines (moved by ScaleFactor times the vectors),
/// - the deformed lattice surface (Fill): a sheet over a plane, outer cell faces over a region; needs input cells.
/// Output port UndeformedLinesPort: the undeformed grid lines, apart so that they can be drawn in a color of their own.
///
/// A line is drawn every Subdivision-th row but through every point along it, so sampling finer than the grid
/// spacing shows non-linear deformation. Lines break at invalid samples (GetValidSampleArrayName()).
///
/// \sa vtkVectorFieldSampler, vtkVectorFieldGeometry
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRML_EXPORT vtkVectorFieldGrid : public vtkPolyDataAlgorithm
{
public:
  static vtkVectorFieldGrid* New();
  vtkTypeMacro(vtkVectorFieldGrid, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Output port of the undeformed grid lines
  static const int UndeformedLinesPort = 1;

  ///@{
  /// Number of lattice steps between two grid lines; 1 draws a line through every row of samples.
  /// Default is 1.
  vtkSetClampMacro(Subdivision, int, 1, VTK_INT_MAX);
  vtkGetMacro(Subdivision, int);
  ///@}

  ///@{
  /// Factor that the vectors are multiplied with to deform the grid; negative deforms it the opposite way.
  /// Default is 1.
  vtkSetMacro(ScaleFactor, double);
  vtkGetMacro(ScaleFactor, double);
  ///@}

  ///@{
  /// Draw the deformed grid lines.
  /// Default is on.
  vtkSetMacro(DeformedLinesVisibility, bool);
  vtkGetMacro(DeformedLinesVisibility, bool);
  vtkBooleanMacro(DeformedLinesVisibility, bool);
  ///@}

  ///@{
  /// Draw the undeformed grid lines.
  /// Default is off.
  vtkSetMacro(UndeformedLinesVisibility, bool);
  vtkGetMacro(UndeformedLinesVisibility, bool);
  vtkBooleanMacro(UndeformedLinesVisibility, bool);
  ///@}

  ///@{
  /// Fill the surface of the deformed lattice.
  /// Default is off.
  vtkSetMacro(Fill, bool);
  vtkGetMacro(Fill, bool);
  vtkBooleanMacro(Fill, bool);
  ///@}

  ///@{
  /// Diameter of the tubes that the lines are drawn as. 0 draws plain lines.
  /// Default is 0.
  vtkSetClampMacro(LineDiameter, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(LineDiameter, double);
  ///@}

  ///@{
  /// Point array that says where the grid is shown: lines and fill are cut where it is below 0.5.
  /// Empty (default) shows the whole grid.
  void SetClipArrayName(const std::string& name);
  std::string GetClipArrayName() const { return this->ClipArrayName; }
  ///@}

protected:
  vtkVectorFieldGrid();
  ~vtkVectorFieldGrid() override;
  vtkVectorFieldGrid(const vtkVectorFieldGrid&) = delete;
  void operator=(const vtkVectorFieldGrid&) = delete;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  /// The polylines through the lattice points, on the input points
  void BuildLatticeLines(vtkPointSet* input, vtkPolyData* lines);
  /// Replace the points of the point set with a copy moved by ScaleFactor times the vectors.
  /// Does nothing if there are no vectors, or not one per point.
  void DeformPoints(vtkPointSet* pointSet, vtkDataArray* vectors);
  /// Cut where the clip array is below 0.5. Returns the input if no clip array is set.
  vtkSmartPointer<vtkPolyData> Clip(vtkPolyData* polyData);
  /// Cut the lines by the clip array and make them tubes if LineDiameter is positive
  vtkSmartPointer<vtkPolyData> FinishLines(vtkPolyData* lines);

  int Subdivision{ 1 };
  double ScaleFactor{ 1.0 };
  bool DeformedLinesVisibility{ true };
  bool UndeformedLinesVisibility{ false };
  bool Fill{ false };
  double LineDiameter{ 0.0 };
  std::string ClipArrayName;

  vtkNew<vtkClipPolyData> Clipper;
  vtkNew<vtkTubeFilter> Tuber;
  vtkNew<vtkGeometryFilter> FillSurface;
  vtkNew<vtkAppendPolyData> OutputAppender;
};

#endif
