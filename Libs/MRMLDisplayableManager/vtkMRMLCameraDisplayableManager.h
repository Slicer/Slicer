#ifndef __vtkMRMLCameraDisplayableManager_h
#define __vtkMRMLCameraDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLCameraNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLCameraDisplayableManager : 
  public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLCameraDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLCameraDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
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

  virtual void OnMRMLSceneClosedEvent();
  virtual void OnMRMLSceneImportedEvent();
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
