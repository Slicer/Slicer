#include "vtkSlicerTask.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTask);

//----------------------------------------------------------------------------
vtkSlicerTask::vtkSlicerTask()
{
  this->TaskObject = nullptr;
  this->TaskFunction = nullptr;
  this->TaskClientData = nullptr;
  this->Type = vtkSlicerTask::Undefined;
}
//----------------------------------------------------------------------------
vtkSlicerTask::~vtkSlicerTask() = default;

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
