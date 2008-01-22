#ifndef __vtkSlicerGradientEditorLogic_h
#define __vtkSlicerGradientEditorLogic_h

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"

#include "vtkMRMLDiffusionWeightedVolumeNode.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerGradientEditorLogic : public vtkSlicerLogic 
  {
  public:
    // Description:
    // Usual vtk class functions.
    static vtkSlicerGradientEditorLogic *New();
    vtkTypeRevisionMacro(vtkSlicerGradientEditorLogic, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Method to add gradients from files to the GUI.
    void AddGradients(const char* filename, vtkMRMLDiffusionWeightedVolumeNode *dwiNode);

  protected:
    vtkSlicerGradientEditorLogic(void);
    virtual ~vtkSlicerGradientEditorLogic(void);

  private:
    vtkSlicerGradientEditorLogic ( const vtkSlicerGradientEditorLogic& ); // Not implemented.
    void operator = ( const vtkSlicerGradientEditorLogic& ); //Not implemented.
  };
#endif

