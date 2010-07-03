/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxApplicationSettingsInterface.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.6.4.1 $

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

#include "vtkKWMimxApplicationSettingsInterface.h"

#include "vtkObjectFactory.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerFont.h"

#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMenu.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWSeparator.h"
#include "vtkKWWidget.h"

#include "vtkKWMimxApplication.h"
#include "vtkKWMimxMainWindow.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxApplicationSettingsInterface );
vtkCxxRevisionMacro(vtkKWMimxApplicationSettingsInterface, "$Revision: 1.6.4.1 $");

//----------------------------------------------------------------------------
vtkKWMimxApplicationSettingsInterface::vtkKWMimxApplicationSettingsInterface()
{
  this->MimxSettingsFrame = NULL;
  this->autoSaveButton = NULL;  
  this->autoSaveScale = NULL;  
  this->workingDirButton = NULL;  
  this->autoSaveDir = NULL;  
  
  this->FontSettingsFrame = NULL;
  this->FontSizeButtons = NULL;
  this->FontFamilyButtons = NULL;
  this->Separator = NULL;
  this->AverageElementLengthEntry = NULL;
  this->PropertyPrecisionScale = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxApplicationSettingsInterface::~vtkKWMimxApplicationSettingsInterface()
{
  if (this->MimxSettingsFrame)
    {
    this->MimxSettingsFrame->Delete();
    this->MimxSettingsFrame = NULL;
    }
  if (this->autoSaveButton)
    {
    this->autoSaveButton->Delete();
    this->autoSaveButton = NULL;
    }
  if (this->autoSaveScale)
    {
    this->autoSaveScale->Delete();
    this->autoSaveScale = NULL;
    }
  if (this->workingDirButton)
    {
    this->workingDirButton->Delete();
    this->workingDirButton = NULL;
    }
  if (this->autoSaveDir)
    {
    this->autoSaveDir->Delete();
    this->autoSaveDir = NULL;
    }
  if ( this->FontSizeButtons )
    {
    this->FontSizeButtons->SetParent ( NULL );
    this->FontSizeButtons->Delete();
    this->FontSizeButtons = NULL;
    }
  if ( this->FontFamilyButtons )
    {
    this->FontFamilyButtons->SetParent ( NULL );
    this->FontFamilyButtons->Delete();
    this->FontFamilyButtons = NULL;
    }
  if ( this->FontSettingsFrame )
    {
    this->FontSettingsFrame->SetParent ( NULL );
    this->FontSettingsFrame->Delete();
    this->FontSettingsFrame = NULL;
    }
  if (this->workingDirButton)
    {
    this->Separator->Delete();
    this->Separator = NULL;
    }
  if (this->workingDirButton)
    {
    this->AverageElementLengthEntry->Delete();
    this->AverageElementLengthEntry = NULL;
    }
  if (this->workingDirButton)
    {
    this->PropertyPrecisionScale->Delete();
    this->PropertyPrecisionScale = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::Create()
{
  if (this->IsCreated())
    {
    vtkErrorMacro("The panel is already created.");
    return;
    }

  // Create the superclass instance (and set the application)

  this->Superclass::Create();

  ostrstream tk_cmd;
  vtkKWWidget *page;
  vtkKWFrame *frame;

  //int label_width = 20;

  // --------------------------------------------------------------
  // Add a "Preferences" page

  this->AddPage(this->GetName());
  page = this->GetPageWidget(this->GetName());

  // --------------------------------------------------------------
  // Slicer Interface settings : main frame
  if (!this->MimxSettingsFrame)
    {
    this->MimxSettingsFrame = vtkKWFrameWithLabel::New();
    }
  this->MimxSettingsFrame->SetParent(this->GetPagesParentWidget());
  this->MimxSettingsFrame->Create();
  this->MimxSettingsFrame->SetLabelText("IA-FEMesh Settings");

  tk_cmd << "pack " << this->MimxSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " 
         << " -in " << page->GetWidgetName() << endl;
  
  frame = this->MimxSettingsFrame->GetFrame();
  
  this->autoSaveButton = vtkKWCheckButtonWithLabel::New();
  this->autoSaveButton->SetParent ( frame );
  this->autoSaveButton->Create();
  this->autoSaveButton->SetLabelText ("Autosave Work:");
  this->autoSaveButton->GetWidget()->SetCommand ( this, "AutoSaveModeCallback");
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2",
                 this->autoSaveButton->GetWidgetName());
  
  this->autoSaveScale = vtkKWScaleWithLabel::New();
  this->autoSaveScale->SetParent ( frame );
  this->autoSaveScale->Create();
  this->autoSaveScale->SetLabelText ("Interval (Minutes):");
  this->autoSaveScale->GetWidget()->SetRange ( 1.0, 20.0);
  this->autoSaveScale->GetWidget()->SetResolution ( 1.0 );
  this->autoSaveScale->GetWidget()->SetCommand ( this, "AutoSaveScaleCallback");
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                 this->autoSaveScale->GetWidgetName());  
  
  this->workingDirButton = vtkKWCheckButtonWithLabel::New();
  this->workingDirButton->SetParent ( frame );
  this->workingDirButton->Create();
  this->workingDirButton->SetLabelText ("Use Working Directory:");
  this->workingDirButton->GetWidget()->SetCommand ( this, "AutoSaveDirectoryModeCallback");
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2",
                 this->workingDirButton->GetWidgetName());
                 
  this->autoSaveDir = vtkKWLoadSaveButtonWithLabel::New();
  this->autoSaveDir->SetParent ( frame );
  this->autoSaveDir->Create();
  this->autoSaveDir->SetLabelText ("Autosave Directory:");
  this->autoSaveDir->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();
  this->autoSaveDir->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->autoSaveDir->GetWidget()->TrimPathFromFileNameOff();
  this->autoSaveDir->GetWidget()->SetCommand ( this, "AutoSaveDirectoryCallback");
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                 this->autoSaveDir->GetWidgetName()); 
  
  this->Separator = vtkKWSeparator::New();
  this->Separator->SetParent ( frame );
  this->Separator->Create();
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                 this->Separator->GetWidgetName()); 
  
  this->AverageElementLengthEntry = vtkKWEntryWithLabel::New();
  this->AverageElementLengthEntry->SetParent ( frame );
  this->AverageElementLengthEntry->Create();
  this->AverageElementLengthEntry->SetLabelText("Average Element Length:");
  this->AverageElementLengthEntry->GetWidget()->SetRestrictValueToDouble( );
  this->AverageElementLengthEntry->GetWidget()->SetCommand ( this, "AverageElementLengthCallback");
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                 this->AverageElementLengthEntry->GetWidgetName()); 
  
  this->PropertyPrecisionScale = vtkKWScaleWithLabel::New();
  this->PropertyPrecisionScale->SetParent ( frame );
  this->PropertyPrecisionScale->Create();
  this->PropertyPrecisionScale->SetLabelText("ABAQUS Material Property Precision:");
  this->PropertyPrecisionScale->GetWidget()->SetRange(0.0, 10.0);
  this->PropertyPrecisionScale->GetWidget()->SetResolution(1.0);
  this->PropertyPrecisionScale->GetWidget()->SetCommand( this, "ABAQUSPrecisionCallback");
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x",
                 this->PropertyPrecisionScale->GetWidgetName()); 
                                                              
  // --------------------------------------------------------------
  // Slicer interface settings : Font settings frame
  if ( !this->FontSettingsFrame )
    {
    this->FontSettingsFrame = vtkKWFrameWithLabel::New();
    }
  this->FontSettingsFrame->SetParent(this->GetPagesParentWidget());
  this->FontSettingsFrame->Create();
  this->FontSettingsFrame->SetLabelText("Font Settings");

  tk_cmd << "pack " << this->FontSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " << " -in "
         << page->GetWidgetName() << endl;
  frame = this->FontSettingsFrame->GetFrame();


  // --------------------------------------------------------------
  // Slicer interface settings : Font size?
  
  vtkKWFrameWithScrollbar *scrollframe = vtkKWFrameWithScrollbar::New();
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

  if ( !this->FontSizeButtons)
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
  // Slicer interface settings : Font family?
  
  if ( !this->FontFamilyButtons)
    {
    this->FontFamilyButtons = vtkKWRadioButtonSet::New();
    }
  this->FontFamilyButtons->SetParent (scrollframe->GetFrame());
  this->FontFamilyButtons->Create();
  vtkKWMimxApplication *app = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  vtkSlicerTheme *theme = app->GetSlicerTheme();
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
  restartLabel->SetText ("(for best results, restart IA-FEMesh)");


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

  // Update

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::SetWorkingDirectory( char *directoryName )
{
  vtkKWMimxApplication *app
    = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  
  app->SetAutoSaveDirectory( directoryName );
}


