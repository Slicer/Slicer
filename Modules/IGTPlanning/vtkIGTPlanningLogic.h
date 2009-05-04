#ifndef __vtkIGTPlanningLogic_h
#define __vtkIGTPlanningLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkIGTPlanning.h"
#include "vtkIGTPlanningMRMLManager.h"

class VTK_IGT_EXPORT vtkIGTPlanningLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkIGTPlanningLogic *New();
  vtkTypeMacro(vtkIGTPlanningLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: The name of the Module---this is used to construct
  // the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  vtkGetObjectMacro(MRMLManager, vtkIGTPlanningMRMLManager);

  //
  // Register all the nodes used by this module with the current MRML
  // scene.
  virtual void RegisterMRMLNodesWithScene()
      { 
      this->MRMLManager->RegisterMRMLNodesWithScene(); 
      }

  virtual void SetAndObserveMRMLScene(vtkMRMLScene* scene)
      {
      Superclass::SetAndObserveMRMLScene(scene);
      this->MRMLManager->SetMRMLScene(scene);
      }

  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                   void *callData )
      { 
      this->MRMLManager->ProcessMRMLEvents(caller, event, callData); 
      }

  // events to observe
  virtual vtkIntArray* NewObservableEvents();

private:
  vtkIGTPlanningLogic();
  ~vtkIGTPlanningLogic();
  vtkIGTPlanningLogic(const vtkIGTPlanningLogic&);
  void operator=(const vtkIGTPlanningLogic&);

  // the mrml manager is created in the constructor
  vtkSetObjectMacro(MRMLManager, vtkIGTPlanningMRMLManager);

  // because the mrml nodes are very complicated for this module, we
  // delegate the handeling of them to a MRML manager
  vtkIGTPlanningMRMLManager* MRMLManager;

  char *ModuleName;

};

#endif
