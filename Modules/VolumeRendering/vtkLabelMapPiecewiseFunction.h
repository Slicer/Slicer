#ifndef __vtkLabelMapPiecewiseFunction_h
#define __vtkLabelMapPiecewiseFunction_h
#include "vtkVolumeRenderingModule.h"
#include "vtkPiecewiseFunction.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkLabelMapPiecewiseFunction : public vtkPiecewiseFunction
{
public:
    static vtkLabelMapPiecewiseFunction *New();
   // void PrintSelf(ostream& os, vtkIndent indent);
    
protected:
    vtkLabelMapPiecewiseFunction(void);
    ~vtkLabelMapPiecewiseFunction(void);
};
#endif
