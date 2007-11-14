// .NAME vtkLabelMapPiecewiseFunction - Extend vtkPiecewiseFunction for Labelmaps
// .SECTION Description
//  vtkLabelMapPiecewiseFunction - extends vtkPiecewiseFunction for specific behavior of labelmaps.
// That means, that piecewise FUnction can be linked with a labelmap. Also the vtkPiecewiseFunction should not be
// edited anymore with AddPoints and corresponding methods but with EditLabel and GetLabel instead, as this is needed
// to help and save the opacities later.
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

    // Description:
    // Initialize the labelmap piecewise function with a vtkMRMLScalarVolume (to get the vtkLookuptable)
    // and opacity that determines which opacity every value in the vtkLookuptable will get and a 
    // treshold that determines how if you ignore a grayscale value while setting up the opacity or not
    // Note: the treshold functionality has not been implemented yet
    void Init(vtkMRMLScalarVolumeNode *node,double opacity, int treshold);

    // Description:
    // Change the opacity at a specific index. Use that instead of AddPoint or corresponding methods in vtkPiecewiseFunction
    void EditLabel(int index,double opacity);

    //Description:
    // Get the opacity at a specific index
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
    // Description:
    //
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
