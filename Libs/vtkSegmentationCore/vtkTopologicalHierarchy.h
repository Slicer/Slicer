/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// .NAME vtkTopologicalHierarchy - Assigns hierarchy level values to the elements of a poly data collection
// .SECTION Description


#ifndef __vtkTopologicalHierarchy_h
#define __vtkTopologicalHierarchy_h

// VTK includes
#include <vtkPolyDataCollection.h>

#include "vtkSegmentationCoreConfigure.h"

class vtkIntArray;

/// \ingroup vtkSegmentationCore
/// \brief Algorithm class for computing topological hierarchy of multiple poly data models.
///   The levels of the models are determined according to the models they contain, an outer
///   model always having larger level value than the inner ones. To determine whether a model
///   contains another, their bounding boxes are considered. It is possible to constrain a gap
///   or allow the inner model to protrude the surface of the outer one. The size of this gap
///   or allowance is defined as a factor /sa ContainConstraintFactor of the outer model size.
///   This algorithm can be used to automatically determine optimal opacities in complex scenes.
class vtkSegmentationCore_EXPORT vtkTopologicalHierarchy : public vtkObject
{
public:

  static vtkTopologicalHierarchy *New();
  vtkTypeMacro(vtkTopologicalHierarchy, vtkObject );
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get output topological hierarchy levels
  virtual vtkIntArray* GetOutputLevels();

  /// Compute topological hierarchy levels for input poly data models using
  /// their bounding boxes.
  /// This function has to be explicitly called!
  /// Output can be get using GetOutputLevels()
  virtual void Update();

  /// Set input poly data collection
  vtkSetObjectMacro(InputPolyDataCollection, vtkPolyDataCollection);

  /// Set constraint factor (used when determining if a poly data contains another)
  vtkSetMacro(ContainConstraintFactor, double);
  /// Get constraint factor (used when determining if a poly data contains another)
  vtkGetMacro(ContainConstraintFactor, double);

protected:
  /// Set output topological hierarchy levels
  vtkSetObjectMacro(OutputLevels, vtkIntArray);

protected:
  /// Determines if polyOut contains polyIn considering the constraint factor
  /// /sa ContainConstraintFactor
  bool Contains(vtkPolyData* polyOut, vtkPolyData* polyIn);

  /// Determines if there are empty entries in the output level array
  bool OutputContainsEmptyLevels();

protected:
  /// Collection of poly data to determine the hierarchy for
  vtkPolyDataCollection* InputPolyDataCollection;

  /// Array containing the topological hierarchy levels for the input poly data
  /// Update function needs to be called to compute the array
  /// The level values correspond to the poly data with the same index in the input collection
  vtkIntArray* OutputLevels;

  /// Constraint factor used when determining if a poly data contains another
  /// It defines a 'gap' that is needed between the outer and inner poly data. The gap is computed
  /// as this factor multiplied by the bounding box edge length at each dimension.
  /// In case of positive value, the inner poly data has to be that much smaller than the outer one
  /// In case of negative value, it is rather an allowance by which the inner polydata can reach
  /// outside the other
  double ContainConstraintFactor;

  /// Maximum level that can be assigned to a poly data
  unsigned int MaximumLevel;

protected:
  vtkTopologicalHierarchy();
  ~vtkTopologicalHierarchy() override;

private:
  vtkTopologicalHierarchy(const vtkTopologicalHierarchy&) = delete;
  void operator=(const vtkTopologicalHierarchy&) = delete;
};

#endif
