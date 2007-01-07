#ifndef __vtkIGTTracker_h
#define __vtkIGTTracker_h

#include "vtkMatrix4x4.h"

class vtkIGTTracker : public vtkMatrix4x4 
{
 public:
      static vtkIGTTracker *New();
      vtkTypeMacro(vtkIGTTracker,vtkMatrix4x4);
      void PrintSelf(ostream& os, vtkIndent indent);

 protected:
      vtkIGTTracker();
      ~vtkIGTTracker() {};


};

#endif
