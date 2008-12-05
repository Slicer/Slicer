/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMainMenuGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.14.4.1 $

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
// .NAME vtkKWMimxMainMenuGroup - Base class for all the menu groups.
//
// .SECTION Description
// Base class that contains all the Set functions for the derived classes.
// Contains declaration for operation menu button which in turn launches
// GUI associated with the selected operations. The derived menu groups
// are Image, Surface, Block(s), Mesh, Quality, Materials and Load/BC

#ifndef __vtkKWMimxMainMenuGroup_h
#define __vtkKWMimxMainMenuGroup_h

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkLinkedListWrapper.h"
#include "vtkLinkedListWrapperTree.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMenuButtonWithLabel;
class vtkKWMimxViewProperties;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxMainMenuGroup : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxMainMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxMainMenuGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Set functions to access all the lists
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(SurfaceList, vtkLinkedListWrapper);
  vtkGetObjectMacro(BBoxList, vtkLinkedListWrapper);
  vtkGetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  vtkGetObjectMacro(ImageList, vtkLinkedListWrapper);
  vtkSetObjectMacro(DoUndoTree, vtkLinkedListWrapperTree);
  vtkGetObjectMacro(MainFrame, vtkKWFrame);
  virtual void SetMenuButtonsEnabled(int);
 protected:
        vtkKWMimxMainMenuGroup();
        virtual ~vtkKWMimxMainMenuGroup();

        virtual void CreateWidget();
        //vtkKWMenuButtonWithLabel *ObjectMenuButton;
        vtkKWMenuButtonWithLabel *OperationMenuButton;
        //vtkKWMenuButtonWithLabel *TypeMenuButton;
  vtkKWFrame *MainFrame;
  vtkLinkedListWrapper *SurfaceList;
  vtkLinkedListWrapper *BBoxList;
  vtkLinkedListWrapper *FEMeshList;
  vtkLinkedListWrapper *ImageList;
  vtkLinkedListWrapper *ImageMaskList;
        const char* ExtractFileName(const char*);
        vtkKWMimxMainWindow *MimxMainWindow;
        vtkKWMimxViewProperties *ViewProperties;
        vtkLinkedListWrapperTree *DoUndoTree;

        int ErrorState;
private:
  vtkKWMimxMainMenuGroup(const vtkKWMimxMainMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxMainMenuGroup&); // Not implemented
 };

#endif


