#include "vtkSlicerTask.h"

vtkCxxRevisionMacro(vtkSlicerTask, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerTask);

// vtkSlicerTask* vtkSlicerTask::New()
// {
//   // First try to create the object from the vtkObjectFactory
//   vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerTask");
//   if(ret)
//     {
//       return (vtkSlicerTask*)ret;
//     }
//   // If the factory was unable to create the object, then create it here.
//   return new vtkSlicerTask;
// }

vtkSlicerTask
::vtkSlicerTask()
{
  this->TaskObject = 0;
  this->TaskFunction = 0;
}

vtkSlicerTask
::~vtkSlicerTask()
{
}


void
vtkSlicerTask
::SetTaskFunction(vtkSlicerLogic *object,
                  vtkSlicerLogic::TaskFunctionPointer function,
                  void *clientdata)
{
  this->TaskObject = object;
  this->TaskFunction = function;
  this->TaskClientData = clientdata;
}

void
vtkSlicerTask
::Execute()
{
  if (this->TaskObject)
    {
    ((*this->TaskObject).*(this->TaskFunction))(this->TaskClientData);
    }
}


void
vtkSlicerTask
::PrintSelf(ostream& os, vtkIndent indent)
{
}
