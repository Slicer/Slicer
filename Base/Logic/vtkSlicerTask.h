#ifndef __vtkSlicerTask_h
#define __vtkSlicerTask_h

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkSlicerLogic.h"


class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerTask : public vtkObject
{
public:
  static vtkSlicerTask *New();
  vtkTypeRevisionMacro(vtkSlicerTask,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  typedef vtkSlicerLogic::TaskFunctionPointer TaskFunctionPointer;
  //ETX
  
  // Description:
  // Set the function and object to call for the task.
  void SetTaskFunction(vtkSlicerLogic*, TaskFunctionPointer, void *clientdata);
  
  // Description:
  // Execute the task.
  void Execute();
  
protected:
  vtkSlicerTask();
  virtual ~vtkSlicerTask();
  vtkSlicerTask(const vtkSlicerTask&);
  void operator=(const vtkSlicerTask&);

private:
  //BTX
  vtkSmartPointer<vtkSlicerLogic> TaskObject;
  vtkSlicerLogic::TaskFunctionPointer TaskFunction;
  void *TaskClientData;
  //ETX
  
};
#endif


