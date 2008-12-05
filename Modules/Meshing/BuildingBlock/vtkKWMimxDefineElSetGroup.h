/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDefineElSetGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.14.2.1 $

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
// .NAME vtkKWMimxDefineElSetGroup - A pop up window for creation of element sets.
//
// .SECTION Description
// The window contains GUI for element set creation. The mesh from which element sets
// need to be created need to be specified. 5 different element selection types exist.
// 1) Select elements through, 2) Select elements on the surface, 3) Select visible 
// elements, 4) Select elements belonging to a building-block face and 5) Select single
// element. The widget used for the interaction is vtkMimxCreateElementSetWidgetFEMesh.
// Two selection modes exist. Full and Partial which is determined through the opacity
// value of the input mesh. If the opacity value is 1.0 the mode is full mesh, else 
// partial mesh. One more option that exists is Editing mode with add and subtract as
// sub options. In Full mesh, Add mode will add elements chosen to the existing selection
// but for partial mesh, Add mode will retain only the chosen elements and remove other
// elements from selection. In subtract mode, the chosen elements are removed from the 
// selection.
//
// .SECTION See Also
// vtkMimxCreateElementSetWidgetFEMesh

#ifndef __vtkKWMimxDefineElSetGroup_h
#define __vtkKWMimxDefineElSetGroup_h

#include "vtkKWTopLevel.h"
#include "vtkKWComboBox.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkUnstructuredGrid;
class vtkKWCheckButtonSet; 
class vtkKWComboBox;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWPushButton;
class vtkKWRadioButton;
class vtkKWRadioButtonSet;
class vtkKWScaleWithLabel;
class vtkMimxCreateElementSetWidgetFEMesh;
class vtkMimxMeshActor;
class vtkKWMimxCreateFEMeshFromBBGroup;
class vtkKWMimxMainWindow;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxDefineElSetGroup : public vtkKWTopLevel
{
public:
  static vtkKWMimxDefineElSetGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxDefineElSetGroup,vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);
 
  virtual void Update();
  virtual void UpdateEnableState();
 
  // Description:
  // To gain access to the element set combobox. This is used to update
  // element set list when a new element set is created.
  vtkSetObjectMacro(ElementSetCombobox, vtkKWComboBox);

  // Description:
  // To invoke apply push button call back for vtkMimxCreateFEMeshFromBBGroup.
  // This is used in invoking mesh extrusion with right mouse button click.
  void SetCreateFEMeshFromBBGroup(vtkKWMimxCreateFEMeshFromBBGroup *Group);

  vtkGetObjectMacro(SetLabelEntry, vtkKWEntryWithLabel);
  vtkGetObjectMacro(ApplyButton, vtkKWPushButton);
  vtkGetObjectMacro(ExtrudeFrame, vtkKWFrame);
  vtkGetObjectMacro(ModeFrame, vtkKWFrame);

  // Description:
  // Setting the dimension state. 0 implies a volumetric mesh and 1 implies surface mesh.
  vtkSetMacro(DimensionState, int);
  vtkSetMacro(WithdrawOnApply, int);
  vtkGetMacro(WithdrawOnApply, int);

  // Description:
  // Set main window which contains all the GUI. To access renderer so as to update the 
  // display.
  void SetMimxMainWindow(vtkKWMimxMainWindow *window);
  vtkKWMimxMainWindow *GetMimxMainWindow( );

  // Description:
  // To get the selected mesh. Used to create a mesh from surface extrusion.
  vtkUnstructuredGrid* GetSelectedMesh();

  // Description:
  // Call backs for different types of element selection.
  void SelectElementsThroughCallback(int mode);
  void SelectElementsSurfaceCallback(int mode);
  void SelectVisibleElementsSurfaceCallback(int mode);
  void SelectSingleElementCallback(int mode);
  void SelectMultipleElementsCallback(int mode);

  // Description:
  // Clear all the selection made.
  void RemoveSelectionsCallback( );

  // Description:
  // Enable/Disable element selection. mode represents the element selection mode.
  // 0 selection through, 1 surface elements, 2 visible elements, 3 elements belonging
  // to a face of a building block and 4 single element.
  int EnableElementSelection(int mode);
  void DisableElementSelection(int mode);

  // Description:
  // Get the input.
  vtkMimxMeshActor* GetSelectedObject();

  // Description:
  // Set the input.
  void SetMeshActor( vtkMimxMeshActor *actor );

  // Description:
  // Set the widget boolean state to add.
  void AddElementsCallback();

  // Description:
  // Set the widget boolean state to subtract.
  void SubtractElementsCallback();

  // Description:
  // Cancel button call back to close the window.
  void CancelButtonCallback();

  // Description:
  // Apply button callback for creation of element set
  int ApplyButtonCallback();

  // Description:
  // Super class method to close the window and deselect all
  // the buttons selected.
  virtual void Withdraw();

  // Description:
  // To set the opacity value of the mesh and subsequently set the
  // editing mode of the widget.
  void OpacityCallback(double Val);

  // Description:
  // To reset the state. Used when the selections are cleared or when
  // the window is closed.
  void ResetState();

  // Description:
  // Since the window is used in element selection of volumetric as well
  // as surface mesh, need to set separate dialog based on what it is being
  // used for.
  void SetDialogTitle(const char *title);

  // Description:
  // If the mode is set to surface only, the through element selection is disabled.
  void SetModeSurfaceOnly();

  // Description:
  // Set the mode to any type of mesh.
  void SetModeAll();

  // Description:
  // Number of divisions along the thickness of extrusion. Used in creation of FE mesh
  // through surface extrusion.
  int GetNumberOfDivisions();

  // Description:
  // Extrusion length.
  double GetExtrusionLength();
protected:
        vtkKWMimxDefineElSetGroup();
        ~vtkKWMimxDefineElSetGroup();
        virtual void CreateWidget();
  vtkKWMimxMainWindow *MimxMainWindow;
  
  vtkKWCheckButtonSet *SelectButtonSet;
  vtkKWEntryWithLabel *SetLabelEntry;
  vtkKWPushButton *CancelButton;
  vtkKWPushButton *ApplyButton;
  vtkKWFrame *ButtonFrame;
  vtkKWFrame *ModeFrame;
  vtkKWFrame *OperationFrame;
  vtkKWPushButton *ClearSelectionButton;
  vtkMimxCreateElementSetWidgetFEMesh *SelectCellsWidget;
  vtkKWComboBox *ElementSetCombobox;
  vtkKWScaleWithLabel *OpacityScale;
  vtkKWFrame *ExtrudeFrame;
  vtkKWEntryWithLabel *ExtrudeLengthEntry;
  vtkKWEntryWithLabel *NumberOfDivisionsEntry;
  vtkKWMimxCreateFEMeshFromBBGroup *CreateFEMeshFromBBGroup;
  vtkKWRadioButtonSet *AddSubtractButton;
private:
  vtkKWMimxDefineElSetGroup(const vtkKWMimxDefineElSetGroup&); // Not implemented
  void operator=(const vtkKWMimxDefineElSetGroup&); // Not implemented
  
  int SelectionState;
  int DimensionState;
  bool SurfaceOnlyMode;
  int WithdrawOnApply;
  vtkMimxMeshActor *MeshActor;  
};

#endif

