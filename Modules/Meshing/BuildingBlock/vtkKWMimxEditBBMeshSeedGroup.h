/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditBBMeshSeedGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.17.4.1 $

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
// .NAME vtkKWMimxEditBBMeshSeedGroup - GUI to edit mesh seed based on average
// element length or number of divisions.
// .SECTION Description
// GUI contains option for the entries to change the mesh seeds either based on
// average element length or the number of divisions. Option for picking a particular
// block in the structure and changing the mesh seeds also exists. Mesh seed color coding
// could also be displayed.
//
// .SECTION SEE ALSO
// vtkMimxColorCodeMeshSeedActor, vtkMimxPlaceLocalAxesWidget

#ifndef __vtkKWMimxEditBBMeshSeedGroup_h
#define __vtkKWMimxEditBBMeshSeedGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWCheckButtonWithLabel;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWRadioButton;
class vtkKWRadioButtonSet;

class vtkMimxColorCodeMeshSeedActor;
class vtkMimxPlaceLocalAxesWidget;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxEditBBMeshSeedGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditBBMeshSeedGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxEditBBMeshSeedGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);

  // Description:
  // Hide the GUI
  void EditBBMeshSeedCancelCallback();

  // Description:
  // Accept the changes made through the entry widgets
  int EditBBMeshSeedApplyCallback();

  // Description:
  // Activate the pick cell widget
  void EditBBMeshSeedPickCellCallback();

  // Description:
  // Change the chosen building block structure
  void SelectionChangedCallback(const char*);

  /*vtkGetObjectMacro(XMeshSeed, vtkKWEntryWithLabel);
  vtkGetObjectMacro(YMeshSeed, vtkKWEntryWithLabel);
  vtkGetObjectMacro(ZMeshSeed, vtkKWEntryWithLabel);*/
  void UpdateObjectLists();

  // Description:
  // To change the label of entry widget from 'Number of Divisions'
  // to 'Element Length'
  void UserDefinedElementLengthCheckButtonCallback();

  // Description:
  // To change the label of entry widget from 'Element Length'
  // to 'Number of Divisions'
  void NumberOfDivisionsCheckButtonCallback();

  // Description:
  // To display mesh seed color code. 1 yes 0 no
  void ShowMeshSeedCallback(int State);

  // Description:
  // Show the building block structure selected
  void ReturnUnstructuredGridActor(const char *name);
  
  // Description:
  // Reset the state by removing block picking widget
  void EditBBMeshSeedVtkInteractionCallback();
  
  // Description:
  // Reset the state
  void InitializeState( );
protected:
        vtkKWMimxEditBBMeshSeedGroup();
        ~vtkKWMimxEditBBMeshSeedGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkMimxPlaceLocalAxesWidget *LocalAxesWidget;
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWFrameWithLabel *CellSelectionFrame;
  vtkKWFrameWithLabel *MeshSeedTypeFrame;
  vtkKWRadioButton *EditButton;
  vtkKWRadioButton *VtkInteractionButton;
  vtkKWRadioButtonSet *MeshSeedTypeButton;
  vtkKWEntryWithLabel *XMeshSeed;
  vtkKWEntryWithLabel *YMeshSeed;
  vtkKWEntryWithLabel *ZMeshSeed;
  vtkKWEntryWithLabel *AvElementLength;
  vtkKWFrame *ButtonFrame;
  vtkKWFrame *EditTypeChoiceFrame;
  vtkKWRadioButtonSet *AxisSelection;
  vtkKWMenuButtonWithLabel *EditTypeSelection;
  vtkKWEntryWithLabel *UserDefinedElementLength;
  vtkKWCheckButtonWithLabel *ShowMeshSeedCheckButton;
  vtkMimxColorCodeMeshSeedActor *ColorCodeMeshSeedActor;
private:
  vtkKWMimxEditBBMeshSeedGroup(const vtkKWMimxEditBBMeshSeedGroup&); // Not implemented
  void operator=(const vtkKWMimxEditBBMeshSeedGroup&); // Not implemented
  
  char PreviousSelection[64];
  double AverageEdgeLength[3];
};

#endif

