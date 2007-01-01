#ifndef __vtkEMSegmentAnatomicalStructureStep_h
#define __vtkEMSegmentAnatomicalStructureStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWEntryWithLabel;
class vtkKWChangeColorButton;
class vtkKWMenu;
class vtkKWTreeWithScrollbars;
class vtkKWPushButtonSet;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentAnatomicalStructureStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentAnatomicalStructureStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentAnatomicalStructureStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // The anatomical tree is shared between several steps
  vtkGetObjectMacro(AnatomicalStructureTree, vtkKWTreeWithScrollbars);
  virtual void ShowAnatomicalStructureTree();
  virtual void SetAnatomicalTreeNodesSelectableOn();
  virtual void SetAnatomicalTreeLeafNodeSelectableState(
    const char* parent, int state);
  virtual void SetAnatomicalTreeParentNodeSelectableState(
    const char* parent, int state);

  // Description:
  // Callbacks.
  virtual void DisplaySelectedNodeAnatomicalAttributesCallback();
  virtual void PopupNodeContextMenuCallback(const char *node);
  virtual void DeleteNodeCallback(vtkIdType);
  virtual void AddChildNodeCallback(vtkIdType);
  virtual void NodeParentChangedCallback(
    const char*, const char*, const char*);
  virtual void SelectedNodeNameChangedCallback(
    vtkIdType, const char*);
  virtual void SelectedNodeIntensityLabelChangedCallback(
    vtkIdType, int);
  virtual void SelectedNodeColorChangedCallback(
    vtkIdType, double r, double g, double b);
  virtual void OpenTreeCallback();
  virtual void CloseTreeCallback();

protected:
  vtkEMSegmentAnatomicalStructureStep();
  ~vtkEMSegmentAnatomicalStructureStep();

  vtkKWMenu *ContextMenu;

  vtkKWTreeWithScrollbars *AnatomicalStructureTree;
  vtkKWFrameWithLabel     *AnatomicalStructureFrame;
  vtkKWPushButtonSet      *AnatomicalStructureTreeButtonSet;

  vtkKWFrameWithLabel      *AnatomicalNodeAttributesFrame;
  vtkKWEntryWithLabel      *AnatomicalNodeAttributeNameEntry;
  vtkKWEntryWithLabel      *AnatomicalNodeIntensityLabelEntry;
  vtkKWChangeColorButton   *AnatomicalNodeAttributeColorButton;

  // Description:
  // The anatomical tree (common to several steps)
  virtual void PopulateAnatomicalStructureTree(const char*, vtkIdType);

private:
  vtkEMSegmentAnatomicalStructureStep(const vtkEMSegmentAnatomicalStructureStep&);
  void operator=(const vtkEMSegmentAnatomicalStructureStep&);
};

#endif
