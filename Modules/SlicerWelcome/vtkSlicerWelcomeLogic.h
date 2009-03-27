#ifndef __vtkSlicerWelcomeLogic_h
#define __vtkSlicerWelcomeLogic_h

#include "vtkObject.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerWelcomeWin32Header.h"

#include <string>
#include <vector>

class VTK_SLICERWELCOME_EXPORT vtkSlicerWelcomeLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkSlicerWelcomeLogic *New();
  vtkTypeMacro(vtkSlicerWelcomeLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

 protected:
  vtkSlicerWelcomeLogic();
  ~vtkSlicerWelcomeLogic();
  vtkSlicerWelcomeLogic(const vtkSlicerWelcomeLogic&);
  void operator=(const vtkSlicerWelcomeLogic&);
};

#endif

