// .NAME vtkMRMLVolumeRenderingNode - MRML node to represent volume rendering information
// .SECTION Description
// This node is especially used to store visualization parameter sets for volume rendering

#ifndef __vtkLabelMapPiecewiseFunction_h
#define __vtkLabelMapPiecewiseFunction_h
#include "vtkVolumeRenderingModule.h"
#include "vtkPiecewiseFunction.h"
#include <string>

class vtkMRMLScalarVolumeNode;
class vtkMRMLColorNode;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkLabelMapPiecewiseFunction :public vtkPiecewiseFunction
{
public:
    // Description:
    // Usual vtk methods: go to www.vtk.org for more details
    static vtkLabelMapPiecewiseFunction *New();
    vtkTypeMacro(vtkLabelMapPiecewiseFunction,vtkPiecewiseFunction);
    void PrintSelf(ostream& os, vtkIndent indent);

    void Init(vtkMRMLScalarVolumeNode *node,double opacity, int treshold);
    void UpdateFromOpacities(vtkMRMLScalarVolumeNode *node);
    void EditLabel(int index,double opacity);
    double GetLabel(int index);
    vtkGetMacro(Size,int);
    //BTX
    vtkGetMacro(Opacities,double*);
    //ETX
    //BTX
    // Description: get a string representation of the size and opacities.
    // format is: <size> <opacity1> ... <opacitySize>
    std::string GetSaveString();

    // Description:
    // Fill the opacity values with the same value declared in str.
    // Use the same format like in GetSaveString <size> <opacity1> ... <opacitySize>
    void FillFromString(std::string str);
    //ETX    
protected:
    // Description:
    // Count of stored opacities in the labelmap.
    int Size;
    // Description:
    // Stored Opacities for labelmaps. The opacities start at the beginning
    // of the associated vtkLookupTable
    double *Opacities;
    vtkMRMLColorNode *ColorNode;
    // Description:
    // Use ::New() to get a new instance.
    vtkLabelMapPiecewiseFunction(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkLabelMapPiecewiseFunction(void);

private:
    // Description:
    // Caution: Not implemented
    vtkLabelMapPiecewiseFunction(const vtkLabelMapPiecewiseFunction&);//Not implemented
    void operator=(const vtkLabelMapPiecewiseFunction&);//Not implemented
};
#endif
