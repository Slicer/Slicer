/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditNodeSetNumbersGroup.h,v $
Language:  C++
Date:      $Date: 2008/04/25 21:31:09 $
Version:   $Revision: 1.1 $

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
// .NAME vtkKWMimxEditNodeSetNumbersGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxEditNodeSetNumbersGroup_h
#define __vtkKWMimxEditNodeSetNumbersGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxEditNodeSetNumbersGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditNodeSetNumbersGroup* New();
vtkTypeRevisionMacro(vtkKWMimxEditNodeSetNumbersGroup,vtkKWMimxGroupBase);
void PrintSelf(ostream& os, vtkIndent indent);
virtual void Update();
virtual void UpdateEnableState();
//void EditNodeSetNumbersDoneCallback();
void EditNodeSetNumbersCancelCallback();
int EditNodeSetNumbersApplyCallback();
void UpdateObjectLists();
void SelectionChangedCallback(const char *Selection);
void NodeSetChangedCallback(const char *Selection);
protected:
        vtkKWMimxEditNodeSetNumbersGroup();
        ~vtkKWMimxEditNodeSetNumbersGroup();
        virtual void CreateWidget();
vtkKWComboBoxWithLabel *ObjectListComboBox;
vtkKWComboBoxWithLabel *NodeSetComboBox;
vtkKWEntryWithLabel *StartingNodeNumberEntry;
private:
  vtkKWMimxEditNodeSetNumbersGroup(const vtkKWMimxEditNodeSetNumbersGroup&); // Not implemented
void operator=(const vtkKWMimxEditNodeSetNumbersGroup&); // Not implemented
 };

#endif

