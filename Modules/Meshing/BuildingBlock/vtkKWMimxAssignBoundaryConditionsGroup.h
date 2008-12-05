/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxAssignBoundaryConditionsGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.25.4.1 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .NAME vtkKWMimxAssignBoundaryConditionsGroup - Assign boundary conditions menu group.
// .SECTION Description
// Menu group for boundary condition assignment, display, node set creation and 
// boundary condition viewing. Boundary condition steps are depicted as pages of notebook.
// The four boundary conditions considered are Displacement, Force, Rotation and Moment.
// Individual boundary conditions in a given step can be edited, deleted and selectively
// displayed. The boundary condition representations can be scaled for better viewing.

#ifndef __vtkKWMimxAssignBoundaryConditionsGroup_h
#define __vtkKWMimxAssignBoundaryConditionsGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkActorCollection;
class vtkCollection;
class vtkGlyph3D;
class vtkStringArray;
class vtkKWCheckButtonWithLabel;
class vtkKWComboBox;
class vtkKWComboBoxWithLabel;
class vtkKWEntry;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWFrameWithLabel;
class vtkKWLabel;
class vtkKWMenuButton;
class vtkKWNotebook;
class vtkKWScale;
class vtkKWSpinBoxWithLabel;
class vtkKWTopLevel;
class vtkKWTreeWithScrollbars;
class vtkMimxBoundaryConditionActor;
class vtkKWMimxDefineNodeSetGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxAssignBoundaryConditionsGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxAssignBoundaryConditionsGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxAssignBoundaryConditionsGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Updating the group when the group is redrawn
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Callback to close the group
  void AssignBoundaryConditionsCancelCallback();

  // Description:
  // Apply changes to the relevant boundary condition
  int AssignBoundaryConditionsApplyCallback();

  // Description:
  // Update the entries of the object list (list of meshes and corresponding node sets
  void UpdateObjectLists();

  // Description:
  // Update all the required entries when the selected mesh is changed in the combo-box
  void SelectionChangedCallback(const char *Selection);

  // Description:
  // Update the entries with the changed node set
  void NodeSetChangedCallback(const char *Selection);

  // Description:
  // Update the boundary condition value display if existant
  void BoundaryConditionTypeSelectionChangedCallback(const char *Selection);

  // Description:
  // Boundary condition steps are depicted as pages  of a note book with 
  // each page corresponding to a step. When a page is selected, all the 
  // information related to corresponding step are updated for display.
  void StepNumberChangedCallback(const char *StepNum);

  // Description:
  // Add a new step. The last step should not be empty (i.e. it should have
  // atleast one boundary condition associated with it.
  void AddStepNumberCallback();

  // Description:
  // To display boundary conditions or not. (Legacy code should be removed).
  void ViewBoundaryConditionsCallback(int Mode);

  // Description:
  // Delete a boundary condition step and the associated notebook page.
  // All the field data associated with the mesh are deleted.
  void DeleteStepNumberCallback();

  // Description:
  // All the boundary conditions are depicted as arrows except for fixed
  // 'Displacement'which is depicted as a cone. The boundary condition 
  // representations are collectively called as glyphs. The normalized
  // Glyph size is calculated based on average edge length of the mesh 
  // and individual glyphs are sized based on the magnitude of the boundary
  // for a given step, given boundary condition type.
  void GlyphSizeChangedCallback(const char *Entry);

  // Description:
  // Get the active note book page number or step number.
  int  GetNotebookStep();

  // Description:
  // To define a new node set. The function launches a pop-up window from which
  // node set selection can be made
  void DefineNodeSetCallback();

  // Description:
  // Display the summary of boundary conditions available. The BC data will be in
  // the form of a tree structure.
  void SummarizeCallback();

  // Description:
  // To close the dialog which contains all the information for a given step
  // such as BC output to be printed, step increment etc.
  void DialogCancelCallback();

  // Description:
  // Close the summary dialog pop-up window.
  void SummaryDialogCancelCallback();

  // Description:
  // Update the visibility of the chosen BC. 0 hide and 1 show.
  void UpdateVisibility(int Mode);

  // Description:
  // Fill the multicolumn list table with the BC's available.
  void UpdateBCVisibilityList( );

  // Description:
  // Change the opacity of the mesh being displayed
  void OpacityCallback(double Val);

  // Description:
  // To launch a pop-up window which contains step details pertaining to step increment
  // output variables of interest etc.
  void StepControlCallback();

  // Description:
  // To enable/disable the node related variables setting options.
  void NodePrintCallback(int mode);

  // Description:
  // To enable/disable the node output entry widgets.
  void NodeOutputCallback(int mode);

  // Description:
  // To enable/disable the element related variables setting options.
  void ElementPrintCallback(int mode);

  // Description:
  // To enable/disable the element output entry widgets.
  void ElementOutputCallback(int mode);

  // Description:
  // Store all the step related entries.
  void DialogApplyCallback();

  // Description:
  // Check if the entry in the combo-box is a valid entry.
  vtkUnstructuredGrid* IsMeshValid();

  // Description:
  // To store the changes made for the node output variables.
  void NodeSetOutputApplyCallback();

  // Description:
  // To store the changes made for element output variables.
  void ElementSetOutputApplyCallback();

  // Description:
  // If this is the first step set the default step parameters
  // else copy the parameters from the previous step.
  void GenerateStepParameters(int StepNum);

  // Description:
  // If the parameter values are already present for a given step number
  // assign the values to the widgets.
  void AssignValuesToStepWindow(int StepNum);

  // Description:
  // Get the string array which contains the lower step parameters
  vtkStringArray* GetLowerStepWithParameters(int StepNum, vtkUnstructuredGrid *ugrid);

  // Description:
  // Get the output parameters for the latest element set and update the widgets
  void OutputElementSetChangedCallback(const char *Entry);

  // Description:
  // Get the output parameters for the latest node set and update the widgets
  void OutputNodeSetChangedCallback(const char *Entry);

  // Description:
  // Copy previous step node and element output parameters. StepNum is the current
  // step number.
  void CopyLowerStepNodeAndElementOutput(int StepNum, vtkUnstructuredGrid *Ugrid);
