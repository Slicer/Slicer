#include "vtkObjectFactory.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkKWUserInterfacePanel.h"


//---------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerModuleGUI, "$Revision: 1.0 $");
vtkStandardNewMacro ( vtkSlicerModuleGUI );

vtkSlicerBaseAcknowledgementLogoIcons* vtkSlicerModuleGUI::AcknowledgementIcons = NULL;

//---------------------------------------------------------------------------
vtkSlicerModuleGUI::vtkSlicerModuleGUI ( ) {

    this->UIPanel = vtkKWUserInterfacePanel::New ( );
    this->HelpAndAboutNotebook = vtkKWNotebook::New ();
    this->HelpAndAboutFrame = vtkSlicerModuleCollapsibleFrame::New ();
    this->HelpText = vtkKWTextWithHyperlinksWithScrollbars::New ( );
    this->AboutText = vtkKWTextWithHyperlinksWithScrollbars::New ( );
    this->LogoFrame = vtkKWFrame::New ( );

    this->ApplicationGUI = NULL;
    this->Logo = 0;
    this->Built = false;
    this->ModuleName = NULL;
}

//---------------------------------------------------------------------------
vtkSlicerModuleGUI::~vtkSlicerModuleGUI ( ) {

  if  ( vtkSlicerModuleGUI::AcknowledgementIcons != NULL )
    {
    vtkSlicerModuleGUI::AcknowledgementIcons->Delete();
    vtkSlicerModuleGUI::AcknowledgementIcons = NULL;
    }
  
    if ( this->UIPanel != NULL ) 
      {
      this->UIPanel->Delete ( );
      this->UIPanel = NULL;
      }
    if ( this->HelpText != NULL )
      {
      this->HelpText->SetParent ( NULL );
      this->HelpText->Delete ( );
      this->HelpText = NULL;
      }
    if ( this->AboutText != NULL )
      {
      this->AboutText->SetParent ( NULL );
      this->AboutText->Delete();
      this->AboutText = NULL;
      }
    if ( this->LogoFrame != NULL )
      {
      this->LogoFrame->SetParent ( NULL);
      this->LogoFrame->Delete ( );
      this->LogoFrame = NULL;
      }
    if ( this->HelpAndAboutNotebook != NULL )
      {
      this->HelpAndAboutNotebook->SetParent ( NULL );
      this->HelpAndAboutNotebook->Delete();
      this->HelpAndAboutNotebook = NULL;
      }
    if ( this->HelpAndAboutFrame != NULL )
      {
      this->HelpAndAboutFrame->SetParent ( NULL );
      this->HelpAndAboutFrame->Delete ( );
      this->HelpAndAboutFrame = NULL;
      }
    this->SetApplicationGUI( NULL );
    this->Logo = 0;
    this->Built = false;
    this->SetModuleName (NULL);
}


vtkSlicerBaseAcknowledgementLogoIcons* vtkSlicerModuleGUI::GetAcknowledgementIcons () 
{
  if  ( vtkSlicerModuleGUI::AcknowledgementIcons == NULL )
    {
    vtkSlicerModuleGUI::AcknowledgementIcons = vtkSlicerBaseAcknowledgementLogoIcons::New(); 
    }
  return vtkSlicerModuleGUI::AcknowledgementIcons;
}

