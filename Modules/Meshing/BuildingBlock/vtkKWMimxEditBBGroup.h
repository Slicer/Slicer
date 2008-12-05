/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditBBGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.37.4.3 $

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
// .NAME vtkKWMimxEditBBGroup - GUI class for building block editing operations.
// .SECTION Description
// The class contains GUI for building block editing. Editing operations incorporated
// are 0) Move vertices, edges and faces 1) Split and edge, 2) Add a single/multiple
// building blocks by selecting faces, 3) Delete a single/multiple building blocks by
// selecting the blocks, 4) Consolidate multiple building block structures into one building
// block structure, 5) Mirror a building block structure about any arbitrary plane and
// 6) Merge vertices lying within specified tolerance. The numbers correspond to editing
// mode used in the class.
// A part of the building block structure could be selected to be edited upon, but the changes
// are reflected in the complete building block structure. The editing operations in a given 
// session are stored in a linked list so the Do and UnDo functionality is available for traversing
// through editing operations. Many widgets are used to incorporate the editing interaction and are
// listed in the below.
// vtkMimxUnstructuredGridWidget - For movement of vertices, edges and faces of the building block
// vtkMimxExtractEdgeWidget - To select the edge required
// vtkMimxExtractFaceWidget - To select single/multiple faces
// vtkMimxExtractCellWidget - To select single/multiple faces
// vtkMimxSelectCellsWidget - To select portion of building block structure
// vtkMimxViewMergePointsWidget - To merge vertices in a single building block structure or 
// consolidate multiple building block structures into one structure.
// 
// .SECTION SEE ALSO
// vtkMimxUnstructuredGridWidget, vtkMimxExtractEdgeWidget, vtkMimxExtractFaceWidget,
// vtkMimxExtractCellWidget, vtkMimxSelectCellsWidget, vtkMimxViewMergePointsWidget


#ifndef __vtkKWMimxEditBBGroup_h
#define __vtkKWMimxEditBBGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkDataSetCollection;
class vtkPlaneWidget;
class vtkKWCheckButton;
class vtkKWCheckButtonSet;
class vtkKWCheckButtonWithLabel;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButtonSet;
class vtkMimxExtractCellWidget;
class vtkMimxExtractEdgeWidget;
class vtkMimxExtractFaceWidget;
class vtkMimxSelectCellsWidget;
class vtkMimxUnstructuredGridWidget;
class vtkMimxViewMergePointsWidget;
class vtkKWMimxMergeBBGroup;
class vtkKWMimxMirrorBBGroup;
class vtkRightButtonPressEventCallback;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxEditBBGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditBBGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxEditBBGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Get/Set MimxMainWindow to access the renderwindow.
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);

  vtkGetObjectMacro(ObjectListComboBox, vtkKWComboBoxWithLabel);
  /*vtkGetObjectMacro(DoUndoButtonSet, vtkKWPushButtonSet);*/

  // Description:
  // Callback to set editing mode to add building blocks
  void EditBBAddCellCallback(int Mode);

  // Description:
  // Callback to set editing mode to split building block edge
  void EditBBSplitCellCallback(int Mode);

  // Description:
  // Callback to set editing mode to delete building blocks
  void EditBBDeleteCellCallback(int Mode);

  // Description:
  // Callback to hide the GUI
  void EditBBCancelCallback();

  // Description:
  // Callback to apply the building block edit selections made
  int EditBBApplyCallback();

  // Description:
  // Callback to set editing mode to move vertices, edges and faces
  void EditBBMoveCellCallback(int Mode);

  // Description:
  // Callback to set editing mode to mirror building block structure
  void EditBBMirrorCallback(int Mode);

  // Description:
  // Callback to set editing mode to merge vertices
  void EditBBMergeCallback(int Mode);

  // Description:
  // Callback to set editing mode to consolidate building blocks
  void EditBBConsolidateBlockCallback(int Mode);

  // Description:
  // Callback to reset the modes when building block selection changes
  void BBSelectionChangedCallback(const char*);

  // Description:
  // Update the building block list in the combobox
  void UpdateObjectLists();

  // Description:
  // Callback for do button. The next building block structure in the edit
  // pipeline is shown hiding the current structure
  void DoBBCallback();

  // Description:
  // Callback for undo button. The previous building block structure in the edit
  // pipeline is shown hiding the current structure
  void UndoBBCallback();

  // Description:
  // Callback for full set or subset mode. 0 - Show full set, 1 - Activate the widget
  // used to select a sub-structure
  void SelectFullSetCallback(int mode);

  // Description:
  // Callback for subset selection
  void SelectSubsetCallback();

  // Description:
  // Add newly created building block structure to the list and the combobox
  void AddEditedBB(int BBNum, vtkUnstructuredGrid *output, 
          const char* name, vtkIdType& count, const char *FoundationName);

  // Description:
  // To deselect all mode selection state (for resetting the selection mode to none)
  void DeselectAllButtons();

  // Description:
  // To set the state of Do and UnDo. The Do button is enabled if there is a building
  // block structure next to the current structure in edit pipeline else disabled. 
  // The UnDo button is enabled if there is a previous structure in the edit pipeline
  // else disabled.
  void SetDoUndoButtonSelectSubsetButton();

  // Description:
  // Based on the editing mode operations pack/unpack the handle size entry widget
  // and tolerance entry widget. The modes are as described in the description.
  void RepackEntryFrame(int type);

  // Description:
  // Show the GUI used in mirroring. The mirroring plane can be placed in XY, YZ or
  // ZX plane by default or can be oriented arbitrarily.
  void RepackMirrorFrame();

  // Description:
  // Show the GUI containing the merge frame. It contains multicolumnlist with the
  // list of building blocks that could be selected to be consolidated.
  void RepackMergeFrame();

  // Description:
  // Frame containing entry to set the handle size.
  void RepackSphereSizeFrame();

  // Description:
  // Callback to either display or not the merge feed back widget.
  void ViewMergeFeedbackCallback(int Mode);

  // Description:
  // Callback to change the merge tolerance.  
  void MergeToleranceChangeCallback(const char *Tol);

  // Description:
  // Callback to change the handle size.
  void SphereSizeChangeCallback(const char *Radius);

  // Description:
  // Radiomode is the editing mode (see the class description for details)
  // and structure mode is either full (0) or partial (1).
  void GetUserInterfaceMode(int &radioMode, int &structureMode );
  
  // Description:
  // Place mirroring plane
  void PlaceMirroringPlaneAboutX();
  void PlaceMirroringPlaneAboutY();
  void PlaceMirroringPlaneAboutZ();
  void PlaceMirroringPlane();

