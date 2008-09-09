#ifndef __vtkVolumeRenderingCudaModuleLogic_h
#define __vtkVolumeRenderingCudaModuleLogic_h
#include "vtkSlicerModuleLogic.h"
#include "vtkVolumeRenderingCudaModule.h"

class VTK_VOLUMERENDERINGCUDAMODULE_EXPORT vtkVolumeRenderingCudaModuleLogic :public vtkSlicerModuleLogic
{
public:
  static vtkVolumeRenderingCudaModuleLogic *New();
  vtkTypeMacro(vtkVolumeRenderingCudaModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  //virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
    //                               void *callData ){};

  


protected:
  vtkVolumeRenderingCudaModuleLogic();
  ~vtkVolumeRenderingCudaModuleLogic();
  vtkVolumeRenderingCudaModuleLogic(const vtkVolumeRenderingCudaModuleLogic&);
  void operator=(const vtkVolumeRenderingCudaModuleLogic&);
};

#endif