//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::SetFontFamilyCallback (  )
{
  vtkKWMimxApplication *app = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {
    const char *font= this->FontFamilyButtons->GetWidget(0)->GetVariableValue();
    app->SetApplicationFontFamily ( font ); 
    }
}

//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::SetFontSizeCallback ( )
{
  vtkKWMimxApplication *app = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {
    const char *v = this->FontSizeButtons->GetWidget(0)->GetVariableValue();
    app->SetApplicationFontSize(v);
    }
}

//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::Update()
{
  this->Superclass::Update();
  
  vtkKWMimxApplication *app
    = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  
  if ( app != NULL )
  {
    if ( app->GetAutoSaveFlag() )
    {
      if (this->autoSaveButton)
      { 
        this->AutoSaveDirectoryModeCallback( 1 );
        this->autoSaveButton->GetWidget()->SelectedStateOn();
      }
    }
    else
    {
      if (this->autoSaveButton) 
      {
        this->AutoSaveDirectoryModeCallback( 0 );
        this->autoSaveButton->GetWidget()->SelectedStateOff();
      }
    }
    
    if ( app->GetAutoSaveWorkDirFlag() )
    {
      if (this->workingDirButton)
      { 
        this->AutoSaveDirectoryModeCallback( 1 );
        this->workingDirButton->GetWidget()->SelectedStateOn();
      }
    }
    else
    {
      if (this->workingDirButton) 
      {
        this->AutoSaveDirectoryModeCallback( 0 );
        this->workingDirButton->GetWidget()->SelectedStateOff();
      }
    }
    
    double saveTime = static_cast<double>(app->GetAutoSaveTime());
    if (this->autoSaveScale)
    {
      this->autoSaveScale->GetWidget()->SetValue( saveTime );
    }
    
    const char *autoDir =  app->GetAutoSaveDirectory();
    if (this->autoSaveDir)
    {
      this->autoSaveDir->GetWidget()->SetInitialFileName( autoDir );
    }
    
    
    if ( this->FontSizeButtons )
      {
      if ( !(strcmp(app->GetApplicationFontSize(), "small" )))
        {
        this->FontSizeButtons->GetWidget(0)->SetSelectedState(1);
        }
      else if ( !(strcmp(app->GetApplicationFontSize(), "medium")))
        {
        this->FontSizeButtons->GetWidget(1)->SetSelectedState(1);
        }
      else if ( !(strcmp(app->GetApplicationFontSize(), "large")))
        {
        this->FontSizeButtons->GetWidget(2)->SetSelectedState(1);
        }
      else if ( !(strcmp(app->GetApplicationFontSize(), "largest")))
        {
        this->FontSizeButtons->GetWidget(3)->SetSelectedState(1);
        }      
      }
      
    if ( this->FontFamilyButtons )
      {
      if ( !(strcmp (app->GetApplicationFontFamily(), "Arial" )))
        {
        this->FontFamilyButtons->GetWidget(0)->SetSelectedState ( 1 );
        }
      if ( !(strcmp (app->GetApplicationFontFamily(), "Helvetica" )))
        {
        this->FontFamilyButtons->GetWidget(1)->SetSelectedState ( 1 );
        }
      if ( !(strcmp (app->GetApplicationFontFamily(), "Verdana" )))
        {
        this->FontFamilyButtons->GetWidget(2)->SetSelectedState ( 1 );
        }
      }
      
    double elementLength = static_cast<double>(app->GetAverageElementLength());
    if (this->AverageElementLengthEntry)
    {
      this->AverageElementLengthEntry->GetWidget()->SetValueAsDouble( elementLength );
    }
    
    double precision = static_cast<double>(app->GetABAQUSPrecision());
    if (this->PropertyPrecisionScale)
    {
      this->PropertyPrecisionScale->GetWidget()->SetValue( precision );
    }
    
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::AutoSaveModeCallback( int mode )
{
  vtkKWMimxApplication *app
    = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
    
  if ( mode )
  {
    this->autoSaveScale->EnabledOn(); 
    this->workingDirButton->EnabledOn(); 
    if ( ! this->workingDirButton->GetWidget()->GetSelectedState() )
    {
      this->autoSaveDir->EnabledOn();
    }
    app->SetAutoSaveFlag( true );
    vtkKWMimxMainWindow *window = 
      vtkKWMimxMainWindow::SafeDownCast ( this->GetWindow( ) );
    window->EnableAutoSave(); 
  }
  else
  {
    this->autoSaveScale->EnabledOff(); 
    this->workingDirButton->EnabledOff(); 
    this->autoSaveDir->EnabledOff(); 
    app->SetAutoSaveFlag( false );
    vtkKWMimxMainWindow *window = 
      vtkKWMimxMainWindow::SafeDownCast ( this->GetWindow( ) );
    window->DisableAutoSave(); 
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::AutoSaveScaleCallback( double value )
{
  vtkKWMimxApplication *app
    = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  
  app->SetAutoSaveTime( static_cast<int>( value ) );
}
//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::AutoSaveDirectoryModeCallback( int mode )
{
  vtkKWMimxApplication *app
    = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
    
  if ( mode )
  {
    this->autoSaveDir->EnabledOff(); 
    app->SetAutoSaveWorkDirFlag( true );
  }
  else
  {
    this->autoSaveDir->EnabledOn();
    app->SetAutoSaveWorkDirFlag( false );
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::AutoSaveDirectoryCallback( )
{
  vtkKWMimxApplication *app
    = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  
  if ( this->autoSaveDir->GetWidget()->GetFileName() )
    app->SetAutoSaveDirectory( this->autoSaveDir->GetWidget()->GetFileName() );
}
//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::AverageElementLengthCallback(char *value)
{
  vtkKWMimxApplication *app
    = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  
  double elementLength = atof( value );
  app->SetAverageElementLength( elementLength );
}
//----------------------------------------------------------------------------
void vtkKWMimxApplicationSettingsInterface::ABAQUSPrecisionCallback( double value )
{
  vtkKWMimxApplication *app
    = vtkKWMimxApplication::SafeDownCast(this->GetApplication());
  
  int precision = static_cast<int>( value );
  app->SetABAQUSPrecision( precision );
}
//----------------------------------------------------------------------------
