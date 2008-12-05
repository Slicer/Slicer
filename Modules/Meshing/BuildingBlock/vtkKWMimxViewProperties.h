/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxViewProperties.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:40 $
Version:   $Revision: 1.28.4.1 $

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
// .NAME vtkKWMimxViewProperties - GUI to display the names of the objects loaded
// in the program, their visibility status and the object type.
// .SECTION Description
// The class contains a vtkMultiColumnList with three columns. The first column indicates 
// the visibility and the second column contains the name of the object. The first column
// is a checkbutton entry and the second column is a pushbutton. The push button launches
// a window with the properties of the actor displayed. The third column contains the 
// object types. The objects types are Image, Building Block and FE Mesh.

#ifndef __vtkKWMimxViewProperties_h
#define __vtkKWMimxViewProperties_h

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkLinkedListWrapperTree.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWFrameWithLabel;
class vtkKWFrameWithScrollbar;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWPushButton;

class vtkMimxActorBase;

class vtkKWMimxViewPropertiesGroup;

class vtkLocalLinkedListWrapper;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxViewProperties : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxViewProperties* New();
  vtkTypeRevisionMacro(vtkKWMimxViewProperties,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Set all the necessary program access
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(DoUndoTree, vtkLinkedListWrapperTree);
  vtkGetObjectMacro(MainFrame, vtkKWFrameWithLabel);
  vtkGetObjectMacro(MultiColumnList, vtkKWMultiColumnListWithScrollbars);
  vtkGetObjectMacro(ObjectList, vtkLocalLinkedListWrapper);

  // Description:
  // Change visibility. The second entry if 0 is hide, else if 1 show
  void VisibilityCallback(const char *objectId, int );

  // Description:
  // Add the loaded object to the list maintained by the multicolumn list.
  // Each object is assigned an unique object id. This is done when there
  // are name conflicts between objects.
  void AddObjectList(vtkMimxActorBase*);

  // Description:
  // Delete the object from the multicolumn list based the object type and position.
  void DeleteObjectList(int DataType, int Position );

  // Description:
  // Delete the object from the multicolumn list based the object name.
  void DeleteObjectList(const char *name);

  // Description:
  // Update the visibility taking the visibility in the view window as the reference.
  void UpdateVisibility();

  // Description:
  // Update the visiblity based on the status of the checkbuttons in the multicolumnlist.
  void UpdateVisibilityList();

  // Description:
  // Launch a window containing the actor details of the object whose pushbutton is
  // activated.
  void ViewPropertyCallback(const char *objectId, const char *name);

  // Description:
  // Set the command to the pushbutton created when a new object is added.
  void CreateNameCellCallback(const char *objectId, int actorType, const char *tableWidgetName, int row, int col, const char *widgetName);
  void SortedCommandCallback();

  // Description:
  // Enable the multicolumn list only if the number of objects > 0.
  void EnableViewPropertyList( int mode );
  
  
protected:
        vtkKWMimxViewProperties();
        ~vtkKWMimxViewProperties();
        virtual void CreateWidget();
    vtkKWMultiColumnListWithScrollbars *MultiColumnList;
    vtkKWMimxMainWindow *MimxMainWindow;
    vtkLocalLinkedListWrapper *ObjectList;
    vtkKWFrameWithLabel *MainFrame;
    vtkLinkedListWrapperTree *DoUndoTree;
    vtkKWPushButton *ViewButton;
    vtkKWPushButton *DisplayButton;
    vtkKWMimxViewPropertiesGroup *ViewPropertyDialog;
private:
  vtkKWMimxViewProperties(const vtkKWMimxViewProperties&); // Not implemented
  void operator=(const vtkKWMimxViewProperties&); // Not implemented
  
  int ObjectId;
  
 };

#endif

