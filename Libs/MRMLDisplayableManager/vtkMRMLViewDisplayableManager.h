#ifndef __vtkMRMLViewDisplayableManager_h
#define __vtkMRMLViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLCameraNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLViewDisplayableManager : 
  public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLViewDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLViewDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

protected:

  vtkMRMLViewDisplayableManager();
  virtual ~vtkMRMLViewDisplayableManager();

  virtual void Create();
  
private:

  vtkMRMLViewDisplayableManager(const vtkMRMLViewDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLViewDisplayableManager&);                     // Not Implemented
  
  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

};

#endif
