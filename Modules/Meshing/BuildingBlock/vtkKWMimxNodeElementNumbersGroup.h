/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxNodeElementNumbersGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.2.4.1 $

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
// .NAME vtkKWMimxNodeElementNumbersGroup - GUI used in node and element set creation
// and renumbering.
// .SECTION Description
// The class contains 4 entry options. Node set name input and starting node number
// and element set name input and starting element number. The GUI is parented to
// the other groups where new elements are created.

#ifndef __vtkKWMimxNodeElementNumbersGroup_h
#define __vtkKWMimxNodeElementNumbersGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWEntry;
class vtkKWLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxNodeElementNumbersGroup : public vtkKWMimxGroupBase
{
public:
    static vtkKWMimxNodeElementNumbersGroup* New();
        vtkTypeRevisionMacro(vtkKWMimxNodeElementNumbersGroup,vtkKWMimxGroupBase);
        void PrintSelf(ostream& os, vtkIndent indent);
        virtual void Update();
        virtual void UpdateEnableState();

        // Description:
        // Access to node, element number entry and node, element set names
        vtkGetObjectMacro(NodeNumberEntry, vtkKWEntry);
        vtkGetObjectMacro(ElementNumberEntry, vtkKWEntry);
        vtkGetObjectMacro(NodeSetNameEntry, vtkKWEntry);
        vtkGetObjectMacro(ElementSetNameEntry, vtkKWEntry);

protected:
        vtkKWMimxNodeElementNumbersGroup();
        ~vtkKWMimxNodeElementNumbersGroup();
        virtual void CreateWidget();
        vtkKWComboBoxWithLabel *ObjectListComboBox;
        vtkKWEntry *NodeNumberEntry;
        vtkKWEntry *ElementNumberEntry;
        vtkKWEntry *ElementSetNameEntry;
        vtkKWEntry *NodeSetNameEntry;
        vtkKWLabel *IdLabel;
        vtkKWLabel *NumberLabel;
        vtkKWLabel *NodeLabel;
        vtkKWLabel *ElementLabel;
        
private:
  vtkKWMimxNodeElementNumbersGroup(const vtkKWMimxNodeElementNumbersGroup&); // Not implemented
void operator=(const vtkKWMimxNodeElementNumbersGroup&); // Not implemented
 };

#endif

