#ifndef __vtkPermissionPrompter_h
#define __vtkPermissionPrompter_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>

class VTK_MRML_EXPORT vtkPermissionPrompter : public vtkObject
{
  public:

  /// The Usual vtk class functions
  static vtkPermissionPrompter *New();
  vtkTypeMacro(vtkPermissionPrompter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Member for storing a user name, if required
  vtkGetStringMacro ( Username );
  vtkSetStringMacro ( Username );

  ///
  /// Member for storing a password, if required
  vtkGetStringMacro ( Password );
  vtkSetStringMacro ( Password );

  vtkGetStringMacro (HostName );
  vtkSetStringMacro (HostName );

  ///
  /// When set, a user won't be prompted again during a given session.
  /// If not set (default) a user will be prompted for each transaction.
  vtkGetMacro ( Remember, int );
  vtkSetMacro ( Remember, int );

  vtkGetStringMacro ( PromptMessage );
  vtkSetStringMacro ( PromptMessage );
  vtkGetStringMacro ( PromptTitle );
  vtkSetStringMacro ( PromptTitle );

  ///
  /// Sets null values for UserName and Password.
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
  ~vtkPermissionPrompter() override;
  vtkPermissionPrompter(const vtkPermissionPrompter&);
  void operator=(const vtkPermissionPrompter&);

};

#endif
