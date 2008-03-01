#ifndef __vtkPermissionPrompter_h
#define __vtkPermissionPrompter_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRML.h"

class VTK_MRML_EXPORT vtkPermissionPrompter : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkPermissionPrompter *New();
  vtkTypeRevisionMacro(vtkPermissionPrompter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
 private:

 protected:
  vtkPermissionPrompter();
  virtual ~vtkPermissionPrompter();
  vtkPermissionPrompter(const vtkPermissionPrompter&);
  void operator=(const vtkPermissionPrompter&);
};

#endif

