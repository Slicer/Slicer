// .NAME vtkSlicerDiffusionEditorLogic 
// .SECTION Description
// This class implements Slicer's DiffusionEditor Logic, inherits most behavior from 
// vtkSlicerLogic. This class manages the logic associated with loading gradients from files 
// and testing given gradients for validity. It also holds the logic for the undo/redo/restore 
// mechanism of the DiffusionEditor.
#ifndef __vtkSlicerDiffusionEditorLogic_h
#define __vtkSlicerDiffusionEditorLogic_h

#include "vtkVolumes.h"
#include "vtkSlicerLogic.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include <vtkstd/vector>

class vtkDoubleArray;

class VTK_VOLUMES_EXPORT vtkSlicerDiffusionEditorLogic : public vtkSlicerLogic 
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerDiffusionEditorLogic *New();
    vtkTypeRevisionMacro(vtkSlicerDiffusionEditorLogic, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Adds gradients from a file to the GUI. Filetypes are restricted to .txt/.nhdr.
    // Return value is 0 if file contains invalid values; otherwise 1.
    int AddGradients(const char *filename, int numberOfGradients, vtkDoubleArray *newBValue, 
      vtkDoubleArray *newGradients);

    // Description:
    // Parses given ASCII gradients back into arrays of gradients and bValues.
    // Return value is 0 if given gradients can't be parsed or contain invalid/not enough
    // values; otherwise 1.
    int ParseGradientsBvaluesToArray(const char  *gradients, int numberOfGradients, vtkDoubleArray *newBValues, 
      vtkDoubleArray *newGradients);

    //BTX
    // Description:
    // Parses given gradients and bValues back into a string and returns it.
    std::string ParseGradientsBvaluesToString(vtkDoubleArray *BValues, vtkDoubleArray *Gradients);
    //ETX

    // Description:
    // Pushes the current ActiveVolumeNode into the UndoRedoStack, makes a backup 
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

    // Description:
    // Sets the ActiveVolumeNode; can be DWI or DTI.
    void SetActiveVolumeNode(vtkMRMLVolumeNode *node);

  protected:
    vtkSlicerDiffusionEditorLogic(void);
    virtual ~vtkSlicerDiffusionEditorLogic(void);

    // Description:
    // Updates the values of the current ActiveVolumeNode after undo/redo/restore.
    void UpdateActiveVolumeNode(vtkMRMLVolumeNode *node);

    // Description:
    // Clears stack from all saved nodes.
    void ClearStack();

    //BTX
    // Description:
    // Stack holds all references of created nodes.
    vtkstd::vector<vtkMRMLVolumeNode*> UndoRedoStack;

    // Description:
    // Parses a string into a double value.
    // Return value is 0 if parsing was not successful; otherwise 1.
    int StringToDouble(const std::string &s, double &result);
    //ETX

    // Description:
    // Points to the current node in the UndoRedoStack (node that is displayed in the GUI).
    unsigned int StackPosition;

    // Description:
    // Return value is 1 if editor is in undo mode; otherwise 0.
    int UndoFlag;

    vtkMRMLDiffusionWeightedVolumeNode *ActiveDWINode;
    vtkMRMLDiffusionTensorVolumeNode *ActiveDTINode;

  private:
    vtkSlicerDiffusionEditorLogic (const vtkSlicerDiffusionEditorLogic&); // Not implemented.
    void operator = (const vtkSlicerDiffusionEditorLogic&); //Not implemented.
  };
#endif

