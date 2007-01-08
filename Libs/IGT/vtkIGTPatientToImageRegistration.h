#ifndef __vtkIGTPatientToImageRegistration_h
#define __vtkIGTPatientToImageRegistration_h

#include "vtkMatrix4x4.h"

class vtkIGTPatientToImageRegistration : public vtkMatrix4x4 
{

 public:
      static vtkIGTPatientToImageRegistration *New();
      vtkTypeMacro(vtkIGTPatientToImageRegistration,vtkMatrix4x4);
      void PrintSelf(ostream& os, vtkIndent indent);

 protected:
      vtkIGTPatientToImageRegistration();
      ~vtkIGTPatientToImageRegistration() {};


};


#endif
