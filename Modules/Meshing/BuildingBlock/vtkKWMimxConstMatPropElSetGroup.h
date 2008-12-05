/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxConstMatPropElSetGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.6.4.1 $

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
// .NAME vtkKWMimxConstMatPropElSetGroup - GUI to assign constant material property
// to the chosen element set of a FE mesh.
//
// .SECTION Description
// The GUI consists of the option for choosing the FE mesh, and element set whose
// material property needs to be defined. Window which helps in defining a new element
// set can also be launched (vtkKWMimxDefineElSetGroup). Material property that can
// be specified are Young's modulus and Poisson's ratio. Window containing viewing
// options for material property is also included (vtkKWMimxViewPropertiesOptionGroup).
//
// .SECTION See Also
// vtkKWMimxDefineElSetGroup, vtkKWMimxViewPropertiesOptionGroup

#ifndef __vtkKWMimxConstMatPropElSetGroup_h
#define __vtkKWMimxConstMatPropElSetGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;
class vtkKWMimxDefineElSetGroup;
class vtkKWMimxViewPropertiesOptionGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxConstMatPropElSetGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxConstMatPropElSetGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxConstMatPropElSetGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  
  // Description:
  // Hide the Displayed GUI from screen
  void ConstMatPropElSetCancelCallback();

  // Description:
  // Assign the material properties. If the entry/entries are blank
  // and if the material property already exists, the existing material
  // property is removed.
  int ConstMatPropElSetApplyCallback();

  // Description:
  // Update the FE mesh list and element set list in the corresponding
  // comboboxes.
  void UpdateObjectLists();

  // Description:
  // Change all the relevant change in entries with the change in FE mesh
  // selected.
  void SelectionChangedCallback(const char *Selection);

  // Description:
  // Change all the relevant entries with the change in the element set selected.
  void ElementSetChangedCallback(const char *Selection);

  // Description:
  // Launch a window for element set creation.
  void DefineElementSetCallback( );

protected:
        vtkKWMimxConstMatPropElSetGroup();
        ~vtkKWMimxConstMatPropElSetGroup();
        virtual void CreateWidget();

  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWComboBoxWithLabel *ElementSetComboBox;
  vtkKWEntryWithLabel *YoungsModulusEntry;
  vtkKWEntryWithLabel *PoissonsRatioEntry;
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWPushButton *DefineElSetButton;
  vtkKWPushButton *ViewOptionsButton;

  vtkKWMimxDefineElSetGroup *DefineElementSetDialog;
  vtkKWMimxViewPropertiesOptionGroup *ViewOptionsGroup;

  char elementSetSelectionPrevious[64];

private:
  vtkKWMimxConstMatPropElSetGroup(const vtkKWMimxConstMatPropElSetGroup&); // Not implemented
  void operator=(const vtkKWMimxConstMatPropElSetGroup&); // Not implemented
  
  char meshName[64];
  char elementSetName[64];
};

#endif

