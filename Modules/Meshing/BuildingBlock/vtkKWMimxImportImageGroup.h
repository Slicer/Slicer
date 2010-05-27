/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxImportImageGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.1.2.1 $

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
// .NAME vtkKWMimxImportImageGroup - Load an image from the MRML scene
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It will load a 
// image that is in Slicer's MRML scene into the image lists maintained by IA-FEMesh

#ifndef __vtkKWMimxImportImageGroup_h
#define __vtkKWMimxImportImageGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkKWComboBoxWithLabel;
class vtkKWLoadSaveDialog;
class vtkEntryChangedEventCallback;
class vtkKWFrameWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxImportImageGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxImportImageGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxImportImageGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void Update();
  virtual void UpdateEnableState();
  void UpdateObjectLists();
  void ImportToImageCancelCallback();
  int SelectVolumeToImportToImageCallback();
  
protected:
        vtkKWMimxImportImageGroup();
        ~vtkKWMimxImportImageGroup();
        virtual void CreateWidget();

        virtual void FillModelComboBox();
        
private:
  vtkKWMimxImportImageGroup(const vtkKWMimxImportImageGroup&); // Not implemented
  void operator=(const vtkKWMimxImportImageGroup&); // Not implemented

  vtkKWComboBoxWithLabel* ImageListComboBox;

 };

#endif

