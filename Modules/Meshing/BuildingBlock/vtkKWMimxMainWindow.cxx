/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMainWindow.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.24.4.2 $

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

#include "vtkKWMimxMainWindow.h"

#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataWriter.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridWriter.h"

#include "vtkKWApplication.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWMenu.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWApplicationSettingsInterface.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWScale.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWSeparator.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"

#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkMimxUnstructuredGridActor.h"

#include "vtkSlicerModuleCollapsibleFrame.h"

//#include "vtkKWMimxApplication.h"
//#include "vtkKWMimxApplicationSettingsInterface.h"
#include "vtkKWMimxDisplayPropertiesGroup.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkKWMimxViewProperties.h"

#include "vtkSlicerTheme.h"
#include "vtkKWFrameWithScrollbar.h"

#include "vtkLinkedListWrapperTree.h"
#include "vtkPVAxesActor.h"

#include <vtksys/SystemTools.hxx>

#include "Resources/mimxApply.h"
#include "Resources/mimxAutofit.h"
#include "Resources/mimxCancel.h"
#include "Resources/mimxNegX.h"
#include "Resources/mimxNegY.h"
#include "Resources/mimxNegZ.h"
#include "Resources/mimxParallel.h"
#include "Resources/mimxPerspective.h"
#include "Resources/mimxPosX.h"
#include "Resources/mimxPosY.h"
#include "Resources/mimxPosZ.h"
#include "Resources/mimxSnapshot.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMimxMainWindow );

vtkCxxRevisionMacro(vtkKWMimxMainWindow, "$Revision: 1.24.4.2 $");
//----------------------------------------------------------------------------
vtkKWMimxMainWindow::vtkKWMimxMainWindow()
{
  this->MainWindow = NULL;
  this->RenderWidget = NULL;
  this->PVAxesActor = NULL;
  this->AxesRenderer = NULL;
  this->CallbackCommand = NULL;
  this->ErrorCallback = vtkMimxErrorCallback::New();
  //this->ViewMenu = NULL;
  this->ViewProperties = NULL;
  this->DoUndoTree = NULL;
  this->MainUserInterfacePanel = NULL;
  this->DisplayPropertyDialog = NULL;
  
  /* Default Icons for Apply/Cancel Buttons */
  this->applyIcon = vtkKWIcon::New();
  this->applyIcon->SetImage( image_mimxApply, 
                             image_mimxApply_width, 
                             image_mimxApply_height, 
                             image_mimxApply_pixel_size);
  
  this->cancelIcon = vtkKWIcon::New();
  this->cancelIcon->SetImage( image_mimxCancel, 
                              image_mimxCancel_width, 
                              image_mimxCancel_height, 
                              image_mimxCancel_pixel_size); 
                              
  /* Widgets used to define the Application Settings */
  this->MimxSettingsFrame = NULL;
  this->AutoSaveButton = NULL;
  this->AutoSaveScale = NULL;
  this->WorkingDirButton = NULL;
  this->AutoSaveDir = NULL;
  this->AverageElementLengthEntry = NULL;
  this->PropertyPrecisionScale = NULL;
  this->FontSettingsFrame = NULL;
  this->FontSizeButtons = NULL;
  this->FontFamilyButtons  = NULL; 
  
  /* Initialize the Default Application Settings */
  /*
   * TODO: these are disabled for use inside slicer
  vtkKWFrameWithLabel::SetDefaultLabelFontWeightToNormal( );
  this->SlicerTheme = vtkSlicerTheme::New( );
  strcpy ( this->ApplicationFontSize,   "small" );
  strcpy ( this->ApplicationFontFamily, "Arial" );
  */
  this->SlicerTheme = NULL;
  
  /* Default Auto Save Settings */
  this->AutoSaveFlag = true;
  this->AutoSaveTime = 5;
  this->AutoSaveWorkDirFlag = true;
  this->AverageElementLength = 1.0;
  this->ABAQUSPrecision = 2;
  strcpy(this->WorkingDirectory,"");
  std::string tmpDate =  "Mesh-" + vtksys::SystemTools::GetCurrentDateTime("%Y%m%d-%H%M");
  strcpy(this->DateTimeString, tmpDate.c_str()); 
  
#if defined(WIN32)
  strcpy(this->AutoSaveDirectory,"/tmp");
#else
  strcpy(this->AutoSaveDirectory,"C:/Windows/Temp");
#endif

  /* Set the Default Render Window Colors */
  this->TextColor[0] = this->TextColor[1] = this->TextColor[2] = 1.0;
  this->BackgroundColor[0] = this->BackgroundColor[1] = this->BackgroundColor[2] = 0.0;
  
  /* The default is a standalone application */
  this->StandAloneApplication = false;  
        

}

//----------------------------------------------------------------------------
vtkKWMimxMainWindow::~vtkKWMimxMainWindow()
{
  if (this->AxesRenderer)
    { 
    this->RemoveOrientationAxis();
    this->AxesRenderer->Delete();
    }
  if (this->PVAxesActor)
    this->PVAxesActor->Delete();
  if (this->CallbackCommand)
    this->CallbackCommand->Delete();
  if (this->ErrorCallback)
    this->ErrorCallback->Delete();
  // Is this needed now???
  if(this->MainWindow)
        this->MainWindow->Delete();
  if(this->RenderWidget)
          this->RenderWidget->Delete();
  if(this->ViewProperties)
    this->ViewProperties->Delete();
  if(this->DoUndoTree)
    this->DoUndoTree->Delete();
  if(this->MainUserInterfacePanel)
    this->MainUserInterfacePanel->Delete();
  if (this->DisplayPropertyDialog )
    this->DisplayPropertyDialog->Delete();
  if (this->applyIcon)
    this->applyIcon->Delete();
  if (this->cancelIcon)
    this->cancelIcon->Delete();
  if (this->MimxSettingsFrame)
    this->MimxSettingsFrame->Delete();
  if (this->AutoSaveButton)
    this->AutoSaveButton->Delete();
  if (this->AutoSaveScale)
    this->AutoSaveScale->Delete();
  if (this->WorkingDirButton)
    this->WorkingDirButton->Delete();
  if (this->AutoSaveDir)
    this->AutoSaveDir->Delete();
  if (this->AverageElementLengthEntry)
    this->AverageElementLengthEntry->Delete();
  if (this->PropertyPrecisionScale)
    this->PropertyPrecisionScale->Delete();
  if (this->FontSettingsFrame)
    this->FontSettingsFrame->Delete();
  if (this->FontSizeButtons)
    this->FontSizeButtons->Delete();
  if (this->FontFamilyButtons)
    this->FontFamilyButtons->Delete();
}



