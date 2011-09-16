#ifndef __vtkEMSegmentAnatomicalStructureStep_h
#define __vtkEMSegmentAnatomicalStructureStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWEntryWithLabel;
//class vtkKWChangeColorButton;
class vtkKWMenu;
class vtkKWTreeWithScrollbars;
class vtkKWPushButtonSet;
class vtkSlicerNodeSelectorWidget;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWCheckButton;
class vtkKWFrame;
class vtkKWLabelWithLabel;
class vtkKWTopLevel;
class vtkKWPushButton;

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
  virtual void ShowAnatomicalStructureTree(vtkKWFrame * parent = NULL);
  virtual void SetAnatomicalTreeNodesSelectableOn();
  virtual void SetAnatomicalTreeLeafNodeSelectableState(
    const char* parent, int state);
  virtual void SetAnatomicalTreeParentNodeSelectableState(
    const char* parent, int state);

  // Description:
  // Callbacks.
  virtual void SelectedAnatomicalNodeChangedCallback();
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
  void  RemoveAnatomicalStructureTree();
  void PopUpLabelColorSelect();
  void LabelWindowCollapseCallback();

protected:
  vtkEMSegmentAnatomicalStructureStep();
  ~vtkEMSegmentAnatomicalStructureStep();

  vtkKWMenu *ContextMenu;

  vtkKWTreeWithScrollbars *AnatomicalStructureTree;
  vtkKWFrameWithLabel     *AnatomicalStructureFrame;
  vtkKWPushButtonSet      *AnatomicalStructureTreeButtonSet;

  vtkKWFrameWithLabel                *AnatomicalNodeAttributesFrame;
  vtkKWEntryWithLabel                *AnatomicalNodeAttributeNameEntry;
  vtkKWEntryWithLabel                *AnatomicalNodeIntensityLabelEntry;
  vtkKWPushButton                    *AnatomicalNodeAttributeColorButton;
  vtkKWLabelWithLabel                *AnatomicalNodeAttributeColorLabel;
  vtkKWCheckButton                   *ShowOnlyNamedColorsCheckButton;
  vtkKWMultiColumnListWithScrollbars *ColorMultiColumnList;
  int NumberOfColumns;
  bool MultiSelectMode;
  vtkKWFrameWithLabel                *ColormapFrame;
  vtkSlicerNodeSelectorWidget        *ColorSelectorWidget;

  vtkCallbackCommand      *SelectedColorChangedCallbackCommand;
  vtkCallbackCommand      *SelectedColormapChangedCallbackCommand;

  vtkKWTopLevel* LabelTopLevel;
  vtkKWPushButton* LabelApply;
  vtkKWFrame* LabelTopLevelFrame; 

  //BTX
  enum
    {
      EntryColumn = 0,
      NameColumn = 1,
      ColourColumn = 2,
    };
  //ETX
  
  // Description:
  // The anatomical tree (common to several steps)
  virtual void PopulateAnatomicalStructureTree(const char*, vtkIdType);

  static void SelectedColormapChangedCallback(vtkObject *caller,  
                     unsigned long eid, void *clientData, void *callData);
  static void SelectedColorChangedCallback(vtkObject *caller, 
                     unsigned long eid, void *clientData, void *callData);
  
private:
  vtkEMSegmentAnatomicalStructureStep(const vtkEMSegmentAnatomicalStructureStep&);
  void operator=(const vtkEMSegmentAnatomicalStructureStep&);

  void UpdateAnatomicalNodeAttributeColorButton();
  void AddSelectedColorChangedObserver();
  void RemoveSelectedColorChangedObserver();

  // Select a row in ColorMultiColumnList that has the matching entryValue
  // Return -1 with no selection performed if such an entry does not exist.
  int SelectRowByIntensityLabelEntryValue(int entryValue, int columnIndex = EntryColumn);

  // Get the intensity label value of the first selected row in ColorMultiColumnList
  // return -1 if no row has been selected
  int GetIntensityLabelEntryValueOfFirstSelectedRow(int columnIndex = EntryColumn);

  // AnatomicalNodeIntensityLabelEntry and ColorMultiColumnList are sending update
  // messages to each other, we need a lock to prevent endless loop.
  bool LockSelectedColorChangedMessage;

};

#endif
