#ifndef __vtkLiverAblationLogic_h
#define __vtkLiverAblationLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkLiverAblation.h"
#include "vtkLiverAblationMRMLManager.h"

class VTK_LIVERABLATION_EXPORT vtkLiverAblationLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkLiverAblationLogic *New();
  vtkTypeMacro(vtkLiverAblationLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: The name of the Module---this is used to construct
  // the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  vtkGetObjectMacro(MRMLManager, vtkLiverAblationMRMLManager);

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
  vtkLiverAblationLogic();
  ~vtkLiverAblationLogic();
  vtkLiverAblationLogic(const vtkLiverAblationLogic&);
  void operator=(const vtkLiverAblationLogic&);

  // the mrml manager is created in the constructor
  vtkSetObjectMacro(MRMLManager, vtkLiverAblationMRMLManager);

  // because the mrml nodes are very complicated for this module, we
  // delegate the handeling of them to a MRML manager
  vtkLiverAblationMRMLManager* MRMLManager;

  char *ModuleName;

};

#endif