//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::AddOrientationAxis()
{
// creation of axes representation
   // we need two renderers
   if ( this->RenderWidget )
   {
       if(!this->AxesRenderer)
         {
         this->AxesRenderer = vtkRenderer::New();
         this->PVAxesActor = vtkPVAxesActor::New();
         this->CallbackCommand = vtkCallbackCommand::New();
         this->CallbackCommand->SetCallback(updateAxis);
         this->CallbackCommand->SetClientData(this);
         this->RenderWidget->GetRenderer()->AddObserver(vtkCommand::AnyEvent,this->CallbackCommand);
         }
       this->AxesRenderer->InteractiveOff();
       this->RenderWidget->GetRenderWindow()->SetNumberOfLayers(2);
       this->RenderWidget->GetRenderer()->SetLayer(0);
       this->AxesRenderer->SetLayer(1);
       this->AxesRenderer->SetViewport(0.0,0.0,0.25,0.25);
       this->AxesRenderer->AddActor(this->PVAxesActor);
       this->RenderWidget->GetRenderWindow()->AddRenderer(this->AxesRenderer);
       this->RenderWidget->Render();
   }
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::RemoveOrientationAxis()
{
// creation of axes representation
   // we need two renderers
   if ( this->RenderWidget && this->AxesRenderer)
   {
       this->AxesRenderer->InteractiveOff();
       this->RenderWidget->GetRenderer()->SetLayer(0);
       this->AxesRenderer->SetLayer(1);
       this->RenderWidget->GetRenderer()->RemoveObserver(this->CallbackCommand);
       this->RenderWidget->GetRenderWindow()->RemoveRenderer(this->AxesRenderer);
       this->RenderWidget->Render();
       this->AxesRenderer->Delete();
       this->PVAxesActor->Delete();
       this->AxesRenderer = NULL;
       this->CallbackCommand->Delete();
   }
}


void vtkKWMimxMainWindow::CustomApplicationSettingsModuleEntry()
{
    // Custom Configuration of the Toolbar Menu
    if ( this->StandAloneApplication )
      {
            vtkKWMenu* fileMenu = this->MainWindow->GetFileMenu();
        fileMenu->DeleteItem (0);
        
        vtkKWMenu* viewMenu = this->MainWindow->GetViewMenu();
        viewMenu->AddCommand("View Settings", this, "DisplayPropertyCallback");
        viewMenu->AddSeparator( );
        
        vtkKWMenu *fontSizeMenu = vtkKWMenu::New();
        fontSizeMenu->SetParent(viewMenu);
        fontSizeMenu->Create();
        fontSizeMenu->AddRadioButton("small", this, "SetApplicationFontSize small");
        fontSizeMenu->AddRadioButton("medium", this, "SetApplicationFontSize medium");
        fontSizeMenu->AddRadioButton("large", this, "SetApplicationFontSize large");
        fontSizeMenu->AddRadioButton("largest", this, "SetApplicationFontSize largest");
        viewMenu->AddCascade("Font Size", fontSizeMenu);
        
        vtkKWMenu *fontTypeMenu = vtkKWMenu::New();
        fontTypeMenu->SetParent(viewMenu);
        fontTypeMenu->Create();
        fontTypeMenu->AddRadioButton("Arial", this, "SetApplicationFontFamily Arial");
        fontTypeMenu->AddRadioButton("Helvetica", this, "SetApplicationFontFamily Helvetica");
        fontTypeMenu->AddRadioButton("Verdana", this, "SetApplicationFontFamily Verdana");
        viewMenu->AddCascade("Font Family", fontTypeMenu);
        
        fontSizeMenu->SelectItem( this->GetApplicationFontSize() );
        fontTypeMenu->SelectItem( this->GetApplicationFontFamily() );
        
        this->AddFontApplicationSettingsPanel();
      }

    if (this->MimxSettingsFrame == NULL)
    {
      this->AddCustomApplicationSettingsPanel();
    }      
}


//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::AddCustomApplicationSettingsPanel()
{

  vtkKWApplicationSettingsInterface *applicationMenu = 
           this->MainWindow->GetApplicationSettingsInterface( ); 

  ostrstream tk_cmd;
  vtkKWWidget *page;
  vtkKWFrame *frame;

  // --------------------------------------------------------------
  // Add a "Preferences" page

  int id = applicationMenu->AddPage( "IA-FEMesh" );
  page = applicationMenu->GetPageWidget(id);
                                                              
  // --------------------------------------------------------------
  // IA-FEMesh Interface settings : main frame
  if (this->MimxSettingsFrame == NULL)
  {
    this->MimxSettingsFrame = vtkKWFrameWithLabel::New();
  }
  this->MimxSettingsFrame->SetParent(applicationMenu->GetPagesParentWidget());
  this->MimxSettingsFrame->Create();
  this->MimxSettingsFrame->SetLabelText("IA-FEMesh Settings");

  tk_cmd << "pack " << this->MimxSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " 
         << " -in " << page->GetWidgetName() << endl;
  
  frame = this->MimxSettingsFrame->GetFrame();
  
  if ( this->StandAloneApplication )
  {
    if (this->AutoSaveButton == NULL)
    {
      this->AutoSaveButton = vtkKWCheckButtonWithLabel::New();
    }
    this->AutoSaveButton->SetParent ( frame );
    this->AutoSaveButton->Create();
    this->AutoSaveButton->SetLabelText ("Autosave Work:");
    this->AutoSaveButton->GetWidget()->SetCommand ( this, "AutoSaveModeCallback");
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2",
                   this->AutoSaveButton->GetWidgetName());
    
    if (this->AutoSaveScale == NULL)
    {
      this->AutoSaveScale = vtkKWScaleWithLabel::New();
    }
    this->AutoSaveScale->SetParent ( frame );
    this->AutoSaveScale->Create();
    this->AutoSaveScale->SetLabelText ("Interval (Minutes):");
    this->AutoSaveScale->GetWidget()->SetRange ( 1.0, 20.0);
    this->AutoSaveScale->GetWidget()->SetResolution ( 1.0 );
    this->AutoSaveScale->GetWidget()->SetCommand ( this, "AutoSaveScaleCallback");
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                   this->AutoSaveScale->GetWidgetName());  
    
    if (this->WorkingDirButton == NULL)
    {
      this->WorkingDirButton = vtkKWCheckButtonWithLabel::New();
    }
    this->WorkingDirButton->SetParent ( frame );
    this->WorkingDirButton->Create();
    this->WorkingDirButton->SetLabelText ("Use Working Directory:");
    this->WorkingDirButton->GetWidget()->SetCommand ( this, "AutoSaveDirectoryModeCallback");
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2",
                   this->WorkingDirButton->GetWidgetName());
                   
    if (this->AutoSaveDir == NULL)
    {
      this->AutoSaveDir = vtkKWLoadSaveButtonWithLabel::New();
    }
    this->AutoSaveDir->SetParent ( frame );
    this->AutoSaveDir->Create();
    this->AutoSaveDir->SetLabelText ("Autosave Directory:");
    this->AutoSaveDir->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();
    this->AutoSaveDir->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
    this->AutoSaveDir->GetWidget()->TrimPathFromFileNameOff();
    this->AutoSaveDir->GetWidget()->SetCommand ( this, "AutoSaveDirectoryCallback");
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                   this->AutoSaveDir->GetWidgetName()); 
    
    vtkKWSeparator *separator = vtkKWSeparator::New();
    separator->SetParent ( frame );
    separator->Create();
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                   separator->GetWidgetName());
    separator->Delete(); 
  }
  
  if (this->AverageElementLengthEntry == NULL)
  {
    this->AverageElementLengthEntry = vtkKWEntryWithLabel::New();
  }
  this->AverageElementLengthEntry->SetParent ( frame );
  this->AverageElementLengthEntry->Create();
  this->AverageElementLengthEntry->SetLabelText("Average Element Length:");
  this->AverageElementLengthEntry->GetWidget()->SetRestrictValueToDouble( );
  this->AverageElementLengthEntry->GetWidget()->SetCommand ( this, "AverageElementLengthCallback");
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                 this->AverageElementLengthEntry->GetWidgetName()); 
  
  if (this->PropertyPrecisionScale == NULL)
  {
    this->PropertyPrecisionScale = vtkKWScaleWithLabel::New();
  }
  this->PropertyPrecisionScale->SetParent ( frame );
  this->PropertyPrecisionScale->Create();
  this->PropertyPrecisionScale->SetLabelText("ABAQUS Material Property Precision:");
  this->PropertyPrecisionScale->GetWidget()->SetRange(0.0, 10.0);
  this->PropertyPrecisionScale->GetWidget()->SetResolution(1.0);
  this->PropertyPrecisionScale->GetWidget()->SetCommand( this, "ABAQUSPrecisionCallback");
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                 this->PropertyPrecisionScale->GetWidgetName());
                 
  
  
  // --------------------------------------------------------------
  // Pack 

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);

  // Update

  this->Update();
}


