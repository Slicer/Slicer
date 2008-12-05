/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxImageViewProperties.h,v $
Language:  C++
Date:      $Date: 2008/02/01 15:24:57 $
Version:   $Revision: 1.5 $

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
// .NAME vtkKWMimxImageViewProperties - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxImageViewProperties_h
#define __vtkKWMimxImageViewProperties_h

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"
#include "vtkLocalLinkedListWrapper.h"



class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMultiColumnListWithScrollbars;


class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxImageViewProperties : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxImageViewProperties* New();
  vtkTypeRevisionMacro(vtkKWMimxImageViewProperties,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
 // const char* GetActivePage();        // return the name of the chosen page
 // int GetActiveOption();      // return the chosen operation
  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(ObjectList, vtkLocalLinkedListWrapper);
  vtkGetObjectMacro(MainFrame, vtkKWFrameWithLabel);
  void VisibilityCallback(int );
  void AddObjectList();
  void DeleteObjectList(int );
//  void UpdateVisibility();
protected:
        vtkKWMimxImageViewProperties();
        ~vtkKWMimxImageViewProperties();
        virtual void CreateWidget();
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkLocalLinkedListWrapper *ObjectList;
  vtkKWFrameWithLabel *MainFrame;
private:
  vtkKWMimxImageViewProperties(const vtkKWMimxImageViewProperties&); // Not implemented
  void operator=(const vtkKWMimxImageViewProperties&); // Not implemented
 };

#endif

