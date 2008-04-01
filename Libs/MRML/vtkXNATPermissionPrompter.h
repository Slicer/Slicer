#ifndef __vtkXNATPermissionPrompter_h
#define __vtkXNATPermissionPrompter_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRML.h"
#include "vtkPermissionPrompter.h"

class VTK_MRML_EXPORT vtkXNATPermissionPrompter : public vtkPermissionPrompter
{
  public:
  
  // The Usual vtk class functions
  static vtkXNATPermissionPrompter *New();
  vtkTypeRevisionMacro(vtkXNATPermissionPrompter, vtkPermissionPrompter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Member for storing a host name, if required
  vtkGetStringMacro ( Host );
  vtkSetStringMacro ( Host );

 private:
  char *Host;
  
 protected:
  vtkXNATPermissionPrompter();
  virtual ~vtkXNATPermissionPrompter();
  vtkXNATPermissionPrompter(const vtkXNATPermissionPrompter&);
  void operator=(const vtkXNATPermissionPrompter&);

};

#endif

