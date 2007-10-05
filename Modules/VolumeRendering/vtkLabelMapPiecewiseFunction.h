#ifndef __vtkLabelMapPiecewiseFunction_h
#define __vtkLabelMapPiecewiseFunction_h
#include "vtkVolumeRenderingModule.h"
#include "vtkPiecewiseFunction.h"
#include "vtkMRMLScalarVolumeNode.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkLabelMapPiecewiseFunction :public vtkPiecewiseFunction
{
public:
    static vtkLabelMapPiecewiseFunction *New();
    vtkTypeMacro(vtkLabelMapPiecewiseFunction,vtkPiecewiseFunction);
    // void PrintSelf(ostream& os, vtkIndent indent);
    void Init(vtkMRMLScalarVolumeNode *node,double opacity, int treshold);
    void UpdateFromOpacities(vtkMRMLScalarVolumeNode *node);
    void EditLabel(int index,double opacity);
    double GetLabel(int index);
    vtkGetMacro(Size,int);
    //BTX
    std::string GetSaveString();
    void FillFromString(std::string);
    //ETX    
protected:
    int Size;
    double *Opacities;
    vtkMRMLColorNode *ColorNode;
    vtkLabelMapPiecewiseFunction(void);
    ~vtkLabelMapPiecewiseFunction(void);
    vtkLabelMapPiecewiseFunction(const vtkLabelMapPiecewiseFunction&);//Not implemented
    void operator=(const vtkLabelMapPiecewiseFunction&);//Not implemented
};
#endif
