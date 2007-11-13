// .NAME vtkLabelMapColorTransferFunction - Extend vtkColorTransferFunction for Labelmaps
// .SECTION Description
//  vtkLabelMapColorTransferFunction - extends vtkColorTransferFunction for specific behavior of labelmaps.
// That means, that a specific labelmap can be linked with this Colortransferfunction and all the color mapping
// will be adjusted automatically to the corresponding vtkLookupTable
#ifndef __vtkLabelMapColorTransferFunction_h
#define __vtkLabelMapColorTransferFunction_h

#include "vtkVolumeRenderingModule.h"
#include "vtkColorTransferFunction.h"

class vtkMRMLScalarVolumeNode;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkLabelMapColorTransferFunction :public vtkColorTransferFunction
{
public:
    //Usual vtk methods: go to www.vtk.org for more details
    static vtkLabelMapColorTransferFunction *New();
    vtkTypeMacro(vtkLabelMapColorTransferFunction,vtkColorTransferFunction);
    //void PrintSelf(ostream& os, vtkIndent indent);

    void Init(vtkMRMLScalarVolumeNode *node);

    
protected:
    vtkLabelMapColorTransferFunction(void);
    ~vtkLabelMapColorTransferFunction(void);
private:
    vtkLabelMapColorTransferFunction(const vtkLabelMapColorTransferFunction&); //Not implemented
    void operator=(const vtkLabelMapColorTransferFunction&);//Not implemented
};
#endif
