/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateBBFromBoundsGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.13.4.2 $

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
// .NAME vtkKWMimxCreateBBFromBoundsGroup - GUI to create building block either
// from bounds or from picking.
// .SECTION Description
// GUI contains two radio buttons for manual building block creation through
// picking and automatically from the bounds of the surface chosen. The manual 
// picking is done through vtkMimxCreateBuildingBlockFromPickWidget and the 
// extrusion length can be changed real time. For the manual creation of the 
// building block, the initial building block is in red and using right mouse
// button the block can be selected and it turns white immediately.
//
// .SECTION See Also
// vtkMimxCreateBuildingBlockFromPickWidget

#ifndef __vtkKWMimxCreateBBFromBoundsGroup_h
#define __vtkKWMimxCreateBBFromBoundsGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkUnstructuredGrid;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWFrameWithLabel;
class vtkKWRadioButton;
class vtkMimxCreateBuildingBlockFromPickWidget;
class vtkRightButtonBBManualCreateCallback;     // For right hand mouse button manual block creation

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxCreateBBFromBoundsGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateBBFromBoundsGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateBBFromBoundsGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  
  // Description:
  // To draw the widgets required for automatic building block creation
  void SurfaceBoundCallback( );

  // Description:
  // To draw the widgets required for manual building block creation
  void ManualBoundCallback( );

  // Description:
  // Accept the created building block
  int CreateBBFromBoundsApplyCallback();

  // Description:
  // Hide the GUI from display
  void CreateBBFromBoundsCancelCallback();

  // Description:
  // Update the surface list entries
  void UpdateObjectLists();

  // Description:
  // Enable rubber band pick selection for manual building block creation
  void EnableRubberBandPick( vtkPolyData *surface, double extrusionLength );

  // Description:
  // Disable rubber band pick
  void DisableRubberBandPick();

  // Description:
  // Change the extrusion length for the manual building block creation
  void ExtrusionLengthChangedCallback(const char *value);
  
protected:
        vtkKWMimxCreateBBFromBoundsGroup();
        ~vtkKWMimxCreateBBFromBoundsGroup();
        virtual void CreateWidget();
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWFrame *ModeFrame;
  vtkKWRadioButton *SurfaceBoundButton;
  vtkKWRadioButton *ManualBoundButton;
  vtkKWFrame *EntryFrame;
  vtkKWEntryWithLabel *ExtrusionLengthEntry;
  vtkMimxCreateBuildingBlockFromPickWidget *CreateBuildingBlockFromPickWidget;
  vtkRightButtonBBManualCreateCallback *RightButtonPressCallback;
  int MaualBBCreationCount;
private:
  vtkKWMimxCreateBBFromBoundsGroup(const vtkKWMimxCreateBBFromBoundsGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateBBFromBoundsGroup&); // Not implemented
 };

#endif

