#include "vtkSlicerTask.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTask);

//----------------------------------------------------------------------------
vtkSlicerTask::vtkSlicerTask()
{
  this->TaskObject = 0;
  this->TaskFunction = 0;
  this->Type = vtkSlicerTask::Undefined;
}
//----------------------------------------------------------------------------
vtkSlicerTask::~vtkSlicerTask()
{
}

//----------------------------------------------------------------------------
void vtkSlicerTask::SetTaskFunction(vtkMRMLAbstractLogic *object,
                                    vtkMRMLAbstractLogic::TaskFunctionPointer function,
                                    void *clientdata)
{
  this->TaskObject = object;
  this->TaskFunction = function;
  this->TaskClientData = clientdata;
}

//----------------------------------------------------------------------------
void vtkSlicerTask::Execute()
{
  if (this->TaskObject)
    {
    ((*this->TaskObject).*(this->TaskFunction))(this->TaskClientData);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerTask::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}
