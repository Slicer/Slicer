#ifndef __vtkIGTNavigationLogic_h
#define __vtkIGTNavigationLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkIGTNavigation.h"
#include "vtkIGTNavigationMRMLManager.h"

class VTK_IGT_EXPORT vtkIGTNavigationLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkIGTNavigationLogic *New();
  vtkTypeMacro(vtkIGTNavigationLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: The name of the Module---this is used to construct
  // the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  vtkGetObjectMacro(MRMLManager, vtkIGTNavigationMRMLManager);

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
  vtkIGTNavigationLogic();
  ~vtkIGTNavigationLogic();
  vtkIGTNavigationLogic(const vtkIGTNavigationLogic&);
  void operator=(const vtkIGTNavigationLogic&);

  // the mrml manager is created in the constructor
  vtkSetObjectMacro(MRMLManager, vtkIGTNavigationMRMLManager);

  // because the mrml nodes are very complicated for this module, we
  // delegate the handeling of them to a MRML manager
  vtkIGTNavigationMRMLManager* MRMLManager;

  char *ModuleName;

};

#endif
