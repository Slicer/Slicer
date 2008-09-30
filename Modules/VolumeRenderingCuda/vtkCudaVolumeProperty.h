#ifndef VTKCUDAVOLUMEPROPERTY_H_
#define VTKCUDAVOLUMEPROPERTY_H_

#include "vtkVolumeProperty.h"
#include "vtkVolumeRenderingCuda.h"

class VTK_VOLUMERENDERINGCUDA_EXPORT vtkCudaVolumeProperty : public vtkVolumeProperty
{
  vtkTypeRevisionMacro(vtkCudaVolumeProperty, vtkVolumeProperty);
  static vtkCudaVolumeProperty *New();
  
  
protected:
  vtkCudaVolumeProperty();
  virtual ~vtkCudaVolumeProperty();
};

#endif /*VTKCUDAVOLUMEPROPERTY_H_*/
