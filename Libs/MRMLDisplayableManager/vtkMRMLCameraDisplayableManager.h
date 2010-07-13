#ifndef __vtkMRMLCameraDisplayableManager_h
#define __vtkMRMLCameraDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLCameraNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLCameraDisplayableManager : 
  public vtkMRMLAbstractDisplayableManager
{

public:
  static vtkMRMLCameraDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLCameraDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);
  
  virtual void RemoveMRMLObservers();

  void UpdateCameraNode();

//  void AddCameraObservers();
//  void RemoveCameraObservers();

  ///
  /// Events
  //BTX
  enum
  {
    ActiveCameraChangedEvent   = 30000
  };
  //ETX

protected:

  vtkMRMLCameraDisplayableManager();
  virtual ~vtkMRMLCameraDisplayableManager();

  virtual void Create();

  virtual void OnMRMLSceneCloseEvent();
  virtual void OnMRMLSceneLoadEndEvent();
  virtual void OnMRMLSceneRestoredEvent();
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);

  void SetAndObserveCameraNode(vtkMRMLCameraNode * newCameraNode);
  
private:

  vtkMRMLCameraDisplayableManager(const vtkMRMLCameraDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLCameraDisplayableManager&);                     // Not Implemented
  
  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

};

#endif
