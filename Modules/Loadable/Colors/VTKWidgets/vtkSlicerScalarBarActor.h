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
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Get for the flag on using VTK6 annotation as label
  vtkGetMacro(UseAnnotationAsLabel, int);
  /// Set for the flag on using VTK6 annotation as label
  vtkSetMacro(UseAnnotationAsLabel, int);
  /// Get/Set for the flag on using VTK6 annotation as label
  vtkBooleanMacro(UseAnnotationAsLabel, int);

protected:
  vtkSlicerScalarBarActor();
  ~vtkSlicerScalarBarActor();

  // Description:
  // Determine the size and placement of any tick marks to be rendered.
  //
  // This method must set this->P->TickBox.
  // It may depend on layout performed by ComputeScalarBarLength.
  //
  // The default implementation creates exactly this->NumberOfLabels
  // tick marks, uniformly spaced on a linear or logarithmic scale.
  virtual void LayoutTicks();

  /// flag for setting color name as label
  int UseAnnotationAsLabel;

private:
  vtkSlicerScalarBarActor(const vtkSlicerScalarBarActor&);  // Not implemented.
  void operator=(const vtkSlicerScalarBarActor&);  // Not implemented.
};

#endif
