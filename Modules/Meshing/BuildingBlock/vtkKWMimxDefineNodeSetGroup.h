/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDefineNodeSetGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.5.4.1 $

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
// .NAME vtkKWMimxDefineNodeSetGroup - A pop up window for creation of node sets.
//
// .SECTION Description
// The window contains GUI for node set creation. The mesh from which node sets
// need to be created need to be specified. 5 different node selection types exist.
// 1) Select nodes through, 2) Select nodes on the surface, 3) Select visible 
// nodes, 4) Select nodes belonging to a building-block face and 5) Select single
// node. The widget used for the interaction is vtkMimxSelectPointsWidget.
// Two selection modes exist. Full and Partial which is determined through the opacity
// value of the input mesh. If the opacity value is 1.0 the mode is full mesh, else 
// partial mesh. One more option that exists is Editing mode with add and subtract as
// sub options. In Full mesh, Add mode will add nodes chosen to the existing selection
// but for partial mesh, Add mode will retain only the chosen nodes and remove other
// nodes from selection. In subtract mode, the chosen nodes are removed from the 
// selection.
//
// .SECTION See Also
// vtkMimxSelectPointsWidget

#ifndef __vtkKWMimxDefineNodeSetGroup_h
#define __vtkKWMimxDefineNodeSetGroup_h

#include "vtkKWTopLevel.h"
#include "vtkKWComboBox.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWCheckButtonSet; 
class vtkKWComboBox;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWPushButton;
class vtkKWPushButtonSet;
class vtkKWRadioButtonSet;
class vtkKWScaleWithLabel;
class vtkMimxMeshActor;
class vtkMimxSelectPointsWidget;
class vtkLinkedListWrapper;
class vtkKWMimxMainWindow;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxDefineNodeSetGroup : public vtkKWTopLevel
{
public:
  static vtkKWMimxDefineNodeSetGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxDefineNodeSetGroup,vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Access to the combobox which contains node set names.
  // Used to update the combobox entries when a new node set is created.
  vtkSetObjectMacro(NodeSetCombobox, vtkKWComboBox);

  // Description:
  // Access to main window to update the display.
  void SetMimxMainWindow(vtkKWMimxMainWindow *window);
  vtkKWMimxMainWindow *GetMimxMainWindow( );
  
  // Description:
  // Node selection callbacks. mode 0 to disable and mode 1 enable
  // the selection.
  void SelectNodesThroughCallback(int mode);
  void SelectNodesSurfaceCallback(int mode);
  void SelectVisibleNodesSurfaceCallback(int mode);
  void SelectFaceNodeCallback(int mode);
  void SelectSingleNodeCallback(int mode);

  // Description:
  // Disable node selection. 
  // Due to the change in how the function behaves, mode has no influence on how 
  // the function behaves, hence should be removed.
  void DisableNodeSelection(int mode);

  // Description:
  // Enable/Disable node selection. mode represents the node selection mode.
  // 0 selection through, 1 surface nodes, 2 visible nodes, 3 nodes belonging
  // to a face of a building block and 4 single node.
  int  EnableNodeSelection(int mode);
  
  // Description:
  // To generate node set after the selection is made.
  int  ApplyButtonCallback();

  // Description:
  // Mesh from which node set need to be created.
  void SetMeshActor( vtkMimxMeshActor *actor );

  // Description:
  // To change the opacity so as to change the edit mode.
  void OpacityCallback(double Val);

  // Description:
  // Set the edit mode to add
  void AddNodesCallback();

  // Description:
  // Set the edit mode to subtract.
  void SubtractNodesCallback();

  // Description:
  // To close the window.
  virtual void Withdraw();

  // Description:
  // Clear all the selections from the screen.
  void RemoveSelectionsCallback();

  // Description:
  // Deselect any selection mode chosen and reset the 
  // buttons in the pop-up window.
  void ResetState();
protected:
        vtkKWMimxDefineNodeSetGroup();
        ~vtkKWMimxDefineNodeSetGroup();
        virtual void CreateWidget();
        vtkMimxMeshActor* GetSelectedObject();
        
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkLinkedListWrapper *ObjectList;
  
  vtkKWCheckButtonSet *SelectButtonSet;
  vtkKWEntryWithLabel *SetLabelEntry;
  vtkKWPushButton *CancelButton;
  vtkKWPushButton *ApplyButton;
  vtkKWFrame *ButtonFrame;
  vtkKWFrame *ModeFrame;
  vtkKWFrame *OperationFrame;
  vtkMimxSelectPointsWidget *SelectPointsWidget;
  vtkKWScaleWithLabel *OpacityScale;
  vtkKWRadioButtonSet *AddSubtractButton;
  vtkKWPushButton *ClearSelectionButton;
  vtkKWComboBox *NodeSetCombobox;
private:
  vtkKWMimxDefineNodeSetGroup(const vtkKWMimxDefineNodeSetGroup&); // Not implemented
  void operator=(const vtkKWMimxDefineNodeSetGroup&); // Not implemented
  
  int SelectionState;
  vtkMimxMeshActor *MeshActor;  
};
 
#endif

