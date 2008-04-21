// .NAME vtkSlicerGradientEditorLogic 
// .SECTION Description
// This class implements Slicer's main GradientsEditor Logic, inherits most behavior from 
// vtkSlicerLogic. This class manages the logic associated with loading gradients from files 
// and testing given gradients for validity. It also holds the logic for the undo/redo/restore 
// mechanism of the GradientEditor.
#ifndef __vtkSlicerGradientEditorLogic_h
#define __vtkSlicerGradientEditorLogic_h

#include "vtkVolumes.h"
#include "vtkSlicerLogic.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include <vtkstd/vector>

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
    // Adds gradients from a file to the GUI. Filetypes are restricted to .txt/.nhdr.
    // Return value is 0 when file contains invalid values; otherwise 1.
    int AddGradients(const char *filename, int numberOfGradients, vtkDoubleArray *newBValue, 
      vtkDoubleArray *newGradients);

    // Description:
    // Parses given ASCII gradients back into arrays of gradients and bValues.
    // Return value is 0 when given gradients can't be parsed or contain invalid/not enough
    // values; otherwise 1.
    int ParseGradients(const char  *gradients, int numberOfGradients, vtkDoubleArray *newBValues, 
      vtkDoubleArray *newGradients);
    
    //BTX
    // Description:
    // Parses given gradients and bValues back into a string and returns it.
    std::string GetGradientsAsString(vtkDoubleArray *BValues, vtkDoubleArray *Gradients);

    // Description:
    // Parses a string into a double value.
    // Return value is 0 when parsing was not successful; otherwise 1.
    int StringToDouble(const std::string &s, double &result);
    //ETX

    // Description:
    // Pushes the current ActiveVolumeNode onto the UndoRedoStack, and makes a backup 
    // copy of the node so that changes are undoable/redoable; 
    void SaveStateForUndoRedo();

    // Description:
    // Redo the last change of measurement/gradient values (after undo).
    void Redo();

    // Description:
    // Undo the last change of measurement/gradient values.
    void Undo();

    // Description:
    // All parameters of measurement/gradient are restored to original.
    void Restore();

    // Description:
    // Return value is 1 if there is still a node in the stack for undo; otherwise 0.
    int IsUndoable();

    // Description:
    // Return value is 1 if there is still a node in the stack for redo; otherwise 0.
    int IsRedoable();

    void SetActiveVolumeNode(vtkMRMLVolumeNode *node);

  protected:
    vtkSlicerGradientEditorLogic(void);
    virtual ~vtkSlicerGradientEditorLogic(void);

    vtkMRMLDiffusionWeightedVolumeNode *ActiveDWINode;
    vtkMRMLDiffusionTensorVolumeNode *ActiveDTINode;

    // Description:
    // Updates the values of the current ActiveVolumeNode after undo/redo/restore.
    void UpdateActiveVolumeNode(vtkMRMLVolumeNode *node);

    // Description:
    // Stack holds all references of created DWINodes.
    //BTX
    vtkstd::vector<vtkMRMLVolumeNode*> UndoRedoStack;
    //ETX

    // Description:
    // Points to the current node in the UndoRedoStack (node that is displayed in the GUI).
    unsigned int StackPosition;

    // Description:
    // Return value is 1 if editor is in undo mode; otherwise 0.
    int UndoFlag;

  private:
    vtkSlicerGradientEditorLogic (const vtkSlicerGradientEditorLogic&); // Not implemented.
    void operator = (const vtkSlicerGradientEditorLogic&); //Not implemented.
  };
#endif

