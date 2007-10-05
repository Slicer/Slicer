#ifndef __vtkVolumeRenderingModuleLogic_h
#define __vtkVolumeRenderingModuleLogic_h
#include "vtkSlicerModuleLogic.h"
#include "vtkVolumeRenderingModule.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkVolumeRenderingModuleLogic :public vtkSlicerModuleLogic
{
public:
  static vtkVolumeRenderingModuleLogic *New();
  vtkTypeMacro(vtkVolumeRenderingModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  //virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
    //                               void *callData ){};

  


protected:
  vtkVolumeRenderingModuleLogic();
  ~vtkVolumeRenderingModuleLogic();
  vtkVolumeRenderingModuleLogic(const vtkVolumeRenderingModuleLogic&);
  void operator=(const vtkVolumeRenderingModuleLogic&);
};

#endif
