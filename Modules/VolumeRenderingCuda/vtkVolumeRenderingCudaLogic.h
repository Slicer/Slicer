#ifndef __vtkVolumeRenderingCudaLogic_h
#define __vtkVolumeRenderingCudaLogic_h
#include "vtkSlicerModuleLogic.h"
#include "vtkVolumeRenderingCuda.h"

class VTK_VOLUMERENDERINGCUDA_EXPORT vtkVolumeRenderingCudaLogic :public vtkSlicerModuleLogic
{
public:
  static vtkVolumeRenderingCudaLogic *New();
  vtkTypeMacro(vtkVolumeRenderingCudaLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  //virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
    //                               void *callData ){};

  


protected:
  vtkVolumeRenderingCudaLogic();
  ~vtkVolumeRenderingCudaLogic();
  vtkVolumeRenderingCudaLogic(const vtkVolumeRenderingCudaLogic&);
  void operator=(const vtkVolumeRenderingCudaLogic&);
};

#endif
