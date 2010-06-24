
#ifndef __vtkMRMLAbstractLogic_h
#define __vtkMRMLAbstractLogic_h


// VTK includes
#include <vtkObject.h>

#include "vtkMRMLLogicWin32Header.h"


class VTK_MRML_LOGIC_EXPORT vtkMRMLAbstractLogic : public vtkObject
{
public:
  static vtkMRMLAbstractLogic *New();
  vtkTypeMacro(vtkMRMLAbstractLogic, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

private:
  vtkMRMLAbstractLogic();
  ~vtkMRMLAbstractLogic();
  vtkMRMLAbstractLogic(const vtkMRMLAbstractLogic&);
  void operator=(const vtkMRMLAbstractLogic&);
};


#endif 

