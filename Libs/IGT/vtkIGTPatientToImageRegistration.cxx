#include "vtkIGTPatientToImageRegistration.h"

#include "vtkObjectFactory.h"

vtkIGTPatientToImageRegistration* vtkIGTPatientToImageRegistration::New()
{
  vtkObject* ret=vtkObjectFactory::CreateInstance("vtkIGTPatientToImageRegistration");
  if(ret)
    {
      return (vtkIGTPatientToImageRegistration*) ret;
    }
  return new vtkIGTPatientToImageRegistration;

}

vtkIGTPatientToImageRegistration::vtkIGTPatientToImageRegistration()
{
}



