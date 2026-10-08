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
// or to a list of bounding boxes
// .SECTION Description

#ifndef __vtkTopologicalHierarchy_h
#define __vtkTopologicalHierarchy_h

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkPolyDataCollection.h>

#include "vtkSegmentationCoreExport.h"

class vtkIntArray;

/// \brief Algorithm class for computing topological hierarchy of multiple poly data models or bounding boxes.
///   The levels of the models are determined according to the models they contain, an outer
///   model always having larger level value than the inner ones. To determine whether a model
///   contains another, their bounding boxes are considered. It is possible to constrain a gap
///   or allow the inner model to protrude the surface of the outer one. The size of this gap
///   or allowance is defined as a factor /sa ContainConstraintFactor of the outer model size.
///   Only the bounding boxes are used, so the input can be given either as poly data models
///   (\sa SetInputPolyDataCollection) or directly as bounding boxes (\sa SetInputBounds), for example
///   computed from the voxels of each segment of a binary labelmap.
///   This algorithm can be used to automatically determine optimal opacities in complex scenes.
class vtkSegmentationCore_EXPORT vtkTopologicalHierarchy : public vtkObject
{
public:
  static vtkTopologicalHierarchy* New();
  vtkTypeMacro(vtkTopologicalHierarchy, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get output topological hierarchy levels
  virtual vtkIntArray* GetOutputLevels();

  /// Compute topological hierarchy levels for the input bounding boxes (if set) or for the
  /// input poly data models using their bounding boxes.
  /// This function has to be explicitly called!
  /// Output can be get using GetOutputLevels()
  virtual void Update();

  /// Set input poly data collection. Ignored if input bounds are set.
  vtkSetObjectMacro(InputPolyDataCollection, vtkPolyDataCollection);

  /// Set input bounding boxes: an array of 6 components (xmin, xmax, ymin, ymax, zmin, zmax), one tuple per item.
  /// If set, it is used instead of the input poly data collection. An item with empty bounds (min > max)
  /// neither contains nor is contained by any other item, so it gets level 0.
  vtkSetObjectMacro(InputBounds, vtkDoubleArray);
  vtkGetObjectMacro(InputBounds, vtkDoubleArray);

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

  /// Determines if the bounding box boundsOut contains boundsIn considering the constraint factor
  /// /sa ContainConstraintFactor
  bool Contains(const double boundsOut[6], const double boundsIn[6]);

  /// Determines if there are empty entries in the output level array
  bool OutputContainsEmptyLevels();

protected:
  /// Collection of poly data to determine the hierarchy for
  vtkPolyDataCollection* InputPolyDataCollection;

  /// Bounding boxes to determine the hierarchy for (used instead of the poly data collection if set)
  vtkDoubleArray* InputBounds;

  /// Array containing the topological hierarchy levels for the input items
  /// Update function needs to be called to compute the array
  /// The level values correspond to the item with the same index in the input bounds or poly data collection
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
