#ifndef __vtkIGTCalibration_h
#define __vtkIGTCalibration_h

#include "vtkMatrix4x4.h"

class vtkIGTCalibration : public vtkMatrix4x4 
{
 public:
      static vtkIGTCalibration *New();
      vtkTypeMacro(vtkIGTCalibration,vtkMatrix4x4);
      void PrintSelf(ostream& os, vtkIndent indent);

 protected:
      vtkIGTCalibration();
      ~vtkIGTCalibration() {};


};


#endif
