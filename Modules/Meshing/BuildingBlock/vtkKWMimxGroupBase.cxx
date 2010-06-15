/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxGroupBase.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.16.4.1 $

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

#include "vtkKWMimxGroupBase.h"

#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxUnstructuredGridWidget.h"

#include "vtkActor.h"
#include "vtkMimxBoundingBoxSource.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxMeshActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkKWRenderWidget.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
//#include "vtkKWMimxViewProperties.h"
#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxGroupBase);
vtkCxxRevisionMacro(vtkKWMimxGroupBase, "$Revision: 1.16.4.1 $");
//----------------------------------------------------------------------------
vtkKWMimxGroupBase::vtkKWMimxGroupBase()
{
  this->BBoxList = NULL;
  this->FEMeshList = NULL;
  this->SurfaceList = NULL;
  this->ImageList = NULL;
  this->MimxMainWindow = NULL;
  this->CancelButton = vtkKWPushButton::New();
  this->MainFrame = vtkKWFrame::New();
  this->Count = 0;
  this->ViewProperties = NULL;
  this->MenuGroup = NULL;
  this->ApplyButton = vtkKWPushButton::New();
  this->DoUndoTree = NULL;
  this->CancelStatus = 0;
  strcpy(this->objectSelectionPrevious,"");
 // this->objectSelectionCurrent = "";
}

//----------------------------------------------------------------------------
vtkKWMimxGroupBase::~vtkKWMimxGroupBase()
{
  this->CancelButton->Delete();
  this->MainFrame->Delete();
  this->ApplyButton->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxGroupBase::CreateWidget()
{
        if(this->IsCreated())
        {
                vtkErrorMacro("class already created");
                return;
        }

        this->Superclass::CreateWidget();

}
//----------------------------------------------------------------------------
void vtkKWMimxGroupBase::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxGroupBase::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
        // *** in slicer, the panels don't update correctly 
        this->MimxMainWindow->ForceWidgetRedraw();
}
//----------------------------------------------------------------------------
void vtkKWMimxGroupBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void vtkKWMimxGroupBase::AddMeshToDisplay( vtkUnstructuredGrid *mesh, 
            const char *namePrefix, const char *foundationName, const char *elementSetName)
{
  vtkMimxMeshActor *actor = vtkMimxMeshActor::New();
  actor->SetFoundationName(foundationName);
  this->FEMeshList->AppendItem( actor );
        actor->SetDataSet( mesh );
        actor->SetRenderer( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer() );
        actor->SetInteractor( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor() );
  
  bool useCounter = false;
  int offset = 1;
        std::string myName = foundationName;
  std::string myPrefix = namePrefix;
  if (myPrefix.length() > 0)
  {
    myName += "_";
    myName += myPrefix;
    offset = 0;
  }
  
  
        int maxIndex = 0;
  for (int i=0;i<this->FEMeshList->GetNumberOfItems()-1;i++)
  {
    const char *itemName = this->FEMeshList->GetItem(i)->GetFileName();
    const char *substr = strstr(itemName, myName.c_str());
    if ( substr == itemName)
    {
      char itemIndex[16];
      int startIndex = myName.length();
      int itemLength = strlen(itemName);
      int length = itemLength - startIndex;
      int currentIndex;
      
      useCounter = true;
      if (length == 0)
      {
        currentIndex = 0;
      }
      else
      {
        strncpy(itemIndex, &(itemName[startIndex+offset]), length-offset);
        itemIndex[length-offset] = '\0';
        currentIndex = atoi(itemIndex);
      }
      
      if (maxIndex < currentIndex) maxIndex = currentIndex;
    }
  }
  
  vtkIdType index = maxIndex+1;
  
  if ( (useCounter) || (strcmp(namePrefix, "") != 0) )
  {
    actor->SetObjectName(namePrefix,index);
  }
  else
  {
    actor->SetFilePath(namePrefix);
  }
  
  // check if triangle or quad elements exist
  if (elementSetName != NULL)
  {
    vtkCellTypes *cellTypes = vtkCellTypes::New();
    actor->GetDataSet()->GetCellTypes(cellTypes);
   
    for (int i=0; i<cellTypes->GetNumberOfTypes(); i++)
    {
          if (cellTypes->GetCellType(i) == VTK_TRIANGLE ||
                    cellTypes->GetCellType(i) == VTK_QUAD)
          {
                actor->AddReferenceNode( elementSetName );
                break;
          }
    }
  }

  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
  
  int itemIndex = this->FEMeshList->GetNumberOfItems()-1;
        this->GetMimxMainWindow()->GetViewProperties()->AddObjectList( this->FEMeshList->GetItem( itemIndex ) );
                      
}
//----------------------------------------------------------------------------
void vtkKWMimxGroupBase::AddBuildingBlockToDisplay(vtkUnstructuredGrid *ugrid, 
          const char *namePrefix, const char *foundationName)
{
  /* Create the New Display Node */
  this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
  int currentitem = this->BBoxList->GetNumberOfItems()-1;
        this->BBoxList->GetItem(currentitem)->SetDataType( ACTOR_BUILDING_BLOCK );
        
        vtkMimxUnstructuredGridActor *actor = vtkMimxUnstructuredGridActor::SafeDownCast(
          this->BBoxList->GetItem(currentitem));
        actor->SetFoundationName(foundationName);
        actor->GetDataSet()->DeepCopy( ugrid );
        
        /* Create the Redo/Undo tree */
  this->DoUndoTree->AppendItem(new Node);
  this->DoUndoTree->GetItem(currentitem)->Parent = NULL;
  this->DoUndoTree->GetItem(currentitem)->Child = NULL;
  this->DoUndoTree->GetItem(currentitem)->Data = actor;
        
        bool useCounter = false;
        int offset = 1;
        std::string myName = foundationName;
  std::string myPrefix = namePrefix;
  if (myPrefix.length() > 0)
  {
    myName += "_";
    myName += myPrefix;
    offset = 0;
  }

        int maxIndex = 0;
  for (int i=0;i<this->BBoxList->GetNumberOfItems()-1;i++)
  {
    const char *itemName = this->BBoxList->GetItem(i)->GetFileName();
    const char *substr = strstr(itemName, myName.c_str());
    if ( substr == itemName)
    {
      char itemIndex[16];
      int startIndex = myName.length();
      int itemLength = strlen(itemName);
      int length = itemLength - startIndex;
      int currentIndex;
      
      useCounter = true;
      if (length == 0)
      {
        currentIndex = 0;
      }
      else
      {
        strncpy(itemIndex, &(itemName[startIndex+offset]), length-offset);
        itemIndex[length-offset] = '\0';
        currentIndex = atoi(itemIndex);
      }
      
      if (maxIndex < currentIndex) maxIndex = currentIndex;
    }
  }
  
  vtkIdType index = maxIndex+1;
  
  if ( (useCounter) || (strcmp(namePrefix, "") != 0) )
  {
    actor->SetObjectName(namePrefix,index);
  }
  else
  {
    actor->SetFilePath(namePrefix);
  }
               
  /* Assign Mesh Seeds - The initial values are based on Application Settings */
  double edgeLength = this->GetMimxMainWindow()->GetAverageElementLength();
  actor->MeshSeedFromAverageElementLength( edgeLength, edgeLength, edgeLength );
  actor->GetDataSet()->Modified();
  
  this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp( actor );
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
  this->GetMimxMainWindow()->GetViewProperties()->AddObjectList( this->BBoxList->GetItem(currentitem));
}         

