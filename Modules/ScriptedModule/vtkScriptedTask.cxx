#include "vtkScriptedTask.h"

vtkCxxRevisionMacro(vtkScriptedTask, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkScriptedTask);

vtkScriptedTask
::vtkScriptedTask()
{
  this->Interp = NULL;
  this->Script = NULL;
  this->Result = NULL;
  this->ReturnCode = TCL_OK;
}

vtkScriptedTask
::~vtkScriptedTask()
{
}

void
vtkScriptedTask
::Execute()
{
  if ( !this->Interp )
    {
    vtkErrorMacro( "Cannot execute scripted task - no interpreter.  Did you call SetInterpFromCommand?" );
    return;
    }

  if ( !this->Script )
    {
    vtkErrorMacro( "Cannot execute scripted task - no script." );
    return;
    }

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION <= 2
  this->ReturnCode = Tcl_GlobalEval(this->Interp, this->Script);
#else
  this->ReturnCode = Tcl_EvalEx(this->Interp, Script, -1, TCL_EVAL_GLOBAL);
#endif  

  this->SetResult( Tcl_GetStringResult (this->Interp) );
}

void 
vtkScriptedTask
::SetInterpFromCommand(unsigned long tag)
{
  vtkCommand *c = this->GetCommand(tag);
  vtkTclCommand *tc = (vtkTclCommand *) c;

  this->Interp = tc->Interp;
}


void
vtkScriptedTask
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->Interp) os << indent << "Interp: " << this->Interp << "\n";
  if (this->Script) os << indent << "Script: " << this->Script << "\n";
  if (this->Result) os << indent << "Result: " << this->Result << "\n";
  os << indent << "ReturnCode: " << this->ReturnCode << "\n";
}