void vtkKWMimxMainWindow::CustomApplicationSettingsModuleExit()
{
    // Custom Configuration of the Toolbar Menu
    if ( this->StandAloneApplication )
      {      
        this->RemoveFontApplicationSettingsPanel();
      }

    this->RemoveCustomApplicationSettingsPanel();     
}



//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::CreateWidget()
{
        // setting the position and size of the window
        if(this->IsCreated())
        {
                vtkErrorMacro("Class already created");
                return;
        }
        this->Superclass::CreateWidget();
  
   
        this->ErrorCallback->SetKWApplication(this->GetApplication());
        // for do and undo tree
        if(!this->DoUndoTree)
        {
                this->DoUndoTree = vtkLinkedListWrapperTree::New();
  }
  
    //this->MainFrame = vtkSlicerModuleCollapsibleFrame::New();
    //this->MainFrame->SetParent(this);
    //this->MainFrame->Create();
    //this->MainFrame->AllowFrameToCollapseOn();
    //this->GetApplication()->Script(
    //      "pack %s -side top -anchor nw -expand yes -padx 2 -pady 2 -fill both", 
    //      this->MainFrame->GetWidgetName());
    
        if(!this->ViewProperties)
        {
                this->ViewProperties = vtkKWMimxViewProperties::New();
        }
        this->ViewProperties->SetParent(this/*->MainFrame->GetFrame()*/);
        this->ViewProperties->SetMimxMainWindow(this);
        this->ViewProperties->Create();
        //this->ViewProperties->SetBorderWidth(2);
        this->ViewProperties->SetReliefToGroove();
        this->ViewProperties->SetDoUndoTree(this->DoUndoTree);
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand no -fill x", 
                this->ViewProperties->GetMainFrame()->GetWidgetName());

        // add tabs
        if(!this->MainUserInterfacePanel)
                this->MainUserInterfacePanel = vtkKWMimxMainNotebook::New();
        this->MainUserInterfacePanel->SetMimxMainWindow(this);
        this->MainUserInterfacePanel->SetDoUndoTree(this->DoUndoTree);
        //this->MainUserInterfacePanel->SetMultiColumnList(
        //      this->ViewProperties->GetMultiColumnList());
        this->MainUserInterfacePanel->SetParent( this/*->MainFrame->GetFrame()*/ );
        this->MainUserInterfacePanel->SetApplication(this->GetApplication());
        this->MainUserInterfacePanel->Create();
        this->MainUserInterfacePanel->SetBorderWidth(3);
        this->MainUserInterfacePanel->SetReliefToGroove();
        //this->MainUserInterfacePanel->GetMainFrame()->ExpandFrame();
        this->GetApplication()->Script(
                "pack %s -side top -anchor nw -expand yes -pady 2 -fill both", 
                this->MainUserInterfacePanel->GetWidgetName());
        
  
  this->Update();
}
//----------------------------------------------------------------------------------------------
void updateAxis(vtkObject* vtkNotUsed(caller), unsigned long , void* arg, void* ) {
        double    cPos[3], cFoc[3], aFoc[3];

        // set the axis camera according to the main renderer.
        vtkKWMimxMainWindow *MimxMainWindow = (vtkKWMimxMainWindow *)arg;
        vtkKWRenderWidget *renderWidget = MimxMainWindow->RenderWidget;
        if (renderWidget)
        {
          vtkRenderer *ren = MimxMainWindow->RenderWidget->GetRenderer();
          vtkCamera *cam = ren->IsActiveCameraCreated() ? ren->GetActiveCamera() : NULL;
          if (cam)
          {
            cam->GetPosition(cPos);
            cam->GetFocalPoint(cFoc);
            vtkCamera *axes_cam = MimxMainWindow->AxesRenderer->IsActiveCameraCreated() ? MimxMainWindow->AxesRenderer->GetActiveCamera() : NULL;
            if (axes_cam)
            {
              axes_cam->GetFocalPoint(aFoc);
              axes_cam->SetViewUp(cam->GetViewUp());
              axes_cam->SetPosition(cPos[0] - cFoc[0] +
                aFoc[0],\
                cPos[1] - cFoc[1] + aFoc[1],\
                cPos[2] - cFoc[2] + aFoc[2]);
          MimxMainWindow->AxesRenderer->ResetCamera();
            }
          }
        }
}
//----------------------------------------------------------------------------------------------
void vtkKWMimxMainWindow::DisplayPropertyCallback()
{
        if (!this->DisplayPropertyDialog)
        {
          this->DisplayPropertyDialog = vtkKWMimxDisplayPropertiesGroup::New();
          this->DisplayPropertyDialog->SetMimxMainWindow(this);
          this->DisplayPropertyDialog->SetApplication(this->GetApplication());
          this->DisplayPropertyDialog->Create();
        }
        this->DisplayPropertyDialog->Display();
        
}

