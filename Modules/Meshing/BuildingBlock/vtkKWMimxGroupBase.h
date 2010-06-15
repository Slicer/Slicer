/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxGroupBase.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.15.4.1 $

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
// .NAME vtkKWMimxGroupBase - Base class for all the GUI used
// .SECTION Description
// The class contains two push buttons for Apply and Cancel and a Frame 
// which houses all the GUI.
//
// .SECTION To Be Done
// Should also incorporate combobox widget which is used for selection of an
// object and is common in all the derived classes. Should also include
// UpdateObjectLists function common in all the derived classes.

#ifndef __vtkKWMimxGroupBase_h
#define __vtkKWMimxGroupBase_h

/* VTK Headers */
#include "vtkCommand.h"


/* KWWidget Headers */
#include "vtkKWCompositeWidget.h"

#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxViewProperties.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkKWMimxViewProperties.h"
#include "vtkKWComboBox.h"
#include "vtkLinkedListWrapper.h"
#include "vtkLinkedListWrapperTree.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkKWFrame;
class vtkKWPushButton;
class vtkLinkedListWrapper;
class vtkLinkedListWrapperTree;
class vtkKWMimxMainMenuGroup;

//-----------------------------------------------------------------------------
class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxGroupBase : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxGroupBase* New();
  vtkTypeRevisionMacro(vtkKWMimxGroupBase,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Set all the list  and window access
  vtkSetObjectMacro(SurfaceList, vtkLinkedListWrapper);
  vtkSetObjectMacro(BBoxList, vtkLinkedListWrapper);
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(MainFrame, vtkKWFrame);
  vtkSetObjectMacro(ViewProperties, vtkKWMimxViewProperties);
  vtkSetObjectMacro(MenuGroup, vtkKWMimxMainMenuGroup);
  vtkSetObjectMacro(ImageList, vtkLinkedListWrapper);
  vtkGetObjectMacro(ImageList, vtkLinkedListWrapper);
  vtkSetObjectMacro(DoUndoTree, vtkLinkedListWrapperTree);

  // Description:
  // This is used to count the number of objects generated from a
  // particular type of operation. Initially it was assumed that each group
  // had only one object creation operation. With the changes made, the group
  // can have more than one operation.
  vtkSetMacro(Count, int);
  void AddMeshToDisplay(vtkUnstructuredGrid *ugrid, const char *namePrefix, 
              const char *FoundationName, const char *elementSetName);
  void AddSurfaceToDisplay(vtkPolyData *surface, const char *namePrefix, 
              const char *foundationName);
  void AddBuildingBlockToDisplay(vtkUnstructuredGrid *ugrid, const char *namePrefix, 
              const char *foundationName);
  int UpdateBuildingBlockComboBox(vtkKWComboBox *combobox);
  int UpdateMeshComboBox(vtkKWComboBox *combobox);
  int UpdateSurfaceComboBox(vtkKWComboBox *combobox);
  int UpdateImageComboBox(vtkKWComboBox *combobox);
  
  // added for slicer integration, treat images as first class objects and read from MRML tree
  void AddImageToDisplay(vtkImageData *surface,
            const char *namePrefix, const char *foundationName);

  // version that accepts a transform matrix to orient the images to RAS for Slicer
  void AddImageToDisplay(vtkImageData *surface,
            const char *namePrefix, const char *foundationName, vtkMatrix4x4 *matrix, double origin[3], double spacing[3]);

  int CancelStatus;

protected:
        vtkKWMimxGroupBase();
        virtual ~vtkKWMimxGroupBase();
        virtual void CreateWidget();
  vtkLinkedListWrapper *SurfaceList;
  vtkLinkedListWrapper *BBoxList;
  vtkLinkedListWrapper *FEMeshList;
  vtkLinkedListWrapper *ImageList;
  vtkKWPushButton *ApplyButton;
  vtkKWPushButton *CancelButton;
  vtkKWFrame *MainFrame;

  vtkKWMimxMainWindow *MimxMainWindow;
  vtkKWMimxViewProperties *ViewProperties;
  vtkLinkedListWrapperTree *DoUndoTree;
  vtkKWMimxMainMenuGroup *MenuGroup;
  vtkIdType Count;  // to keep track of number of objects created during runtime 
  char objectSelectionPrevious[256];

private:
  vtkKWMimxGroupBase(const vtkKWMimxGroupBase&); // Not implemented
  void operator=(const vtkKWMimxGroupBase&); // Not implemented
 };

#endif

