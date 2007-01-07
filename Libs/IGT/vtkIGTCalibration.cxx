#include "vtkIGTCalibration.h"

#include "vtkObjectFactory.h"

vtkIGTCalibration* vtkIGTCalibration::New()
{
  vtkObject* ret=vtkObjectFactory::CreateInstance("vtkIGTCalibration");
  if(ret)
    {
      return (vtkIGTCalibration*) ret;
    }
  return new vtkIGTCalibration;

}

vtkIGTCalibration::vtkIGTCalibration()
{
}



