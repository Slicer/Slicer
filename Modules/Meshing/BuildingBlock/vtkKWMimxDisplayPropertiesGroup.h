/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDisplayPropertiesGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.9.4.1 $

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
// .NAME vtkKWMimxDisplayPropertiesGroup - Pop-up window to show GUI used to
// in changing view properties.
// .SECTION Description
// The class contains buttons to 1) Change the background color of the window
// 2) Text color displayed, 3) To reset the viewing axis (+x, -x, +y, -y, +z, -z)
// 4) Choice between parallel and perspective projection, 5) Refitting the screen
// to accommodate all actors in the view port, 6) Screen capture and 7) Show/hide
// the orientation axis.

#ifndef __vtkKWMimxDisplayPropertiesGroup_h
#define __vtkKWMimxDisplayPropertiesGroup_h

#include "vtkKWTopLevel.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWChangeColorButton;
class vtkKWFrameWithLabel;
class vtkKWLoadSaveDialog;
class vtkKWPushButton;
class vtkKWPushButtonSet;
class vtkKWRadioButtonSet;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxDisplayPropertiesGroup : public vtkKWTopLevel
{
public:
  static vtkKWMimxDisplayPropertiesGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxDisplayPropertiesGroup,vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Set/Get main window in which the viewing changes need to be made.
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  
  // Description:
  // Set background color
  virtual void SetBackGroundColor(double color[3]);

  // Description:
  // Launch a file save dialog box to store the window snap shot in JPEG format
  void WindowShapshot();

  // Description:
  // Reposition the camera along the axis chosen
  void PositiveXViewCallback();
  void PositiveYViewCallback();
  void PositiveZViewCallback();
  void NegativeXViewCallback();
  void NegativeYViewCallback();
  void NegativeZViewCallback();

  // Description:
  // Perspective view on
  void PerspectiveViewCallback();

  // Description:
  // Parallel view on
  void ParallelViewCallback();

  // Description:
  // Reset so that the view contains all the visible actors
  void ResetViewCallback();

  // Description:
  // Show/hide the local axis actor
  void ShowLocalAxesActor( );
  
  // Description:
  // Set the text color for the direction axis and all the
  // scalarbaractors in the scene.
  void SetTextColor(double *);
  void SetTextColor( double red, double green, double blue );
  double *GetTextColor( );
   
protected:
        vtkKWMimxDisplayPropertiesGroup();
        ~vtkKWMimxDisplayPropertiesGroup();
        virtual void CreateWidget();
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkLinkedListWrapper *ObjectList;
  vtkKWFrameWithLabel *BackGroundFrame;
  vtkKWFrameWithLabel *CameraFrame;
  vtkKWChangeColorButton *BackGroundColorButton;
  vtkKWPushButtonSet *ViewAxisChoiceButton;
  vtkKWLoadSaveDialog *LoadSaveDialog;
  vtkKWTopLevel *DisplayPropertiesDialog;
  vtkKWPushButton *CloseButton;
  vtkKWChangeColorButton *TextColorButton;
  double TextColor[3];
  
   //int DataType;
private:
  vtkKWMimxDisplayPropertiesGroup(const vtkKWMimxDisplayPropertiesGroup&); // Not implemented
  void operator=(const vtkKWMimxDisplayPropertiesGroup&); // Not implemented
 };

#endif

