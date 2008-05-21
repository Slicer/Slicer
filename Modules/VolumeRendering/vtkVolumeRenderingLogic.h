#ifndef __vtkVolumeRenderingLogic_h
#define __vtkVolumeRenderingLogic_h
#include "vtkSlicerModuleLogic.h"
#include "vtkVolumeRendering.h"

class VTK_SLICERVOLUMERENDERING_EXPORT vtkVolumeRenderingLogic :public vtkSlicerModuleLogic
{
public:
  static vtkVolumeRenderingLogic *New();
  vtkTypeMacro(vtkVolumeRenderingLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  //virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
    //                               void *callData ){};

  
  // Overload the scene setter, this is needed for module creation
  // via LoadableModuleFactory
  virtual void SetMRMLScene(vtkMRMLScene *scene);

protected:
  vtkVolumeRenderingLogic();
  ~vtkVolumeRenderingLogic();
  vtkVolumeRenderingLogic(const vtkVolumeRenderingLogic&);
  void operator=(const vtkVolumeRenderingLogic&);

  static bool First;

};

#endif
