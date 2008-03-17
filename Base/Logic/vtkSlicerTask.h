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
  virtual void Execute();

  // Description:
  // The type of task - this can be used, for example, to decide
  // how many concurrent threads should be allowed
  //BTX
  enum
    {
    Undefined = 0,
    Processing,
    Networking
    };
  //ETX
 
  vtkSetClampMacro (Type, int, vtkSlicerTask::Undefined, vtkSlicerTask::Networking);
  vtkGetMacro (Type, int);
  void SetTypeToProcessing() {this->SetType(vtkSlicerTask::Processing);};
  void SetTypeToNetworking() {this->SetType(vtkSlicerTask::Networking);};

  const char* GetTypeAsString( ) {
    switch (this->Type)
      {
      case vtkSlicerTask::Undefined: return "Undefined";
      case vtkSlicerTask::Processing: return "Processing";
      case vtkSlicerTask::Networking: return "Networking";
      }
    return "Unknown";
  }

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
  
  int Type;
  
};
#endif


