#ifndef __vtkLabelMapColorTransferFunction_h
#define __vtkLabelMapColorTransferFunction_h

#include "vtkVolumeRenderingModule.h"
#include "vtkColorTransferFunction.h"
#include "vtkMRMLScalarVolumeNode.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkLabelMapColorTransferFunction :public vtkColorTransferFunction
{
public:
    static vtkLabelMapColorTransferFunction *New();
    vtkTypeMacro(vtkLabelMapColorTransferFunction,vtkColorTransferFunction);
    void Init(vtkMRMLScalarVolumeNode *node);
    //void PrintSelf(ostream& os, vtkIndent indent);
    
public:
    vtkLabelMapColorTransferFunction(void);
    ~vtkLabelMapColorTransferFunction(void);
};
#endif
