
#ifndef __vtkMRMLAbstractDisplayableManager_h
#define __vtkMRMLAbstractDisplayableManager_h


// VTK includes
#include <vtkObject.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"


class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractDisplayableManager : public vtkObject
{
public:
  static vtkMRMLAbstractDisplayableManager *New();
  vtkTypeMacro(vtkMRMLAbstractDisplayableManager, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

private:
  vtkMRMLAbstractDisplayableManager();
  ~vtkMRMLAbstractDisplayableManager();
  vtkMRMLAbstractDisplayableManager(const vtkMRMLAbstractDisplayableManager&);
  void operator=(const vtkMRMLAbstractDisplayableManager&);
};


#endif 