//---------------------------------------------------------------------------
void vtkSlicerModuleGUI::BuildHelpAndAboutFrame ( vtkKWWidget *parent,
                                                  const char *help,
                                                  const char *about)
{

  const char *helptext = help;
  const char *abouttext = about;
  
  // HELP FRAME
  this->HelpAndAboutFrame->SetParent ( parent );
  this->HelpAndAboutFrame->Create ( );
  this->HelpAndAboutFrame->CollapseFrame ( );
  this->HelpAndAboutFrame->SetLabelText ("Help & Acknowledgement");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->HelpAndAboutFrame->GetWidgetName(), parent->GetWidgetName());


  // create the notebook to display help and about.
  this->HelpAndAboutNotebook->SetParent ( this->HelpAndAboutFrame->GetFrame() );
  this->HelpAndAboutNotebook->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->HelpAndAboutNotebook->GetWidgetName());

  // create a tab for help and also for acknowledgement, if they exist.
  if ( about != NULL )
    {
    this->HelpAndAboutNotebook->AddPage ( "Help", "Information about using this module", NULL );
    this->HelpAndAboutNotebook->AddPage ( "Acknowledgement", "Information about sponsors and contributors", NULL );
    }
  else if ( about == NULL )
    {
    this->HelpAndAboutNotebook->AddPage ( "Help", "Information about using this module", NULL );
    }

  
  // configure the parent class's help text widget
  this->HelpText->SetParent ( this->HelpAndAboutNotebook->GetFrame("Help") );
  this->HelpText->Create ( );
  this->HelpText->SetHorizontalScrollbarVisibility ( 0 );
  this->HelpText->SetVerticalScrollbarVisibility ( 1 );
  this->HelpText->GetWidget()->SetReliefToFlat ( );
  this->HelpText->GetWidget()->SetWrapToWord ( );
  this->HelpText->GetWidget()->QuickFormattingOn ( );
  this->HelpText->SetText ( helptext );
  //Important that Read only after SetText otherwise it doesn't work
  this->HelpText->GetWidget()->ReadOnlyOn ( );
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
                this->HelpText->GetWidgetName ( ) );
  
  // configure the parent class's acknowledgement text widget
  this->AboutText->SetParent ( this->HelpAndAboutNotebook->GetFrame("Acknowledgement") );
  this->AboutText->Create ( );
  this->AboutText->SetHorizontalScrollbarVisibility ( 0 );
  this->AboutText->SetVerticalScrollbarVisibility ( 1 );
  this->AboutText->GetWidget()->SetReliefToFlat ( );
  this->AboutText->GetWidget()->SetWrapToWord ( );
  this->AboutText->GetWidget()->QuickFormattingOn ( );
  this->AboutText->SetText ( abouttext );
  //Important that Read only after SetText otherwise it doesn't work
  this->AboutText->GetWidget()->ReadOnlyOn ( );

  // configure the parent class's logo frame
  this->LogoFrame->SetParent ( this->HelpAndAboutNotebook->GetFrame("Acknowledgement") );
  this->LogoFrame->Create();

  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
                this->LogoFrame->GetWidgetName ( ) );
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
                this->AboutText->GetWidgetName ( ) );
}



//---------------------------------------------------------------------------
void vtkSlicerModuleGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerModuleGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "UIPanel: " << this->GetUIPanel ( ) << "\n";
    os << indent << "HelpAndAboutFrame: " << this->GetHelpAndAboutFrame ( ) << "\n";
    os << indent << "HelpAndAboutNotebook: " << this->GetHelpAndAboutNotebook ( ) << "\n";
    os << indent << "HelpText: " << this->GetHelpText ( ) << "\n";
    os << indent << "AboutText: " << this->GetAboutText ( ) << "\n";
    os << indent << "LogoFrame: " << this->GetLogoFrame ( ) << "\n";
    os << indent << "ApplicationGUI: " << this->GetApplicationGUI() << "\n";
    os << indent << "Logo: " << this->GetLogo() << "\n";
}

//---------------------------------------------------------------------------
void vtkSlicerModuleGUI::SetModuleLogic( vtkSlicerLogic * ) 
{
  vtkErrorMacro("vtkSlicerModuleGUI::SetModuleLogic was called! The " << (this->GetGUIName() ? this->GetGUIName() : "unknown") << " Module GUI object was probably not constructed properly otherwise its virtual SetModuleLogic function would have been called instead...");
}

//---------------------------------------------------------------------------
vtkKWIcon *
vtkSlicerModuleGUI::GetLogo() const
{
  return this->Logo.GetPointer();
}



