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

  // Description:
  // Member for storing a user name, if required
  vtkGetStringMacro ( UserName );
  vtkSetStringMacro ( UserName );

  // Description:
  // Member for storing a password, if required
  vtkGetStringMacro ( Password );
  vtkSetStringMacro ( Password );  
  
  // Description:
  // When set, a user won't be prompted again during a given session.
  // If not set (default) a user will be prompted for each transaction.
  vtkGetMacro ( Remember, int );
  vtkSetMacro ( Remember, int );

 private:
  char *UserName;
  char *Password;
  int Remember;
    
 protected:
  vtkPermissionPrompter();
  virtual ~vtkPermissionPrompter();
  vtkPermissionPrompter(const vtkPermissionPrompter&);
  void operator=(const vtkPermissionPrompter&);

};

#endif

