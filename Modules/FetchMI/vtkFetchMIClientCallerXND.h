#ifndef __vtkFetchMIClientCallerXND_h
#define __vtkFetchMIClientCallerXND_h

#include "vtkObject.h"
#include "vtkFetchMIWin32Header.h"
#include "vtkFetchMIClientCaller.h"

class VTK_FETCHMI_EXPORT vtkFetchMIClientCallerXND : public vtkFetchMIClientCaller
{
  
public:
  static vtkFetchMIClientCallerXND* New();
  vtkTypeRevisionMacro(vtkFetchMIClientCallerXND,vtkFetchMIClientCaller);
  void PrintSelf(ostream& os, vtkIndent indent);

 protected:
  vtkFetchMIClientCallerXND();
  virtual ~vtkFetchMIClientCallerXND();

  vtkFetchMIClientCallerXND(const vtkFetchMIClientCallerXND&); // Not implemented
  void operator=(const vtkFetchMIClientCallerXND&); // Not Implemented
};

#endif

