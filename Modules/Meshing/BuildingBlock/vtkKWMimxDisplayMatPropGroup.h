/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDisplayMatPropGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.4.4.1 $

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
// .NAME vtkKWMimxDisplayMatPropGroup - Class containing GUI to display material
// property assigned to the displayed mesh and the element set.
//
// .SECTION Description
// The class contains two comboboxes containing the names of the meshes and the element
// set contained in the chosen mesh. The class also contains an instance of 
// vtkKWMimxViewPropertiesOptionGroup to control the display parameters.
//
// .SECTION See Also
// vtkKWMimxViewPropertiesOptionGroup

#ifndef __vtkKWMimxDisplayMatPropGroup_h
#define __vtkKWMimxDisplayMatPropGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkKWCheckButton;
class vtkKWCheckButtonWithLabel;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;

class vtkKWMimxViewPropertiesOptionGroup;


class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxDisplayMatPropGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxDisplayMatPropGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxDisplayMatPropGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Update the list of FE mesh list and associated element set list
  void UpdateObjectLists();
  
  // Description:
  // Invert the cutting plane. mode 0 do not invert and 1 invert.
  void InvertPlaneCallback(int mode);

  // Description:
  // Display/hide clipping plane. mode 0 display, mode 1 hide.
  void ClippingPlaneCallback(int mode);

  // Description:
  // Display/Hide property legend. mode = 0, hide and 1 show 
  void ViewPropertyLegendCallback( int mode );
  
  // Description:
  // Hide the GUI from display.
  void DisplayCancelCallback();

  // Description:
  // FE mesh selection changed call back. To change the element set list
  // and corresponding display on the window.
  void SelectionChangedCallback(const char *Selection);

  // Description:
  // To change the display when the element set is changed.
  void ElementSetChangedCallback(const char *Selection);

  // Description:
  // To display the pop-up window in which the range and scale of the property
  // being displayed can be changed.
  void DisplayOptionsCallback( );
  
protected:
        vtkKWMimxDisplayMatPropGroup();
        ~vtkKWMimxDisplayMatPropGroup();
        virtual void CreateWidget();

        // Description:
        // To remove the previous display when the FE mesh selection changes.
        // The element set display is changed to mesh display and display of 
        // scalars is removed.
        void RemovePreviousSelectionDisplay();

  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWComboBoxWithLabel *ElementSetComboBox;
  vtkKWFrame *ViewFrame;
  vtkKWCheckButton *ViewLegendButton;
  vtkKWCheckButton *EnablePlaneButton;
  vtkKWCheckButtonWithLabel *InvertPlaneButton;
  vtkKWPushButton *DisplayOptionsButton;
  vtkKWMimxViewPropertiesOptionGroup *ViewOptionsGroup;
  
private:
  vtkKWMimxDisplayMatPropGroup(const vtkKWMimxDisplayMatPropGroup&); // Not implemented
  void operator=(const vtkKWMimxDisplayMatPropGroup&); // Not implemented
  
  char meshName[64];
  char elementSetName[64];
  char elementSetSelectionPrevious[64];
};

#endif

