#ifndef __vtkMeasurementsLogic_h
#define __vtkMeasurementsLogic_h

#include "vtkObject.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkMeasurementsWin32Header.h"

#include <string>
#include <vector>

class VTK_MEASUREMENTS_EXPORT vtkMeasurementsLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkMeasurementsLogic *New();
  vtkTypeMacro(vtkMeasurementsLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Make a new ruler node, placing it between the last two fiducials added
  /// on the currently selected list, then remove those two fiducials. If the
  /// rulerName is not NULL or an empty string, use that to name the new node
  /// and in the list annotation string; if it's NULL use 'Ruler' for the
  /// annotation, but don't rename the node; if it's an empty string don't use
  /// a prefix for the annotation nor rename the node. Fails if can't get at
  /// the mrml scene, the selection node, the active fid list, or there aren't
  /// two points in the active fid list.  Returns 1 on success, 0 on failure.
  int NewRulerBetweenFiducials(const char *rulerName = NULL);
  
 protected:
  vtkMeasurementsLogic();
  ~vtkMeasurementsLogic();
  vtkMeasurementsLogic(const vtkMeasurementsLogic&);
  void operator=(const vtkMeasurementsLogic&);
};

#endif