//----------------------------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetApplicationFontFamily( const char *font )
{
  char localFont[32];
  strcpy(localFont, font);
    
  if ( this->SlicerTheme )
    {
    this->SlicerTheme->SetFontFamily ( localFont );
    this->Script ( "font configure %s -family %s", this->SlicerTheme->GetApplicationFont2(), localFont );
    this->Script ( "font configure %s -family %s", this->SlicerTheme->GetApplicationFont1(), localFont );
    this->Script ( "font configure %s -family %s", this->SlicerTheme->GetApplicationFont0(), localFont );
    strcpy ( this->ApplicationFontFamily, localFont );
    this->GetApplication()->SetRegistryValue(1, "Font", "Family", localFont);
    }
}

//----------------------------------------------------------------------------
const char *vtkKWMimxMainWindow::GetApplicationFontFamily () const
{
  return this->ApplicationFontFamily;
}

//----------------------------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetApplicationFontSize( const char *size )
{
  char localSize[32];
  strcpy(localSize, size);
  
  if (this->SlicerTheme)
    {
    vtkSlicerFont *font = this->SlicerTheme->GetSlicerFonts();
    if ( font )
      {
      // check to see if m has a valid value:
      if ( font->IsValidFontSize ( localSize ) )
        {
        int f2 = font->GetFontSize2( localSize );
        int f1 = font->GetFontSize1( localSize );
        int f0 = font->GetFontSize0( localSize );
        
        this->Script ( "font configure %s -size %d", this->SlicerTheme->GetApplicationFont2(), f2);
        this->Script ( "font configure %s -size %d", this->SlicerTheme->GetApplicationFont1(), f1);
        this->Script ( "font configure %s -size %d", this->SlicerTheme->GetApplicationFont0(), f0);
        
        strcpy (this->ApplicationFontSize, localSize );
        this->GetApplication()->SetRegistryValue(1, "Font", "Size", localSize);
        }
      }
    }
}

//----------------------------------------------------------------------------
const char *vtkKWMimxMainWindow::GetApplicationFontSize () const
{
  return this->ApplicationFontSize;
}

//----------------------------------------------------------------------------------------------
vtkKWIcon* vtkKWMimxMainWindow::GetApplyButtonIcon( )
{
  return this->applyIcon;
}

//----------------------------------------------------------------------------------------------
vtkKWIcon* vtkKWMimxMainWindow::GetCancelButtonIcon( )
{
  return this->cancelIcon;
}

//----------------------------------------------------------------------------------------------
void vtkKWMimxMainWindow::AutoSaveCallback( )
{
  //std::cout << "Auto Save" << std::endl;
  vtkKWMimxMainNotebook *mimxNotebook = this->MainUserInterfacePanel;
  vtkLinkedListWrapper *meshList = mimxNotebook->GetFEMeshMenuGroup()->GetFEMeshList();
  vtkLinkedListWrapper *bbList = mimxNotebook->GetFEMeshMenuGroup()->GetBBoxList();
  vtkLinkedListWrapper *surfaceList = mimxNotebook->GetSurfaceMenuGroup()->GetSurfaceList();

  int count = surfaceList->GetNumberOfItems();
  count += meshList->GetNumberOfItems();
  count += bbList->GetNumberOfItems();
  //std::cout << "Number of Items: " << count << std::endl;
  if (count > 0 )
  {
    bool saveFlag = true;
    std::string autoSaveDir = this->GetSaveDirectory();
    //std::cout << "Auto save Directory: " << autoSaveDir << std::endl;
    bool dirFlag = vtksys::SystemTools::FileExists(autoSaveDir.c_str(), false);
    if ( ! dirFlag )
    {
      bool createFlag = vtksys::SystemTools::MakeDirectory(autoSaveDir.c_str());
      if (!createFlag)
      {
        vtkMimxErrorCallback *callback = this->GetErrorCallback();
        std::string errorMsg = "Failed to create directory for automatic saving of work.\n";
        errorMsg += "Either change this directory or turn off the auto save feature.\n";
        errorMsg += "This can be done via the Application Settings.\n";
        callback->ErrorMessage( errorMsg.c_str() );
        saveFlag = false;
      }
    }
    
    if ( saveFlag )
    {
      for (int i = 0; i < surfaceList->GetNumberOfItems(); i++)
      {
                vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(surfaceList->GetItem(i))->GetDataSet();
         
                std::string filename = surfaceList->GetItem(i)->GetFileName();
                std::string filePath = autoSaveDir + "/" + 
                            vtksys::SystemTools::GetFilenameWithoutExtension( filename ) + ".vtk";
                        
                vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
                writer->SetFileName( filePath.c_str() );
                writer->SetInput(polydata);
                writer->Update();
                writer->Delete();       
      }
      
      for (int i = 0; i < meshList->GetNumberOfItems(); i++)
      {
                vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(meshList->GetItem(i))->GetDataSet();
          
                std::string filename = meshList->GetItem(i)->GetFileName();
                std::string filePath = autoSaveDir + "/" + 
                            vtksys::SystemTools::GetFilenameWithoutExtension(filename) + ".vtk";
                        
                vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
                writer->SetFileName( filePath.c_str() );
                writer->SetInput(ugrid);
                writer->Update();
                writer->Delete();
      }
       
      for (int i = 0; i < bbList->GetNumberOfItems(); i++)
      {
                vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(bbList->GetItem(i))->GetDataSet();
          
                std::string filename = bbList->GetItem(i)->GetFileName();
                std::string filePath = autoSaveDir + "/" + 
                            vtksys::SystemTools::GetFilenameWithoutExtension( filename ) + ".vtk";
        
                vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
                writer->SetFileName( filePath.c_str() );
                writer->SetInput(ugrid);
                writer->Update();
                writer->Delete();
      }
    }
  }
  
  if ( this->GetAutoSaveFlag() )
  {
    const char *tmpId = vtkKWTkUtilities::CreateTimerHandler(this->GetApplication()->GetMainInterp(),
                                         this->GetAutoSaveTime()*1000*60,  /* Convert minutes to ms */
                                         this,
                                         "AutoSaveCallback");
    strcpy(this->autoSaveEventId, tmpId);
  }                                     
}

