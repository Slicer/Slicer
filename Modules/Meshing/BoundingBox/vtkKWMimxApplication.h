#ifndef __vtkKWMimxApplication_h
#define __vtkKWMimxApplication_h

#include "vtkKWApplication.h"
#include "vtkBoundingBox.h"

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxApplication : public vtkKWApplication
{
public:
  static vtkKWMimxApplication* New();
  vtkTypeRevisionMacro(vtkKWMimxApplication,vtkKWApplication);
 
protected:
  vtkKWMimxApplication();
  ~vtkKWMimxApplication() {};

private:
  vtkKWMimxApplication(const vtkKWMimxApplication&);   // Not implemented.
  void operator=(const vtkKWMimxApplication&);  // Not implemented.
};

#endif
