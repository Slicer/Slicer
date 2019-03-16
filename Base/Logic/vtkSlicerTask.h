#ifndef __vtkSlicerTask_h
#define __vtkSlicerTask_h

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkMRMLAbstractLogic.h"
#include "vtkSlicerBaseLogic.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerTask : public vtkObject
{
public:
  static vtkSlicerTask *New();
  vtkTypeMacro(vtkSlicerTask,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  typedef vtkMRMLAbstractLogic::TaskFunctionPointer TaskFunctionPointer;

  ///
  /// Set the function and object to call for the task.
  void SetTaskFunction(vtkMRMLAbstractLogic*, TaskFunctionPointer, void *clientdata);

  ///
  /// Execute the task.
  virtual void Execute();

  ///
  /// The type of task - this can be used, for example, to decide
  /// how many concurrent threads should be allowed
  enum
    {
    Undefined = 0,
    Processing,
    Networking
    };

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
  ~vtkSlicerTask() override;
  vtkSlicerTask(const vtkSlicerTask&);
  void operator=(const vtkSlicerTask&);

private:
  vtkSmartPointer<vtkMRMLAbstractLogic> TaskObject;
  vtkMRMLAbstractLogic::TaskFunctionPointer TaskFunction;
  void *TaskClientData;

  int Type;

};
#endif


