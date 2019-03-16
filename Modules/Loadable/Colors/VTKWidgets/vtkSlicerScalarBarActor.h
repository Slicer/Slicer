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

  This file was originally developed by Kevin Wang, Princess Margaret Cancer Centre
  and was supported by Cancer Care Ontario (CCO)'s ACRU program
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)

==============================================================================*/

///  vtkSliceRTScalarBarActor - slicer vtk class for adding color names in scalarbar
///
/// This class enhances the vtkScalarBarActor class by adding color names
/// in the label display.

#ifndef __vtkSlicerScalarBarActor_h
#define __vtkSlicerScalarBarActor_h

// VTK includes
#include "vtkScalarBarActor.h"
#include "vtkStringArray.h"
#include "vtkVersion.h"

// MRMLLogic includes
#include "vtkSlicerColorsModuleVTKWidgetsExport.h"

/// \ingroup SlicerRt_QtModules_Isodose
class VTK_SLICER_COLORS_VTKWIDGETS_EXPORT vtkSlicerScalarBarActor
  : public vtkScalarBarActor
{
public:
  // The usual VTK class functions
  static vtkSlicerScalarBarActor *New();
  vtkTypeMacro(vtkSlicerScalarBarActor,vtkScalarBarActor);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get for the flag on using VTK6 annotation as label
  vtkGetMacro(UseAnnotationAsLabel, int);
  /// Set for the flag on using VTK6 annotation as label
  vtkSetMacro(UseAnnotationAsLabel, int);
  /// Get/Set for the flag on using VTK6 annotation as label
  vtkBooleanMacro(UseAnnotationAsLabel, int);

  /// Get if label should be centered on color swatches.
  /// Useful if each displayed color needs a label
  vtkGetMacro(CenterLabel, bool);
  /// Set if label should be centered on color swatches.
  /// Useful if each displayed color needs a label
  vtkSetMacro(CenterLabel, bool);
  /// Get/Set if label should be centered on color swatches.
  /// Useful if each displayed color needs a label
  vtkBooleanMacro(CenterLabel, bool);

protected:
  vtkSlicerScalarBarActor();
  ~vtkSlicerScalarBarActor() override;

  // Description:
  // Determine the size and placement of any tick marks to be rendered.
  //
  // This method must set this->P->TickBox.
  // It may depend on layout performed by ComputeScalarBarLength.
  //
  // The default implementation creates exactly this->NumberOfLabels
  // tick marks, uniformly spaced on a linear or logarithmic scale.
  //
  // It centers labels on color swatches instead of placing labels
  // at the edge.
  void LayoutTicks() override;

  // It centers labels on color swatches instead of placing labels
  // at the edge.
  void ConfigureTicks() override;

  /// flag for setting color name as label
  int UseAnnotationAsLabel;

  bool CenterLabel;

private:
  vtkSlicerScalarBarActor(const vtkSlicerScalarBarActor&) = delete;
  void operator=(const vtkSlicerScalarBarActor&) = delete;
};

#endif
