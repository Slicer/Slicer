#pragma once

class vtkLabelMapPiecewiseFunction
{
public:
    static vtkMRMLVolumeRenderingDisplayNode *New();
    void PrintSelf(ostream& os, vtkIndent indent);
    
protected:
    vtkLabelMapPiecewiseFunction(void);
    ~vtkLabelMapPiecewiseFunction(void);
};
