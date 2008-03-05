#ifndef __vtkScriptedTask_h
#define __vtkScriptedTask_h

#include "vtkTcl.h"
#include "vtkTclUtil.h"
#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkSlicerTask.h"
#include "vtkSlicerLogic.h"

#include "vtkScriptedModule.h"

//
// a class to invoke tcl scripts in a slicer processing 
// rather than in the main thread
//

class VTK_SCRIPTED_EXPORT vtkScriptedTask : public vtkSlicerTask
{
public:
  static vtkScriptedTask *New();
  vtkTypeRevisionMacro(vtkScriptedTask,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Execute the task.
  virtual void Execute();
  
  // Description
  // hook for script to tell this instance what interpreter to use
  // - call it with the tag returned from AddObserver on this instance
  void SetInterpFromCommand(unsigned long tag);

  // Description
  // The string to be executed by the task
  vtkGetStringMacro (Script);
  vtkSetStringMacro (Script);

  // Description
  // The result of the script execution
  vtkGetStringMacro (Result);
  vtkSetStringMacro (Result);

  // Description
  // The return code from the string evaluation
  vtkGetMacro (ReturnCode, int);
  vtkSetMacro (ReturnCode, int);

protected:
  vtkScriptedTask();
  virtual ~vtkScriptedTask();
  vtkScriptedTask(const vtkScriptedTask&);
  void operator=(const vtkScriptedTask&);

  Tcl_Interp *Interp;           /* Tcl interpreter */
  char *Script;
  char *Result;
  int ReturnCode;

private:
  
};
#endif
