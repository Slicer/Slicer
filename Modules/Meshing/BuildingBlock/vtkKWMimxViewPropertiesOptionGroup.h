/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxViewPropertiesOptionGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:40 $
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
// .NAME vtkKWMimxViewPropertiesOptionGroup - GUI for changing the display properties
// for the scalar plots.
// .SECTION Description
// The GUI is a pop-up window which contains options to 1) change the range of scalar values
// displayed, 2) Convert Red to Blue or Blue to Red scale, 3) Show or hide legend bar and
// 4) Show or hide the cutting plane to see the interior elements of the mesh.

#ifndef __vtkKWMimxViewPropertiesOptionGroup_h
#define __vtkKWMimxViewPropertiesOptionGroup_h

#include "vtkKWTopLevel.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkMimxMeshActor.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWCheckButtonWithLabel;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxViewPropertiesOptionGroup : public vtkKWTopLevel
{
public:
  static vtkKWMimxViewPropertiesOptionGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxViewPropertiesOptionGroup,vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);
 
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // To channge the color mode from red to blue and blue to red.
  void ColorModeCallback(int mode);

  // Description:
  // Update the specified range.
  void UpdateRangeCallback();

  // Description:
  // To enable the entry widgets that take upper and lower values.
  void SpecifyRangeCallback(int mode);
  void SpecifyTitleCallback(int mode);
  void DeselectAllButtons();
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MeshActor, vtkMimxMeshActor);
  vtkSetStringMacro(ElementSetName); 
  vtkSetStringMacro(ArrayName);
  vtkGetObjectMacro(DecimalPrecisionWidget, vtkKWComboBoxWithLabel);
  vtkGetObjectMacro(SpecifyTitleButton, vtkKWCheckButtonWithLabel);
  vtkGetObjectMacro(LegendTitle, vtkKWEntryWithLabel);
  void SetDecimalPrecisionCallback( char *value );
  void SetLegendTitleCallback ( char *title );
  void ResetValues();
protected:
        vtkKWMimxViewPropertiesOptionGroup();
        ~vtkKWMimxViewPropertiesOptionGroup();
        virtual void CreateWidget();
  
private:
  vtkKWMimxViewPropertiesOptionGroup(const vtkKWMimxViewPropertiesOptionGroup&); // Not implemented
  void operator=(const vtkKWMimxViewPropertiesOptionGroup&); // Not implemented
  
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkMimxMeshActor *MeshActor;  
  char *ElementSetName;
  char *ArrayName;
  
  vtkKWFrame *OptionsFrame;
  vtkKWMenuButtonWithLabel *ColorMenuButton;
  vtkKWFrameWithLabel *RangeFrame;
  vtkKWCheckButtonWithLabel *SpecifyRangeButton;
  vtkKWEntryWithLabel *RangeMinimum;
  vtkKWEntryWithLabel *RangeMaximum;
  vtkKWPushButton *UpdateRange;
  
  vtkKWFrameWithLabel *LegendFrame;
  vtkKWComboBoxWithLabel *DecimalPrecisionWidget;
  vtkKWCheckButtonWithLabel *SpecifyTitleButton;
  vtkKWEntryWithLabel *LegendTitle;
  vtkKWPushButton *CloseButton;
  
  double* GetRange();
};

#endif

