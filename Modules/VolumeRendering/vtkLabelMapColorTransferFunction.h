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
    // Description:
    // Usual vtk methods: go to www.vtk.org for more details
    static vtkLabelMapColorTransferFunction *New();
    vtkTypeMacro(vtkLabelMapColorTransferFunction,vtkColorTransferFunction);   
    void PrintSelf(ostream& os, vtkIndent indent);
    
    // Description:
    // Initialize the transfer function with a label map. Make sure that a vtkLookupTable is associated
    // with the ScalarVolumeNode
    void Init(vtkMRMLScalarVolumeNode *node);

    
protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkLabelMapColorTransferFunction(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkLabelMapColorTransferFunction(void);
private:
    // Description:
    // Caution: Not implemented
    vtkLabelMapColorTransferFunction(const vtkLabelMapColorTransferFunction&); //Not implemented
    void operator=(const vtkLabelMapColorTransferFunction&);//Not implemented
};
#endif
