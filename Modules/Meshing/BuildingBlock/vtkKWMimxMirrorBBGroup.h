/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMirrorBBGroup.h,v $
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
// .NAME vtkKWMimxMirrorBBGroup - GUI for mirroring a selected building block
// about an a given plane
// .SECTION Description
// The GUI contains an option to activate the mirroring plane and for placing
// the mirroring plane perpendicular to X, Y and Z axes. The mirroring plane
// can be interactively rotated and moved. An instance vtkPlaneWidget is used
// to depict the mirroring plane.
// .SECTION See Also
// vtkPlaneWidget, vtkKWMimxEditBBGroup

#ifndef __vtkKWMimxMirrorBBGroup_h
#define __vtkKWMimxMirrorBBGroup_h

#include "vtkKWMimxGroupBase.h"

#include "vtkKWMimxEditBBGroup.h"
#include "vtkKWRadioButton.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWEntryWithLabel;
class vtkKWRadioButtonSet;
class vtkKWCheckButtonWithLabel;
class vtkKWComboBoxWithLabel;
class vtkPlaneWidget;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxMirrorBBGroup : public vtkKWMimxGroupBase
{
public:
    static vtkKWMimxMirrorBBGroup* New();
        vtkTypeRevisionMacro(vtkKWMimxMirrorBBGroup,vtkKWMimxGroupBase);
        void PrintSelf(ostream& os, vtkIndent indent);
        virtual void Update();
        virtual void UpdateEnableState();

        // Description:
        // Remove the GUI from display
        void MirrorBBCancelCallback();

        // Description:
        // Create a mirrored building block structure
        int MirrorBBApplyCallback();

        // Description:
        // Set macros used because the GUI is displayed as a subgroup
        // of EditBBGroup
        vtkSetObjectMacro(EditBBGroup, vtkKWMimxEditBBGroup);
        vtkSetObjectMacro(VTKRadioButton, vtkKWRadioButton);

        // Description:
        // Callbacks to place the mirroring plane about the three axes
        // X, Y and Z
        void PlaceMirroringPlaneAboutX();
        void PlaceMirroringPlaneAboutY();
        void PlaceMirroringPlaneAboutZ();

        // Description:
        // TO update the building block structure entries
        void UpdateObjectLists();

        // Description:
        // To place the mirroring plane after the selection is changed
        void SelectionChangedCallback(const char*);

protected:
        vtkKWMimxMirrorBBGroup();
        ~vtkKWMimxMirrorBBGroup();
        virtual void CreateWidget();
        vtkKWMimxEditBBGroup *EditBBGroup;
        vtkKWRadioButton *VTKRadioButton;
        vtkKWCheckButtonWithLabel *TypeOfMirroring;
        vtkKWRadioButtonSet *AxisSelection;
        vtkKWComboBoxWithLabel *ObjectListComboBox;
        vtkPlaneWidget *MirrorPlaneWidget;
private:
  vtkKWMimxMirrorBBGroup(const vtkKWMimxMirrorBBGroup&); // Not implemented
void operator=(const vtkKWMimxMirrorBBGroup&); // Not implemented
 };

#endif

