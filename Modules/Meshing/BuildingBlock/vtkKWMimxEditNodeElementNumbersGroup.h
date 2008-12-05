/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditNodeElementNumbersGroup.h,v $
Language:  C++
Date:      $Date: 2008/03/21 20:10:54 $
Version:   $Revision: 1.8 $

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
// .NAME vtkKWMimxEditNodeElementNumbersGroup - a taFEMeshed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxEditNodeElementNumbersGroup_h
#define __vtkKWMimxEditNodeElementNumbersGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWCheckButtonWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxEditNodeElementNumbersGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditNodeElementNumbersGroup* New();
vtkTypeRevisionMacro(vtkKWMimxEditNodeElementNumbersGroup,vtkKWMimxGroupBase);
void PrintSelf(ostream& os, vtkIndent indent);
virtual void Update();
virtual void UpdateEnableState();
//void EditNodeElementNumbersDoneCallback();
//void EditNodeElementNumbersCancelCallback();
//int EditNodeElementNumbersApplyCallback();
//void UpdateObjectLists();
//void SelectionChangedCallback(const char *);
//void ApplyNodeNumbersCallback(int);
//void ApplyElementNumbersCallback(int);
vtkGetObjectMacro(NodeNumberEntry, vtkKWEntryWithLabel);
vtkGetObjectMacro(ElementNumberEntry, vtkKWEntryWithLabel);
vtkGetObjectMacro(NodeSetNameEntry, vtkKWEntryWithLabel);
vtkGetObjectMacro(ElementSetNameEntry, vtkKWEntryWithLabel);

protected:
        vtkKWMimxEditNodeElementNumbersGroup();
        ~vtkKWMimxEditNodeElementNumbersGroup();
        virtual void CreateWidget();
        vtkKWComboBoxWithLabel *ObjectListComboBox;
        vtkKWEntryWithLabel *NodeNumberEntry;
        vtkKWEntryWithLabel *ElementNumberEntry;
        vtkKWEntryWithLabel *ElementSetNameEntry;
        vtkKWEntryWithLabel *NodeSetNameEntry;
        vtkKWCheckButtonWithLabel *NodeNumberCheckButton;
        vtkKWCheckButtonWithLabel *ElementNumberCheckButton;
private:
  vtkKWMimxEditNodeElementNumbersGroup(const vtkKWMimxEditNodeElementNumbersGroup&); // Not implemented
void operator=(const vtkKWMimxEditNodeElementNumbersGroup&); // Not implemented
 };

#endif