protected:
        vtkKWMimxEditBBGroup();
        ~vtkKWMimxEditBBGroup();
        virtual void CreateWidget();
  
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkKWCheckButtonSet *RadioButtonSet;
  vtkKWPushButtonSet *DoUndoButtonSet;
  vtkKWCheckButton *SelectSubsetButton;
  vtkKWFrame *ButtonFrame;
  vtkKWFrame *EntryFrame;
  vtkMimxUnstructuredGridWidget *UnstructuredGridWidget;
  vtkMimxExtractEdgeWidget *ExtractEdgeWidget;
  vtkMimxExtractFaceWidget *ExtractFaceWidget;
  vtkMimxExtractCellWidget *ExtractCellWidget;
  vtkMimxExtractCellWidget *ExtractCellWidgetHBB;
  vtkIdType AddButtonState;
  vtkIdType DeleteButtonState;
  vtkIdType MoveButtonState;
  vtkIdType RegularButtonState;
  vtkIdType SplitButtonState;
  vtkIdType MirrorButtonState;
  vtkIdType ConvertToHBBButtonState;
//  vtkIdType SelectionState;
  vtkIdType SplitCount;
  vtkIdType AddCount;
  vtkIdType DeleteCount;
  vtkIdType MirrorCount;
  vtkIdType ConvertToHBBCount;
  vtkIdType MergeCount;
  vtkIdType CancelStatus;
  vtkKWEntryWithLabel *RadiusEntry;
  vtkKWEntryWithLabel *SphereSizeEntry;
  vtkKWEntryWithLabel *ConsolidateSphereSizeEntry;
  vtkKWFrame *ConsolidateSphereSizeFrame;
  vtkKWFrame *SphereSizeFrame;
  vtkKWEntryWithLabel *ExtrudeEntry;
  vtkKWEntryWithLabel *ToleranceEntry;
 // vtkKWFrameWithLabel *EditButtonFrame;
  vtkKWMimxMergeBBGroup *MergeBBGroup;
  vtkKWMimxMirrorBBGroup *MirrorBBGroup;
  vtkMimxSelectCellsWidget *SelectCellsWidget;

  vtkKWFrameWithLabel *MirrorFrame;
  vtkKWCheckButtonWithLabel *TypeOfMirroring;
  vtkKWPushButtonSet *AxisSelection;
  vtkPlaneWidget *MirrorPlaneWidget;
  
  vtkKWFrameWithLabel *MergeFrame;
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  vtkKWEntryWithLabel *MergeTolerence;
  vtkKWCheckButtonWithLabel *MergeFeedBackButton;
  vtkMimxViewMergePointsWidget *ViewMergePointsWidget;
  vtkRightButtonPressEventCallback *RightButtonPressCallback;
  double defaultRadiusEntry;
  double defaultExtrusionLength;
  double defaultMergeTolerance;
  double Parameter;
  double OriginalRadius;
  vtkDataSetCollection *DataSetCollection;
  double SphereSizeFactor;
  double defaultSphereSizeEntry;
private:
  vtkKWMimxEditBBGroup(const vtkKWMimxEditBBGroup&); // Not implemented
  void operator=(const vtkKWMimxEditBBGroup&); // Not implemented
 };

#endif

