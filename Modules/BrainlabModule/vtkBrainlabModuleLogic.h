#ifndef __vtkBrainlabModuleLogic_h
#define __vtkBrainlabModuleLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkBrainlabModule.h"
#include "vtkBrainlabModuleMRMLManager.h"

class VTK_BRAINLABMODULE_EXPORT vtkBrainlabModuleLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkBrainlabModuleLogic *New();
  vtkTypeMacro(vtkBrainlabModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: The name of the Module---this is used to construct
  // the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  vtkGetObjectMacro(MRMLManager, vtkBrainlabModuleMRMLManager);


private:
  vtkBrainlabModuleLogic();
  ~vtkBrainlabModuleLogic();
  vtkBrainlabModuleLogic(const vtkBrainlabModuleLogic&);
  void operator=(const vtkBrainlabModuleLogic&);

  virtual void SetMRMLSceneInternal(vtkMRMLScene* scene);
  //
  // Register all the nodes used by this module with the current MRML
  // scene.
  virtual void RegisterNodes();
  virtual void ProcessMRMLSceneEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData);

  // the mrml manager is created in the constructor
  vtkSetObjectMacro(MRMLManager, vtkBrainlabModuleMRMLManager);

  // because the mrml nodes are very complicated for this module, we
  // delegate the handeling of them to a MRML manager
  vtkBrainlabModuleMRMLManager* MRMLManager;

  char *ModuleName;

};

#endif
