/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDisplayPropertiesGroup.cxx,v $
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

#include "vtkKWMimxDisplayPropertiesGroup.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkJPEGWriter.h"
#include "vtkObjectFactory.h"
#include "vtkProp.h"
#include "vtkPropCollection.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkScalarBarActor.h"
#include "vtkTextProperty.h"
#include "vtkWindowToImageFilter.h"

#include "vtkKWApplication.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWDialog.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWPushButtonSet.h"

#include "vtkKWMimxMainWindow.h"

#include "vtkPVAxesActor.h"

#include "Resources/mimxAutofit.h"
#include "Resources/mimxAxis.h"
#include "Resources/mimxClose.h"
#include "Resources/mimxNegX.h"
#include "Resources/mimxNegY.h"
#include "Resources/mimxNegZ.h"
#include "Resources/mimxPerspective.h"
#include "Resources/mimxParallel.h"
#include "Resources/mimxPosX.h"
#include "Resources/mimxPosY.h"
#include "Resources/mimxPosZ.h"
#include "Resources/mimxSnapshot.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>


// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxDisplayPropertiesGroup);
vtkCxxRevisionMacro(vtkKWMimxDisplayPropertiesGroup, "$Revision: 1.15.4.1 $");

//----------------------------------------------------------------------------
vtkKWMimxDisplayPropertiesGroup::vtkKWMimxDisplayPropertiesGroup()
{
  this->ObjectList = NULL;
  this->MimxMainWindow = NULL;
  this->BackGroundFrame = NULL;
  this->BackGroundColorButton = NULL;
  this->CameraFrame = NULL;
  this->ViewAxisChoiceButton = NULL;
  this->LoadSaveDialog = NULL;
  this->DisplayPropertiesDialog = NULL;
  this->CloseButton = NULL;
  this->TextColorButton = NULL;
  this->TextColor[0] = 1.0;
  this->TextColor[1] = 1.0;
  this->TextColor[2] = 1.0;
}