//----------------------------------------------------------------------------
void vtkKWMimxGroupBase::AddSurfaceToDisplay(vtkPolyData *surface, 
          const char *namePrefix, const char *foundationName)
{

  this->SurfaceList->AppendItem(vtkMimxSurfacePolyDataActor::New());
  int item = this->SurfaceList->GetNumberOfItems()-1;
  this->SurfaceList->GetItem(item)->SetDataType(ACTOR_POLYDATA_SURFACE);
  this->SurfaceList->GetItem(item)->SetFoundationName( foundationName );
  vtkMimxSurfacePolyDataActor *actor = vtkMimxSurfacePolyDataActor::SafeDownCast(
    this->SurfaceList->GetItem(item));
  actor->GetDataSet()->DeepCopy( surface );
  
  bool useCounter = false;
  int offset = 1;
  std::string myName = foundationName;
  std::string myPrefix = namePrefix;
  if (myPrefix.length() > 0)
  {
    myName += "_";
    myName += myPrefix;
    offset = 0;
  }
  
  int maxIndex = 0;
  for (int i=0;i<this->SurfaceList->GetNumberOfItems()-1;i++)
  {
    const char *itemName = this->SurfaceList->GetItem(i)->GetFileName();
    const char *substr = strstr(itemName, myName.c_str());
    if ( substr == itemName)
    {
      char itemIndex[16];
      int startIndex = myName.length();
      int itemLength = strlen(itemName);
      int length = itemLength - startIndex;
      int currentIndex;
      
      useCounter = true;
      if (length == 0)
      {
        currentIndex = 0;
      }
      else
      {
        strncpy(itemIndex, &(itemName[startIndex+offset]), length-offset);
        itemIndex[length-offset] = '\0';
        currentIndex = atoi(itemIndex);
      }
      
      if (maxIndex < currentIndex) maxIndex = currentIndex;
    }
  }
  
  vtkIdType index = maxIndex + 1;
  if ( (useCounter) || (strcmp(namePrefix, "") != 0) )
  {
    actor->SetObjectName(namePrefix,index);
  }
  else
  {
    actor->SetFilePath(namePrefix);
  }
  actor->GetDataSet()->Modified();
  this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp(
        this->SurfaceList->GetItem(item)->GetActor());
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
  this->GetMimxMainWindow()->GetViewProperties()->AddObjectList( this->SurfaceList->GetItem(item) );
}



