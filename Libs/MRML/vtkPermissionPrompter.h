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
  vtkGetStringMacro ( Username );
  vtkSetStringMacro ( Username );

  // Description:
  // Member for storing a password, if required
  vtkGetStringMacro ( Password );
  vtkSetStringMacro ( Password );  
  
  vtkGetStringMacro (HostName );
  vtkSetStringMacro (HostName );

  // Description:
  // When set, a user won't be prompted again during a given session.
  // If not set (default) a user will be prompted for each transaction.
  vtkGetMacro ( Remember, int );
  vtkSetMacro ( Remember, int );

  vtkGetStringMacro ( PromptMessage );
  vtkSetStringMacro ( PromptMessage );
  vtkGetStringMacro ( PromptTitle );
  vtkSetStringMacro ( PromptTitle );

  // Description:
  // Sets null values for UserName and Password.
  //--- all derived classes should implement this method.
  //--- method should return -1 for empty fields
  //--- return 0 for a cancel
  //--- and return 1 for everything looks ok.
  virtual int Prompt(const char *message );
  
 private:
  char *Username;
  char *HostName;
  char *Password;
  int Remember;
  char *PromptMessage;
  char *PromptTitle;
    
 protected:
  vtkPermissionPrompter();
  virtual ~vtkPermissionPrompter();
  vtkPermissionPrompter(const vtkPermissionPrompter&);
  void operator=(const vtkPermissionPrompter&);

};

#endif