//----------------------------------------------------------------------------
vtkKWMimxDisplayPropertiesGroup::~vtkKWMimxDisplayPropertiesGroup()
{
  if(this->BackGroundFrame)
          this->BackGroundFrame->Delete();
  if(this->BackGroundColorButton)
          this->BackGroundColorButton->Delete();
  if(this->CameraFrame)
          this->CameraFrame->Delete();
  if(this->ViewAxisChoiceButton)
          this->ViewAxisChoiceButton->Delete();
  if(this->LoadSaveDialog)
          this->LoadSaveDialog->Delete();
        if(this->DisplayPropertiesDialog)
          this->DisplayPropertiesDialog->Delete();
        if(this->CloseButton)
          this->CloseButton->Delete();
        if(this->TextColorButton)
          this->TextColorButton->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::CreateWidget()
{

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SetResizable(1, 1);
  this->SetMinimumSize(250, 225);
  this->ModalOff( );
  this->SetMasterWindow(this->GetMimxMainWindow());
  this->SetTitle("View Settings");

  if (!this->BackGroundFrame)
  {
    this->BackGroundFrame = vtkKWFrameWithLabel::New();
  }
  this->BackGroundFrame->SetParent(this/*->GetParent()*/);
  this->BackGroundFrame->Create();
  this->BackGroundFrame->GetFrame()->SetReliefToGroove();
  this->BackGroundFrame->SetLabelText("Colors");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->BackGroundFrame->GetWidgetName());
 
  // Background color button
  if(!this->BackGroundColorButton)
  {
          this->BackGroundColorButton = vtkKWChangeColorButton::New();
  }
  this->BackGroundColorButton->SetParent(this->BackGroundFrame->GetFrame());
  this->BackGroundColorButton->Create();
  this->BackGroundColorButton->SetColor(
          this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->GetBackground());
  this->BackGroundColorButton->SetLabelPositionToLeft();
  this->BackGroundColorButton->SetLabelText("Background Color");
  this->BackGroundColorButton->SetCommand(this, "SetBackGroundColor");
  this->GetApplication()->Script( "pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
          this->BackGroundColorButton->GetWidgetName());
        
        // Text color button
  if(!this->TextColorButton)
  {
          this->TextColorButton = vtkKWChangeColorButton::New();
  }
  this->TextColorButton->SetParent(this->BackGroundFrame->GetFrame());
  this->TextColorButton->Create();
  vtkPVAxesActor *axisActor = this->GetMimxMainWindow()->GetPVAxesActor();
        vtkProperty* labelProperty = axisActor->GetXAxisLabelProperty();
        this->TextColorButton->SetColor(labelProperty->GetColor());
  this->TextColorButton->SetLabelPositionToLeft();
  this->TextColorButton->SetLabelText("Text Color");
  this->TextColorButton->SetCommand(this, "SetTextColor");
  this->GetApplication()->Script( "pack %s -side right -anchor ne -expand n -padx 2 -pady 2", 
          this->TextColorButton->GetWidgetName());
          
          
  // Camera Frame
  if (!this->CameraFrame)
  {
    this->CameraFrame = vtkKWFrameWithLabel::New();
  }
  this->CameraFrame->SetParent( this );
  this->CameraFrame->Create();
  this->CameraFrame->GetFrame()->SetReliefToGroove();
  this->CameraFrame->SetLabelText("View");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->CameraFrame->GetWidgetName());   

  //int defaultWidth = 3;
  if(!this->ViewAxisChoiceButton)
  {
          this->ViewAxisChoiceButton = vtkKWPushButtonSet::New();
  }
  this->ViewAxisChoiceButton->SetParent(this->CameraFrame->GetFrame());
  this->ViewAxisChoiceButton->Create();
  this->ViewAxisChoiceButton->PackHorizontallyOn();
  this->ViewAxisChoiceButton->SetMaximumNumberOfWidgetsInPackingDirection(6);
  
  this->ViewAxisChoiceButton->AddWidget(0);
  this->ViewAxisChoiceButton->GetWidget(0)->SetBalloonHelpString("Set View +X");
  this->ViewAxisChoiceButton->GetWidget(0)->SetCommand(this, "PositiveXViewCallback");
  vtkKWIcon *xPosIcon = vtkKWIcon::New();
  xPosIcon->SetImage(  image_mimxPosX, 
                       image_mimxPosX_width, 
                       image_mimxPosX_height, 
                       image_mimxPosX_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(0)->SetImageToIcon( xPosIcon );
  
  this->ViewAxisChoiceButton->AddWidget(1);
  this->ViewAxisChoiceButton->GetWidget(1)->SetBalloonHelpString("Set View -X");
  this->ViewAxisChoiceButton->GetWidget(1)->SetCommand(this, "NegativeXViewCallback");
  vtkKWIcon *xNegIcon = vtkKWIcon::New();
  xNegIcon->SetImage(  image_mimxNegX, 
                       image_mimxNegX_width, 
                       image_mimxNegX_height, 
                       image_mimxNegX_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(1)->SetImageToIcon( xNegIcon );
  
  this->ViewAxisChoiceButton->AddWidget(2);
  this->ViewAxisChoiceButton->GetWidget(2)->SetBalloonHelpString("Set View +Y");
  this->ViewAxisChoiceButton->GetWidget(2)->SetCommand(this, "PositiveYViewCallback");
  vtkKWIcon *yPosIcon = vtkKWIcon::New();
  yPosIcon->SetImage(  image_mimxPosY, 
                       image_mimxPosY_width, 
                       image_mimxPosY_height, 
                       image_mimxPosY_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(2)->SetImageToIcon( yPosIcon );
  
  this->ViewAxisChoiceButton->AddWidget(3);
  this->ViewAxisChoiceButton->GetWidget(3)->SetBalloonHelpString("Set View -Y");
  this->ViewAxisChoiceButton->GetWidget(3)->SetCommand(this, "NegativeYViewCallback");
  vtkKWIcon *yNegIcon = vtkKWIcon::New();
  yNegIcon->SetImage(  image_mimxNegY, 
                       image_mimxNegY_width, 
                       image_mimxNegY_height, 
                       image_mimxNegY_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(3)->SetImageToIcon( yNegIcon );
  
        this->ViewAxisChoiceButton->AddWidget(4);
  this->ViewAxisChoiceButton->GetWidget(4)->SetBalloonHelpString("Set View +Z");
  this->ViewAxisChoiceButton->GetWidget(4)->SetCommand(this, "PositiveZViewCallback");
  vtkKWIcon *zPosIcon = vtkKWIcon::New();
  zPosIcon->SetImage(  image_mimxPosZ, 
                       image_mimxPosZ_width, 
                       image_mimxPosZ_height, 
                       image_mimxPosZ_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(4)->SetImageToIcon( zPosIcon );
  
  this->ViewAxisChoiceButton->AddWidget(5);
  this->ViewAxisChoiceButton->GetWidget(5)->SetBalloonHelpString("Set View -Z");
  this->ViewAxisChoiceButton->GetWidget(5)->SetCommand(this, "NegativeZViewCallback");
  vtkKWIcon *zNegIcon = vtkKWIcon::New();
  zNegIcon->SetImage(  image_mimxNegZ, 
                       image_mimxNegZ_width, 
                       image_mimxNegZ_height, 
                       image_mimxNegZ_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(5)->SetImageToIcon( zNegIcon );
  
  this->ViewAxisChoiceButton->AddWidget(6);
  this->ViewAxisChoiceButton->GetWidget(6)->SetBorderWidth(2);
  this->ViewAxisChoiceButton->GetWidget(6)->SetBalloonHelpString("Perspective Projection Viewing");
  this->ViewAxisChoiceButton->GetWidget(6)->SetCommand(this, "PerspectiveViewCallback");
  vtkKWIcon *perspectiveIcon = vtkKWIcon::New();
  perspectiveIcon->SetImage(  image_mimxPerspective, 
                       image_mimxPerspective_width, 
                       image_mimxPerspective_height, 
                       image_mimxPerspective_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(6)->SetImageToIcon( perspectiveIcon );
  
  this->ViewAxisChoiceButton->AddWidget(7);
  this->ViewAxisChoiceButton->GetWidget(7)->SetBorderWidth(2);
  this->ViewAxisChoiceButton->GetWidget(7)->SetBalloonHelpString("Parallel Projection Viewing");
  this->ViewAxisChoiceButton->GetWidget(7)->SetCommand(this, "ParallelViewCallback");
  vtkKWIcon *parallelIcon = vtkKWIcon::New();
  parallelIcon->SetImage(  image_mimxParallel, 
                       image_mimxParallel_width, 
                       image_mimxParallel_height, 
                       image_mimxParallel_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(7)->SetImageToIcon( parallelIcon );
  
  this->ViewAxisChoiceButton->AddWidget(8);
  this->ViewAxisChoiceButton->GetWidget(8)->SetBorderWidth(2);
  this->ViewAxisChoiceButton->GetWidget(8)->SetBalloonHelpString("Fit to View");
  this->ViewAxisChoiceButton->GetWidget(8)->SetCommand(this, "ResetViewCallback");
  vtkKWIcon *fitIcon = vtkKWIcon::New();
  fitIcon->SetImage(  image_mimxAutofit, 
                       image_mimxAutofit_width, 
                       image_mimxAutofit_height, 
                       image_mimxAutofit_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(8)->SetImageToIcon( fitIcon );
  
  this->ViewAxisChoiceButton->AddWidget(9);
  this->ViewAxisChoiceButton->GetWidget(9)->SetBorderWidth(2);
  this->ViewAxisChoiceButton->GetWidget(9)->SetBalloonHelpString("Take Snapshot");
  this->ViewAxisChoiceButton->GetWidget(9)->SetCommand(this, "WindowShapshot");
  vtkKWIcon *snapshotIcon = vtkKWIcon::New();
  snapshotIcon->SetImage(  image_mimxSnapshot, 
                       image_mimxSnapshot_width, 
                       image_mimxSnapshot_height, 
                       image_mimxSnapshot_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(9)->SetImageToIcon( snapshotIcon );
  
  this->ViewAxisChoiceButton->AddWidget(10);
  this->ViewAxisChoiceButton->GetWidget(10)->SetBorderWidth(2);
  this->ViewAxisChoiceButton->GetWidget(10)->SetBalloonHelpString("Show Axes");
  this->ViewAxisChoiceButton->GetWidget(10)->SetCommand(this, "ShowLocalAxesActor");
  vtkKWIcon *axisIcon = vtkKWIcon::New();
  axisIcon->SetImage(  image_mimxAxis, 
                       image_mimxAxis_width, 
                       image_mimxAxis_height, 
                       image_mimxAxis_pixel_size);
  this->ViewAxisChoiceButton->GetWidget(10)->SetImageToIcon( axisIcon );
  
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
          this->ViewAxisChoiceButton->GetWidgetName());
        
        vtkKWIcon *closeIcon = vtkKWIcon::New();
  closeIcon->SetImage( image_mimxClose, 
                       image_mimxClose_width, 
                       image_mimxClose_height, 
                       image_mimxClose_pixel_size);
                         
        if(!this->CloseButton)
  {
    this->CloseButton = vtkKWPushButton::New();
  }
  this->CloseButton->SetParent( this );
  this->CloseButton->Create();
  //this->CloseButton->SetText("Close");
  this->CloseButton->SetImageToIcon( closeIcon );
  this->CloseButton->SetReliefToFlat( );
  this->CloseButton->SetCommand(this, "Withdraw");
  this->GetApplication()->Script(
          "pack %s -side right -anchor ne -expand y -padx 2 -pady 6", 
          this->CloseButton->GetWidgetName());

  this->AddBinding("<Return>", this, "Withdraw");
  this->AddBinding("<Escape>", this, "Withdraw");
          
}
//----------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::SetBackGroundColor(double color[3])
{
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->
                SetBackground(this->BackGroundColorButton->GetColor());
        this->GetMimxMainWindow()->GetRenderWidget()->Render();
        
  this->GetMimxMainWindow()->SetBackgroundColor(color);
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::WindowShapshot()
{
        if(!this->LoadSaveDialog)
        {
                this->LoadSaveDialog = vtkKWLoadSaveDialog::New() ;
                this->LoadSaveDialog->SetApplication(this->GetApplication());
                this->LoadSaveDialog->SaveDialogOn();
                this->LoadSaveDialog->Create();
        }
        this->LoadSaveDialog->SetTitle ("Save Window snap shot");
        //      this->LoadSaveDialog->SetFileTypes ("{{JPEG Image} {.jpeg}} {{BMP Image} {.bmp}} {{PNG Image} {.png}} {{TIFF Image} {.tif}} {{PostScript Image} {.ps}}");
        this->LoadSaveDialog->SetFileTypes ("{{JPEG Image} {.jpeg}}");
        this->LoadSaveDialog->SetDefaultExtension (".jpeg");
        this->LoadSaveDialog->RetrieveLastPathFromRegistry("LastPath");
        this->LoadSaveDialog->Invoke();

        if(this->LoadSaveDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                if(this->LoadSaveDialog->GetFileName())
                {
                        const char *filename = this->LoadSaveDialog->GetFileName();
                        vtkWindowToImageFilter *imageout = vtkWindowToImageFilter::New();
                        imageout->SetInput(this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindow());
                        vtkJPEGWriter *imagewriter = vtkJPEGWriter::New();
                        imagewriter->SetInput(imageout->GetOutput());
                        imagewriter->SetFileName(filename);
                        imagewriter->Write();
                        imagewriter->Delete();
                        imageout->Delete();
                }
        }

}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::PositiveXViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
        vtkRenderer *ren = this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i);
        vtkCamera *camera = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;

          camera->SetViewUp(0,1,0);
          camera->SetPosition(1,0,0);
          camera->SetFocalPoint(0.0, 0.0, 0.0);
          camera->ComputeViewPlaneNormal();
    camera->OrthogonalizeViewUp();
    this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i)->ResetCamera();    
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::PositiveYViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
        vtkRenderer *ren = this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i);
        vtkCamera *camera = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;
          camera->SetViewUp(0,0,1);
          camera->SetPosition(0,1,0);
          camera->SetFocalPoint(0.0, 0.0, 0.0);
          camera->ComputeViewPlaneNormal();
    camera->OrthogonalizeViewUp();
    this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i)->ResetCamera();      
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::PositiveZViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
        vtkRenderer *ren = this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i);
        vtkCamera *camera = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;
           camera->SetViewUp(0,1,0);
           camera->SetPosition(0,0,1);
           camera->SetFocalPoint(0.0, 0.0, 0.0);
           camera->ComputeViewPlaneNormal();
     camera->OrthogonalizeViewUp();
     this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i)->ResetCamera();    
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::NegativeXViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
        vtkRenderer *ren = this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i);
        vtkCamera *camera = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;
          camera->SetViewUp(0,1,0);
        camera->SetPosition(-1,0,0);
        camera->SetFocalPoint(0.0, 0.0, 0.0);
          camera->ComputeViewPlaneNormal();
    camera->OrthogonalizeViewUp();
    this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i)->ResetCamera();    
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::NegativeYViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
        vtkRenderer *ren = this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i);
        vtkCamera *camera = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;
          camera->SetViewUp(0,0,1);
          camera->SetPosition(0,-1,0);
          camera->SetFocalPoint(0.0, 0.0, 0.0);
          camera->ComputeViewPlaneNormal();
    camera->OrthogonalizeViewUp();
    this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i)->ResetCamera();    
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::NegativeZViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
        vtkRenderer *ren = this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i);
        vtkCamera *camera = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;
          camera->SetViewUp(0,1,0);
          camera->SetPosition(0,0,-1);
          camera->SetFocalPoint(0.0, 0.0, 0.0);
          camera->ComputeViewPlaneNormal();
    camera->OrthogonalizeViewUp();
    this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i)->ResetCamera();    
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::PerspectiveViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
        vtkRenderer *ren = this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i);
        vtkCamera *camera = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;
          camera->ParallelProjectionOff();
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::ParallelViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
        vtkRenderer *ren = this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i);
        vtkCamera *camera = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;
          camera->ParallelProjectionOn();
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::ResetViewCallback()
{
        for (int i=0;i<this->GetMimxMainWindow()->GetRenderWidget()->GetNumberOfRenderers();i++)
        {
    this->GetMimxMainWindow()->GetRenderWidget()->GetNthRenderer(i)->ResetCamera();    
        }
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::PrintSelf(ostream& os, vtkIndent indent)
{
        this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::ShowLocalAxesActor( )
{
        vtkPVAxesActor *axisActor = this->GetMimxMainWindow()->GetPVAxesActor();
        if ( axisActor->GetVisibility( ) ) 
          axisActor->SetVisibility( 0 );
        else
          axisActor->SetVisibility( 1 );
        this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();    
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::SetTextColor( double *color )
{
        TextColor[0] =  color[0];
        TextColor[1] =  color[1];
        TextColor[2] =  color[2];

        vtkPropCollection *viewPropList = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->GetViewProps();
        //std::cout <<  viewPropList->GetNumberOfItems() << std::endl;  
        viewPropList->InitTraversal();
        for (int i=0;i<viewPropList->GetNumberOfItems();i++)
        {
          vtkProp *currentProp = viewPropList->GetNextProp();
          /* May Need to Add other Actors in Addition to the ScalarBarActor */
          if ( strcmp(currentProp->GetClassName(), "vtkScalarBarActor") == 0 )
          {
          vtkScalarBarActor *tmpActor = (vtkScalarBarActor *) currentProp;
          vtkTextProperty* textProperty = tmpActor->GetLabelTextProperty();
          textProperty->SetColor(TextColor);
          tmpActor->SetLabelTextProperty( textProperty );
          textProperty = tmpActor->GetTitleTextProperty();
          textProperty->SetColor(TextColor);
          tmpActor->SetTitleTextProperty( textProperty );
          }
        }
        
        vtkPVAxesActor *axisActor = this->GetMimxMainWindow()->GetPVAxesActor();
        vtkProperty* labelProperty = axisActor->GetXAxisLabelProperty();
        labelProperty->SetColor(TextColor);
  axisActor->SetXAxisLabelProperty( labelProperty );
  axisActor->SetYAxisLabelProperty( labelProperty );
  axisActor->SetZAxisLabelProperty( labelProperty );
  
  this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor()->Render();
  
  this->GetMimxMainWindow()->SetTextColor(TextColor);
                          
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDisplayPropertiesGroup::SetTextColor( double red, double green, double blue )
{
        double newColor[3];
        
        newColor[0] =  red;
        newColor[1] =  green;
        newColor[2] =  blue;
        
        this->SetTextColor( newColor );   
}
//------------------------------------------------------------------------------------------------------
double* vtkKWMimxDisplayPropertiesGroup::GetTextColor( )
{
        return TextColor;    
}
//------------------------------------------------------------------------------------------------------
