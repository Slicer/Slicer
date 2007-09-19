#include "vtkLabelMapColorTransferFunction.h"
#include "vtkObjectFactory.h"

vtkLabelMapColorTransferFunction* vtkLabelMapColorTransferFunction::New(void)
{
     // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkLabelMapColorTransferFunction");
  if(ret)
    {
      return (vtkLabelMapColorTransferFunction*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkLabelMapColorTransferFunction;
}
vtkLabelMapColorTransferFunction::vtkLabelMapColorTransferFunction(void)
{
}

vtkLabelMapColorTransferFunction::~vtkLabelMapColorTransferFunction(void)
{
}
