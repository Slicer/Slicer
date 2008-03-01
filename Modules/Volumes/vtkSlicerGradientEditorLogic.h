// .NAME vtkSlicerGradientEditorLogic 
// .SECTION Description
// This class implements Slicer's main GradientsEditor Logic. Inherits most behavior from 
// vtkSlicerLogic. This class manages the logic associated with loading gradients from files 
// and testing given gradients for validity.
#ifndef __vtkSlicerGradientEditorLogic_h
#define __vtkSlicerGradientEditorLogic_h

#include "vtkVolumes.h"
#include "vtkSlicerLogic.h"

class vtkMRMLDiffusionWeightedVolumeNode;
class vtkDoubleArray;

class VTK_VOLUMES_EXPORT vtkSlicerGradientEditorLogic : public vtkSlicerLogic 
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerGradientEditorLogic *New();
    vtkTypeRevisionMacro(vtkSlicerGradientEditorLogic, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Method to add gradients from a file to the GUI. Filetypes are restricted to .txt/.nhdr.
    // Return value is 0 when file contains invalid values; otherwise 1.
    int AddGradients(const char *filename, int numberOfGradients, vtkDoubleArray *newBValue, 
      vtkDoubleArray *newGradients);

    // Description:
    // Method to parse given ASCII gradients back into arrays of gradients and bValues.
    // Return value is 0 when given gradients can't be parsed or contain invalid/not enough
    // values; otherwise 1.
    int ParseGradients(const char  *gradients, int numberOfGradients, vtkDoubleArray *newBValue, 
      vtkDoubleArray *newGradients);

    // Description:
    // Method to parse a string into a double value.
    // Return value is 0 when parsing was not successful; otherwise 1.
    //BTX
    int StringToDouble(const std::string &s, double &result);
    //ETX

  protected:
    vtkSlicerGradientEditorLogic(void);
    virtual ~vtkSlicerGradientEditorLogic(void);

  private:
    vtkSlicerGradientEditorLogic (const vtkSlicerGradientEditorLogic&); // Not implemented.
    void operator = (const vtkSlicerGradientEditorLogic&); //Not implemented.
  };
#endif

