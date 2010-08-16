#ifndef __vtkMRMLScriptedDisplayableManager_h
#define __vtkMRMLScriptedDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLScriptedDisplayableManager :
  public vtkMRMLAbstractDisplayableManager
{

public:
  static vtkMRMLScriptedDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLScriptedDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  void SetPythonSource(const std::string& pythonSource);

protected:

  vtkMRMLScriptedDisplayableManager();
  virtual ~vtkMRMLScriptedDisplayableManager();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  virtual void Create();

  virtual void RemoveMRMLObservers();

  virtual void UpdateFromMRML();

  virtual void OnInteractorStyleEvent(int eventid);

  virtual void onMRMLDisplayableNodeModifiedEvent(vtkObject* caller);
  
private:

  vtkMRMLScriptedDisplayableManager(const vtkMRMLScriptedDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLScriptedDisplayableManager&);                   // Not Implemented
  
  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

};

#endif