protected:
   vtkKWMimxAssignBoundaryConditionsGroup();
   ~vtkKWMimxAssignBoundaryConditionsGroup();

   // Description:
   // Legacy code, to be deleted
   int GetValue();

   // Description:
    // Get the BC value for the Direction (X,Y or Z), type of boundary condition
    // and the node sent name chosen.
    double GetValue(int Direction, const char *BoundaryCondition, const char *NodeSetName);

    virtual void CreateWidget();

    // Description:
    // command for the change in the page picked.
    virtual void ProcessCallbackCommandEvents(
    vtkObject *caller, unsigned long event, void *calldata);
    
    vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWPushButton *DefineNodeSetPushButton;
  vtkKWNotebook *StepNotebook;
  vtkKWComboBox *NodeSetComboBox;
  vtkKWMenuButton *BoundaryConditionTypeComboBox;
  vtkKWFrame *StepFrame;
  vtkKWFrame *DirectionFrame;
  vtkKWEntryWithLabel *DirectionXEntry;
  vtkKWEntryWithLabel *DirectionYEntry;
  vtkKWEntryWithLabel *DirectionZEntry;
  vtkKWEntryWithLabel *GlyphSizeEntry;
  vtkKWComboBoxWithLabel *StepNumberComboBox;
  vtkKWPushButton *AddStepPushButton;
  vtkKWPushButton *DeleteStepPushButton;
  vtkKWCheckButtonWithLabel *ViewBoundaryConditionsButton;
  vtkKWPushButton *SummaryButton;
  vtkKWMimxDefineNodeSetGroup *DefineNodeSetDialog;
  vtkKWTopLevel *SummaryDialog;
  vtkKWTreeWithScrollbars *BCTree;
  vtkKWPushButton *CancelDialogButton;
  vtkKWFrame *InfoFrame;
  vtkKWFrame *ButtonFrame;
  vtkKWLabel *NodeSetLabel;
  vtkKWLabel *SubHeadingLabel;
  vtkKWEntry *SubHeadingEntry;
  vtkKWFrame *SubHeadingFrame;
  
  vtkKWTopLevel *StepControlDialog;
  vtkKWLabel *StepParameterLabel;
  vtkKWFrameWithLabel *StepParameterFrame;
  vtkKWEntryWithLabel *StepIncrementEntry;
  vtkKWCheckButtonWithLabel *NlGeomButton;
  vtkKWCheckButtonWithLabel *UnsymmButton;
  vtkKWMenuButton *AnalysisTypeMenu;
  vtkKWEntryWithLabel *TimeIncrementEntry;
  vtkKWCheckButtonWithLabel *NodePrintButton;
  vtkKWFrameWithLabel *NodePrintFrame;
  vtkKWComboBoxWithLabel *NodeSetMenu;
  vtkKWEntryWithLabel *NodePrintFrequencyEntry;
  vtkKWCheckButtonWithLabel *NodePrintSummaryButton;
  vtkKWCheckButtonWithLabel *NodePrintTotalButton;
  vtkKWFrameWithLabel *ElementPrintFrame;
  vtkKWCheckButtonWithLabel *ElementPrintButton;
  vtkKWComboBoxWithLabel *ElementSetMenu;
  vtkKWMenuButtonWithLabel *ElementPositionSetMenu;
  vtkKWEntryWithLabel *ElementPrintFrequencyEntry;
  vtkKWMenuButtonWithLabel *ElementPrintPositionMenu;
  vtkKWCheckButtonWithLabel *ElementPrintSummaryButton;
  vtkKWCheckButtonWithLabel *ElementPrintTotalButton;
  vtkKWCheckButtonWithLabel *NodeOutputButton;
  vtkKWCheckButtonWithLabel *ElementOutputButton;
  vtkKWMenuButtonWithLabel *ElementOutputPositionMenu;
  vtkKWPushButton *StepControlApplyButton;
  vtkKWPushButton *NodeSetOutputApplyButton;
  vtkKWPushButton *ElementSetOutputApplyButton;
  vtkKWPushButton *StepControlCancelButton;
  vtkKWPushButton *DefineControlPushButton;
  vtkKWEntryWithLabel *NodePrintVariablesEntry;
  vtkKWEntryWithLabel *NodeOutputVariablesEntry;
  vtkKWEntryWithLabel *ElementPrintVariablesEntry;
  vtkKWEntryWithLabel *ElementOutputVariablesEntry;
  vtkKWMenuButtonWithLabel *AmplitudeTypeMenu;
  
  // Description:
  // To concatenate strings. Should be replaced with printf statements
  void ConcatenateStrings(const char*, const char*, 
                          const char*, const char*, const char*, char*);

  // Description:
  // Check if step is empty(0) or not(1).
  int IsStepEmpty(vtkUnstructuredGrid *ugrid);

  // Description:
  // Delete all the boundary condition actor. (this is used when the step number is changed)
  void DeleteBoundaryConditionActors();

  // Description:
  // Create boundary condition actors. (this is used when the step number is changed)
  void CreateBoundaryConditionActors();

  // Description:
  // Modify the actors when the BC magnitudes change
  void ModifyBoundaryConditionActors();

  // Description:
  // Create, Delete and Modify BC actors
  void CreateBoundaryConditionActor(const char *NodeSetName, const char *BoundaryConditionType);
  void DeleteBoundaryConditionActor(const char *NodeSetName, const char *BoundaryConditionType);
  void ModifyBoundaryConditionActor(const char *NodeSetName, const char *BoundaryConditionType);


  int DoesBoundaryConditionExist(const char *NodeSetName, const char *BoundaryConditionType);

  // Description:
  // Show or hide a given BC. ConditionNum is the order in which the BC are stored in the
  // multicolumnlist that is used for display.
  void ShowBoundaryConditionRepresentation(int ConditionNum);
  void HideBoundaryConditionRepresentation(int ConditionNum);

  // Description:
  // Function to compute the BC representation (both the glyphs (arrows or cone) and their size)
  void ComputeBoundaryCondRepresentation( const char*, const char*, double xValue, 
      double yValue, double zValue);

  int CancelStatus;
  vtkCollection *ActorCollection;
  vtkCollection *GlyphCollection;
  vtkKWFrame *GlyphFrame;
  vtkKWScale *MeshOpacityScale;
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  vtkKWSpinBoxWithLabel *OpacityThumbwheel;
  vtkKWFrameWithLabel *ViewFrame;
  double GlyphSize;
  char nodeSetSelectionPrevious[256];
  char stepNumPrevious[64];
  char boundaryConditionTypePrevious[64];
  double sizePrevious;
  double factorX;
  double factorY;
  double factorZ;
  vtkStringArray *NodeSetNameArray;
  vtkStringArray *BoundaryConditionTypeArray;
  vtkStringArray *BoundaryConditionArray;
private:
  vtkKWMimxAssignBoundaryConditionsGroup(const vtkKWMimxAssignBoundaryConditionsGroup&); // Not implemented
  void operator=(const vtkKWMimxAssignBoundaryConditionsGroup&); // Not implemented
  
  vtkMimxBoundaryConditionActor *BoundaryConditionActor;
  int startNotebookPage;
  int endNotebookPage;
  int currentNotebookPage;
};

#endif

