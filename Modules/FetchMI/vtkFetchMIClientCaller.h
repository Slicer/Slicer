#ifndef __vtkFetchMIClientCaller_h
#define __vtkFetchMIClientCaller_h

#include "vtkObject.h"
#include "vtkFetchMIWin32Header.h"


class VTK_FETCHMI_EXPORT vtkFetchMIClientCaller : public vtkObject
{
  
public:
  static vtkFetchMIClientCaller* New();
  vtkTypeRevisionMacro(vtkFetchMIClientCaller,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set on the Name of this ClientCaller
  vtkGetStringMacro ( Name );
  vtkSetStringMacro ( Name );

 protected:
  vtkFetchMIClientCaller();
  virtual ~vtkFetchMIClientCaller();

  char *Name;
  
  vtkFetchMIClientCaller(const vtkFetchMIClientCaller&); // Not implemented
  void operator=(const vtkFetchMIClientCaller&); // Not Implemented
};

#endif

