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

 protected:
  vtkMeasurementsLogic();
  ~vtkMeasurementsLogic();
  vtkMeasurementsLogic(const vtkMeasurementsLogic&);
  void operator=(const vtkMeasurementsLogic&);
};

#endif

