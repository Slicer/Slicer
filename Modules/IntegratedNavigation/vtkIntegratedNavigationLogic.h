#ifndef __vtkIntegratedNavigationLogic_h
#define __vtkIntegratedNavigationLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkIntegratedNavigation.h"
#include "vtkIntegratedNavigationMRMLManager.h"

class VTK_IGT_EXPORT vtkIntegratedNavigationLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkIntegratedNavigationLogic *New();
  vtkTypeMacro(vtkIntegratedNavigationLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: The name of the Module---this is used to construct
  // the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  vtkGetObjectMacro(MRMLManager, vtkIntegratedNavigationMRMLManager);

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
  vtkIntegratedNavigationLogic();
  ~vtkIntegratedNavigationLogic();
  vtkIntegratedNavigationLogic(const vtkIntegratedNavigationLogic&);
  void operator=(const vtkIntegratedNavigationLogic&);

  // the mrml manager is created in the constructor
  vtkSetObjectMacro(MRMLManager, vtkIntegratedNavigationMRMLManager);

  // because the mrml nodes are very complicated for this module, we
  // delegate the handeling of them to a MRML manager
  vtkIntegratedNavigationMRMLManager* MRMLManager;

  char *ModuleName;

};

#endif