//----------------------------------------------------------------------------------------------
void vtkKWMimxMainWindow::EnableAutoSave( )
{
  const char *tmpId = 
    vtkKWTkUtilities::CreateTimerHandler(this->GetApplication()->GetMainInterp(),
                                        this->GetAutoSaveTime()*1000*60,  /* Convert minutes to ms */
                                        this,
                                        "AutoSaveCallback");
  strcpy(this->autoSaveEventId, tmpId);
}

//----------------------------------------------------------------------------------------------
void vtkKWMimxMainWindow::DisableAutoSave( )
{
  vtkKWTkUtilities::CancelTimerHandler (this->GetApplication()->GetMainInterp(), this->autoSaveEventId);
}

//----------------------------------------------------------------------------
/*
vtkKWApplicationSettingsInterface* 
vtkKWMimxMainWindow::GetApplicationSettingsInterface()
{
  // If not created, create the application settings interface, connect it
  // to the current window, and manage it with the current interface manager.

  if (!this->ApplicationSettingsInterface)
    {
    this->ApplicationSettingsInterface = 
      vtkKWMimxApplicationSettingsInterface::New();
    this->ApplicationSettingsInterface->SetWindow(this);
    this->ApplicationSettingsInterface->SetUserInterfaceManager(
      this->GetApplicationSettingsUserInterfaceManager());
    }
  return this->ApplicationSettingsInterface;
}
*/
//----------------------------------------------------------------------------
double vtkKWMimxMainWindow::GetAverageElementLength()
{
  return this->AverageElementLength;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetAverageElementLength(double length)
{
  this->AverageElementLength = length;
  this->GetApplication()->SetRegistryValue(1, "MeshProperties", "AverageElementLength", "%f", this->AverageElementLength);
}

//----------------------------------------------------------------------------
int vtkKWMimxMainWindow::GetABAQUSPrecision()
{
  return this->ABAQUSPrecision;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetABAQUSPrecision(int precision)
{
  this->ABAQUSPrecision = precision;
  this->GetApplication()->SetRegistryValue(1, "MeshProperties", "ABAQUSPrecision", "%d", this->ABAQUSPrecision);
}

//----------------------------------------------------------------------------
double *vtkKWMimxMainWindow::GetTextColor( )
{
  return this->TextColor;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetTextColor( double color[3] )
{
  this->TextColor[0] = color[0];
  this->TextColor[1] = color[1];
  this->TextColor[2] = color[2];
  
  this->GetApplication()->SaveColorRegistryValue(1, "ViewerText", this->TextColor);
}

//----------------------------------------------------------------------------
double *vtkKWMimxMainWindow::GetBackgroundColor( )
{
  return this->BackgroundColor;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetBackgroundColor( double color[3] )
{
  this->BackgroundColor[0] = color[0];
  this->BackgroundColor[1] = color[1];
  this->BackgroundColor[2] = color[2];
  
  this->GetApplication()->SaveColorRegistryValue(1, "ViewerBackground", this->BackgroundColor);
}

//----------------------------------------------------------------------------
const char *vtkKWMimxMainWindow::GetWorkingDirectory()
{
  return this->WorkingDirectory;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetWorkingDirectory( const char *dirName )
{
  strcpy(this->WorkingDirectory, dirName);
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::InitializeWorkingDirectory( const char *dirName )
{
  if ( strlen(this->WorkingDirectory) == 0 ) 
  {
    SetWorkingDirectory(dirName);
  }
}

//----------------------------------------------------------------------------
bool vtkKWMimxMainWindow::GetAutoSaveFlag( )
{
  return this->AutoSaveFlag;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetAutoSaveFlag(bool saveFlag)
{
  this->AutoSaveFlag = saveFlag;
  this->GetApplication()->SetRegistryValue(1, "AutoSave", "SaveFlag", "%d", this->AutoSaveFlag);
}

//----------------------------------------------------------------------------
int vtkKWMimxMainWindow::GetAutoSaveTime( )
{
  return this->AutoSaveTime;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetAutoSaveTime(int saveTime)
{
  this->AutoSaveTime = saveTime;
  this->GetApplication()->SetRegistryValue(1, "AutoSave", "SaveFrequency", "%d", this->AutoSaveTime);
}

//----------------------------------------------------------------------------
bool vtkKWMimxMainWindow::GetAutoSaveWorkDirFlag( )
{
  return this->AutoSaveWorkDirFlag;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetAutoSaveWorkDirFlag(bool saveFlag)
{
  this->AutoSaveWorkDirFlag = saveFlag;
  this->GetApplication()->SetRegistryValue(1, "AutoSave", "UseWorkingDirectory", "%d", this->AutoSaveWorkDirFlag);
}

//----------------------------------------------------------------------------
const char *vtkKWMimxMainWindow::GetAutoSaveDirectory() const
{
  return this->AutoSaveDirectory;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetAutoSaveDirectory(const char *dirName)
{
  strcpy(this->AutoSaveDirectory, dirName);
  this->GetApplication()->SetRegistryValue(1, "AutoSave", "SaveDirectory", this->AutoSaveDirectory);
}



//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::RemoveFontApplicationSettingsPanel()
{
                   
  // --------------------------------------------------------------
  // IA-FEMesh Interface settings : Font Settings
  if (this->FontSettingsFrame == NULL)
  {
      this->Script ("pack forget %s ",
                  this->FontSettingsFrame->GetWidgetName());
  }

  // --------------------------------------------------------------
  // Interface settings : Font size
  if (this->FontScrollFrame == NULL)
  {  
      this->Script ( "pack forget %s",
                  this->FontScrollFrame->GetWidgetName());  
  }
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::RemoveCustomApplicationSettingsPanel()
{
  //vtkKWApplicationSettingsInterface *applicationMenu = 
  //         this->MainWindow->GetApplicationSettingsInterface( ); 
  //vtkKWWidget*  page = applicationMenu->GetPageWidget( "IA-FEMesh" );
  
  this->MimxSettingsFrame->SetEnabled(0);
  this->AverageElementLengthEntry->SetEnabled(0);
  this->PropertyPrecisionScale->SetEnabled(0);
  
  //page->Unpack();
  /*
  applicationMenu->RemovePage( "IA-FEMesh" );
                     
  this->Script ("pack forget %s ",
                  this->MimxSettingsFrame->GetWidgetName());
                  
  this->MimxSettingsFrame->Delete(); 
  this->MimxSettingsFrame = NULL;
  this->AutoSaveButton->Delete();   
  this->AutoSaveButton = NULL;
  this->AutoSaveScale->Delete(); 
  this->AutoSaveScale = NULL;
  this->WorkingDirButton->Delete(); 
  this->WorkingDirButton = NULL;
  this->AutoSaveDir->Delete(); 
  this->AutoSaveDir = NULL;
  this->AverageElementLengthEntry->Delete(); 
  this->AverageElementLengthEntry = NULL;
  this->PropertyPrecisionScale->Delete(); 
  this->PropertyPrecisionScale = NULL;
  */
  //applicationMenu->Update();
  
}


//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::AddFontApplicationSettingsPanel()
{
  vtkKWApplicationSettingsInterface *applicationMenu = 
           this->MainWindow->GetApplicationSettingsInterface( ); 

  ostrstream tk_cmd;
  vtkKWWidget *page;
  vtkKWFrame *frame;

  // --------------------------------------------------------------
  // Add a "Preferences" page

  //applicationMenu->AddPage( "Preferences" );
  page = applicationMenu->GetPageWidget(applicationMenu->GetName());

                                                              
  // --------------------------------------------------------------
  // IA-FEMesh Interface settings : Font Settings
  if (this->FontSettingsFrame == NULL)
  {
    this->FontSettingsFrame = vtkKWFrameWithLabel::New();
  }
  this->FontSettingsFrame->SetParent(applicationMenu->GetPagesParentWidget());
  this->FontSettingsFrame->Create();
  this->FontSettingsFrame->SetLabelText("Font Settings");

  tk_cmd << "pack " << this->FontSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " << " -in "
         << page->GetWidgetName() << endl;
  frame = this->FontSettingsFrame->GetFrame();


  // --------------------------------------------------------------
  // Interface settings : Font size
  
  vtkKWFrameWithScrollbar *scrollframe = vtkKWFrameWithScrollbar::New();
  this->FontScrollFrame = scrollframe;
  scrollframe->SetParent ( frame );
  scrollframe->Create();
  scrollframe->VerticalScrollbarVisibilityOn();
  scrollframe->HorizontalScrollbarVisibilityOn();
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",
                 scrollframe->GetWidgetName());  

  vtkKWLabel *fontSizeLabel = vtkKWLabel::New();
  fontSizeLabel->SetParent ( scrollframe->GetFrame());
  fontSizeLabel->Create();
  fontSizeLabel->SetText ("Font size:");

  if (this->FontSizeButtons == NULL)
  {
    this->FontSizeButtons = vtkKWRadioButtonSet::New();
  }
  this->FontSizeButtons->SetParent (scrollframe->GetFrame());
  this->FontSizeButtons->Create();
  vtkKWRadioButton *button;
  button = this->FontSizeButtons->AddWidget ( 0 );
  button->SetText  ( "Use small font" );
  button->SetValue ( "small" );
  button->SetCommand ( this, "SetFontSizeCallback");
    
  button = this->FontSizeButtons->AddWidget ( 1 );
  button->SetText ("Use medium font" );
  button->SetValue ( "medium" );
  button->SetCommand ( this, "SetFontSizeCallback");
  button->SetVariableName ( this->FontSizeButtons->GetWidget(0)->GetVariableName());

  button = this->FontSizeButtons->AddWidget ( 2 );
  button->SetText ( "Use large font");
  button->SetValue ( "large" );
  button->SetCommand ( this, "SetFontSizeCallback");
  button->SetVariableName ( this->FontSizeButtons->GetWidget(0)->GetVariableName());

  button = this->FontSizeButtons->AddWidget (3 ); 
  button->SetText ( "Use largest font");
  button->SetValue ( "largest");
  button->SetCommand ( this, "SetFontSizeCallback");
  button->SetVariableName ( this->FontSizeButtons->GetWidget(0)->GetVariableName());

  vtkKWLabel *fontFamilyLabel = vtkKWLabel::New();
  fontFamilyLabel->SetParent ( scrollframe->GetFrame());
  fontFamilyLabel->Create();
  fontFamilyLabel->SetText ("Font family:");

  //--- set selected value from application
  // --------------------------------------------------------------
  // Font family
  if (this->FontFamilyButtons == NULL)
  {
    this->FontFamilyButtons = vtkKWRadioButtonSet::New();
  }  
  this->FontFamilyButtons->SetParent (scrollframe->GetFrame());
  this->FontFamilyButtons->Create();
  
  vtkSlicerTheme *theme = this->GetSlicerTheme();
  int numfonts = theme->GetSlicerFonts()->GetNumberOfFontFamilies();
  const char *font;
  for ( int i = 0; i < numfonts; i++ )
    {
    font = theme->GetSlicerFonts()->GetFontFamily(i);
    button = this->FontFamilyButtons->AddWidget ( i );
    button->SetText  (font );
    button->SetValue ( font );
    button->SetVariableName (this->FontFamilyButtons->GetWidget(0)->GetVariableName() );
    button->SetCommand( this, "SetFontFamilyCallback" );
    }
  
  vtkKWLabel *restartLabel = vtkKWLabel::New();
  restartLabel->SetParent ( scrollframe->GetFrame());
  restartLabel->Create();
  restartLabel->SetText ("(for best results, restart application)");


  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",
                 fontFamilyLabel->GetWidgetName());
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -expand n",
                 this->FontFamilyButtons->GetWidgetName());
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",
                 fontSizeLabel->GetWidgetName());
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",                 
                 this->FontSizeButtons->GetWidgetName());
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",                 
                 restartLabel->GetWidgetName() );

  fontSizeLabel->Delete();
  fontFamilyLabel->Delete();
  restartLabel->Delete();
  scrollframe->Delete();
  
  // --------------------------------------------------------------
  // Pack 

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetFontFamilyCallback (  )
{
  if (this->FontFamilyButtons)
  {
    const char *font= this->FontFamilyButtons->GetWidget(0)->GetVariableValue();
    this->SetApplicationFontFamily ( font );
  }
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetFontSizeCallback ( )
{
  if (this->FontSizeButtons)
  {
    const char *v = this->FontSizeButtons->GetWidget(0)->GetVariableValue();
    this->SetApplicationFontSize(v);
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::AutoSaveModeCallback( int mode )
{ 
  if ( mode )
  {
    this->AutoSaveScale->EnabledOn(); 
    this->WorkingDirButton->EnabledOn(); 
    if ( ! this->WorkingDirButton->GetWidget()->GetSelectedState() )
    {
      this->AutoSaveDir->EnabledOn();
    }
    this->SetAutoSaveFlag( true );
    this->EnableAutoSave(); 
  }
  else
  {
    this->AutoSaveScale->EnabledOff(); 
    this->WorkingDirButton->EnabledOff(); 
    this->AutoSaveDir->EnabledOff(); 
    this->SetAutoSaveFlag( false );
    this->DisableAutoSave(); 
  }
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::AutoSaveScaleCallback( double value )
{
  this->SetAutoSaveTime( static_cast<int>( value ) );
}
//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::AutoSaveDirectoryModeCallback( int mode )
{
  if ( mode )
  {
    this->AutoSaveDir->EnabledOff(); 
    this->SetAutoSaveWorkDirFlag( true );
  }
  else
  {
    this->AutoSaveDir->EnabledOn();
    this->SetAutoSaveWorkDirFlag( false );
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::AutoSaveDirectoryCallback( )
{
  if ( this->AutoSaveDir->GetWidget()->GetFileName() )
    this->SetAutoSaveDirectory( this->AutoSaveDir->GetWidget()->GetFileName() );
}
//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::AverageElementLengthCallback(char *value)
{
  double elementLength = atof( value );
  this->SetAverageElementLength( elementLength );
}
//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::ABAQUSPrecisionCallback( double value )
{
  int precision = static_cast<int>( value );
  this->SetABAQUSPrecision( precision );
}
//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::Update()
{
  //this->Superclass::Update();
  
  if ( this->GetAutoSaveFlag() )
  {
    if (this->AutoSaveButton)
    { 
      this->AutoSaveDirectoryModeCallback( 1 );
      this->AutoSaveButton->GetWidget()->SelectedStateOn();
    }
  }
  else
  {
    if (this->AutoSaveButton) 
    {
      this->AutoSaveDirectoryModeCallback( 0 );
      this->AutoSaveButton->GetWidget()->SelectedStateOff();
    }
  }
  
  if ( this->GetAutoSaveWorkDirFlag() )
  {
    if (this->WorkingDirButton)
    { 
      this->AutoSaveDirectoryModeCallback( 1 );
      this->WorkingDirButton->GetWidget()->SelectedStateOn();
    }
  }
  else
  {
    if (this->WorkingDirButton) 
    {
      this->AutoSaveDirectoryModeCallback( 0 );
      this->WorkingDirButton->GetWidget()->SelectedStateOff();
    }
  }
  
  double saveTime = static_cast<double>(this->GetAutoSaveTime());
  if (this->AutoSaveScale)
  {
    this->AutoSaveScale->GetWidget()->SetValue( saveTime );
  }
  
  const char *autoDir =  this->GetAutoSaveDirectory();
  if (this->AutoSaveDir)
  {
    this->AutoSaveDir->GetWidget()->SetInitialFileName( autoDir );
  }
  
  double elementLength = static_cast<double>(this->GetAverageElementLength());
  if (this->AverageElementLengthEntry)
  {
    if ( elementLength <= 0.0 ) elementLength = 1.0;
    this->AverageElementLengthEntry->GetWidget()->SetValueAsDouble( elementLength );
  }
  
  double precision = static_cast<double>(this->GetABAQUSPrecision());
  if (this->PropertyPrecisionScale)
  {
    this->PropertyPrecisionScale->GetWidget()->SetValue( precision );
  }
  
  
  if ( this->FontSizeButtons )
    {
    if ( !(strcmp(this->GetApplicationFontSize(), "small" )))
      {
      this->FontSizeButtons->GetWidget(0)->SetSelectedState(1);
      }
    else if ( !(strcmp(this->GetApplicationFontSize(), "medium")))
      {
      this->FontSizeButtons->GetWidget(1)->SetSelectedState(1);
      }
    else if ( !(strcmp(this->GetApplicationFontSize(), "large")))
      {
      this->FontSizeButtons->GetWidget(2)->SetSelectedState(1);
      }
    else if ( !(strcmp(this->GetApplicationFontSize(), "largest")))
      {
      this->FontSizeButtons->GetWidget(3)->SetSelectedState(1);
      }      
    }
    
//  if ( this->FontFamilyButtons )
//    {
//    if ( !(strcmp (this->GetApplicationFontFamily(), "Arial" )))
//      {
//      this->FontFamilyButtons->GetWidget(0)->SetSelectedState ( 1 );
//      }
//    if ( !(strcmp (this->GetApplicationFontFamily(), "Helvetica" )))
//      {
//      this->FontFamilyButtons->GetWidget(1)->SetSelectedState ( 1 );
//      }
//    if ( !(strcmp (this->GetApplicationFontFamily(), "Verdana" )))
//      {
//      this->FontFamilyButtons->GetWidget(2)->SetSelectedState ( 1 );
//      }
//    }     
}
//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::LoadRegistryApplicationSettings()
{
  int registryFlag;
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "AutoSave", "SaveFlag");
  if ( registryFlag )
  {
    this->AutoSaveFlag = static_cast<bool> ( 
          this->GetApplication()->GetIntRegistryValue(1, "AutoSave", "SaveFlag") );
  }
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "AutoSave", "SaveFrequency");
  if ( registryFlag )
  {
    this->AutoSaveTime = this->GetApplication()->GetIntRegistryValue(1, "AutoSave", "SaveFrequency");
  }
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "AutoSave", "UseWorkingDirectory");
  if ( registryFlag )
  {
    this->AutoSaveWorkDirFlag = static_cast<bool> ( 
          this->GetApplication()->GetIntRegistryValue(1, "AutoSave", "UseWorkingDirectory") );
  }
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "AutoSave", "SaveDirectory");
  if ( registryFlag )
  {
    this->GetApplication()->GetRegistryValue(1, "AutoSave", "SaveDirectory", this->AutoSaveDirectory);
  }
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "ViewerBackground", "Colors");
  if ( registryFlag )
  {
    this->GetApplication()->RetrieveColorRegistryValue(1, "ViewerBackground", this->BackgroundColor);
  }
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "ViewerText", "Colors");
  if ( registryFlag )
  {
    this->GetApplication()->RetrieveColorRegistryValue(1, "ViewerText", this->TextColor);
  }
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "Font", "Family");
  if ( registryFlag )
  {
    this->GetApplication()->GetRegistryValue(1, "Font", "Family", this->ApplicationFontFamily);
  }
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "Font", "Size");
  if ( registryFlag )
  {
    this->GetApplication()->GetRegistryValue(1, "Font", "Size", this->ApplicationFontSize);
  } 
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "MeshProperties", "ABAQUSPrecision");
  if ( registryFlag )
  {
    this->ABAQUSPrecision = static_cast<int> ( 
          this->GetApplication()->GetIntRegistryValue(1, "MeshProperties", "ABAQUSPrecision") );
  } 
  
  registryFlag = this->GetApplication()->HasRegistryValue (1, "MeshProperties", "AverageElementLength");
  if ( registryFlag )
  {
    this->AverageElementLength = static_cast<double> ( 
          this->GetApplication()->GetFloatRegistryValue(1, "MeshProperties", "AverageElementLength") );
  } 
  
}

//----------------------------------------------------------------------------
const char *vtkKWMimxMainWindow::GetSaveDirectory()
{
  if ( this->AutoSaveWorkDirFlag )
    sprintf(this->SaveDirectory, "%s/%s", this->WorkingDirectory, this->DateTimeString);
  else
    sprintf(this->SaveDirectory, "%s/%s", this->AutoSaveDirectory, this->DateTimeString);
  
  return this->SaveDirectory;
}

//----------------------------------------------------------------------------
void vtkKWMimxMainWindow::InstallDefaultTheme ( )
{
  InstallTheme( this->SlicerTheme );
}

//---------------------------------------------------------------------------
void vtkKWMimxMainWindow::InstallTheme ( vtkKWTheme *theme )
{
  if ( theme != NULL ) 
  {
    if ( vtkSlicerTheme::SafeDownCast (theme) == this->SlicerTheme ) {
        this->GetApplication()->SetTheme (this->SlicerTheme );
    } else {
        this->GetApplication()->SetTheme ( theme );
    }
  }
}

//---------------------------------------------------------------------------
vtkSlicerTheme *vtkKWMimxMainWindow::GetSlicerTheme ( )
{
  return this->SlicerTheme;
}

//---------------------------------------------------------------------------
void vtkKWMimxMainWindow::SetStatusText ( const char *text )
{
  this->MainWindow->SetStatusText( text );
}

//---------------------------------------------------------------------------
vtkKWUserInterfaceManager* vtkKWMimxMainWindow::GetMainUserInterfaceManager ( )
{
  return this->MainWindow->GetMainUserInterfaceManager( );
}

//---------------------------------------------------------------------------
void vtkKWMimxMainWindow::UpdateEnableState ( )
{
  this->MainWindow->UpdateEnableState( );
}
//----------------------------------------------------------------------------------------------

// Force redraw since in slicer the panels weren't redrawing automatically
//---------------------------------------------------------------------------
void vtkKWMimxMainWindow::ForceWidgetRedraw ( )
{
  //this->MainFrame->CollapseFrame( );
  //this->MainFrame->ExpandFrame( );
}
//----------------------------------------------------------------------------------------------


// clear and display the objects from the lists.  This is used when the 
 // module is entered and exited, so the actors don't display in the slicer
 // window when we are not in the module.  This traverses the list and stores
 // state in the MRML nodes. 

 void  vtkKWMimxMainWindow::SaveVisibilityStateOfObjectLists(void)
 {
   // pass the request on down to the appropriate tabs
   if(this->MainUserInterfacePanel != NULL)
         this->MainUserInterfacePanel->SaveVisibilityStateOfObjectLists();
 }
 
 void  vtkKWMimxMainWindow::RestoreVisibilityStateOfObjectLists(void)
 {
   if(this->MainUserInterfacePanel != NULL)
          this->MainUserInterfacePanel->RestoreVisibilityStateOfObjectLists();
 }
 
 