//----------------------------------------------------------------------------
void vtkKWMimxGroupBase::AddImageToDisplay(vtkImageData *surface,
          const char *namePrefix, const char *foundationName, vtkMatrix4x4 *matrix, double origin[3], double spacing[3])
{

  // initialize an image actor to hold the reference to the stored image in the MRML scene
  vtkMimxImageActor *actor = vtkMimxImageActor::New();
  actor->SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindow()->GetInteractor());

  // copy the image referenced elsewhere in the MRML scene so we will have our own copy for the rest of
  // the processing workflow
  vtkImageData *imageToAdd = vtkImageData::New();
  imageToAdd->DeepCopy(surface);
  actor->SetImageDataSet( imageToAdd, matrix, origin, spacing );
  actor->SetDataType(ACTOR_IMAGE);
  actor->SetFoundationName( foundationName );

  // add this image actor into the list of actors maintained by the module
  this->ImageList->AppendItem(actor);

  // add an entry in the view properties so we can enable/disable the image planes; then force a re-render
  this->GetMimxMainWindow()->GetViewProperties()->AddObjectList( actor );
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();

  actor->Delete();
  imageToAdd->Delete();
}



//----------------------------------------------------------------------------
void vtkKWMimxGroupBase::AddImageToDisplay(vtkImageData *surface,
          const char *namePrefix, const char *foundationName)
{
  // *** not used anymore because we needed to transform data and pass the transform down through for
  // integration with Slicer.
  this->ImageList->AppendItem(vtkMimxImageActor::New());
  int item = this->ImageList->GetNumberOfItems()-1;
  this->ImageList->GetItem(item)->SetDataType(ACTOR_IMAGE);
  this->ImageList->GetItem(item)->SetFoundationName( foundationName );
  vtkMimxImageActor *actor = vtkMimxImageActor::SafeDownCast(this->ImageList->GetItem(item));
  actor->SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindow()->GetInteractor());
  actor->SetImageDataSet( surface );

  // add an entry in the view properties so we can enable/disable the image planes; then force a re-render
  this->GetMimxMainWindow()->GetViewProperties()->AddObjectList( this->ImageList->GetItem(item) );
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
}




//----------------------------------------------------------------------------
int vtkKWMimxGroupBase::UpdateSurfaceComboBox(vtkKWComboBox *combobox)
{
  combobox->DeleteAllValues();
  
  int defaultItem = -1;
  for (int i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
  {
    combobox->AddValue( this->SurfaceList->GetItem(i)->GetFileName() );
      
    int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->SurfaceList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
  }
  
  if ((this->SurfaceList->GetNumberOfItems() > 0) && (defaultItem == -1))
  {
    defaultItem = this->SurfaceList->GetNumberOfItems()-1;
  }
    
  if (defaultItem != -1)
  {
    combobox->SetValue(
          this->SurfaceList->GetItem(defaultItem)->GetFileName());
  }
  
  return defaultItem;
}
//----------------------------------------------------------------------------
int vtkKWMimxGroupBase::UpdateMeshComboBox(vtkKWComboBox *combobox)
{
  combobox->DeleteAllValues();
  
  int defaultItem = -1;
  for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
    combobox->AddValue( this->FEMeshList->GetItem(i)->GetFileName() );
      
    int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->FEMeshList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
  }
  
  if ((this->FEMeshList->GetNumberOfItems() > 0) && (defaultItem == -1))
  {
    defaultItem = this->FEMeshList->GetNumberOfItems()-1;
  }
    
  if (defaultItem != -1)
  {
    combobox->SetValue(
          this->FEMeshList->GetItem(defaultItem)->GetFileName());
  }
  
  return defaultItem;
}
//----------------------------------------------------------------------------
int vtkKWMimxGroupBase::UpdateBuildingBlockComboBox(vtkKWComboBox *combobox)
{
  combobox->DeleteAllValues();
  
  int defaultItem = -1;
  for (int i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  {
    combobox->AddValue( 
      this->BBoxList->GetItem(i)->GetFileName() );
      
    int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->BBoxList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
  }
  
  if ((this->BBoxList->GetNumberOfItems() > 0) && (defaultItem == -1))
  {
    defaultItem = this->BBoxList->GetNumberOfItems()-1;
  }
    
  if (defaultItem != -1)
  {
    combobox->SetValue(
          this->BBoxList->GetItem(defaultItem)->GetFileName());
  }
  
  return defaultItem;
}
//----------------------------------------------------------------------------
int vtkKWMimxGroupBase::UpdateImageComboBox(vtkKWComboBox *combobox)
{
  combobox->DeleteAllValues();
  
  int defaultItem = -1;
  for (int i = 0; i < this->ImageList->GetNumberOfItems(); i++)
  {
    combobox->AddValue( 
      this->ImageList->GetItem(i)->GetFileName() );
      
    int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->ImageList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
  }
  
  if ((this->ImageList->GetNumberOfItems() > 0) && (defaultItem == -1))
  {
    defaultItem = this->ImageList->GetNumberOfItems()-1;
  }
    
  if (defaultItem != -1)
  {
    combobox->SetValue(
          this->ImageList->GetItem(defaultItem)->GetFileName());
  }
  
  return defaultItem;
}       
