#include "vtkObjectFactory.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkKWUserInterfacePanel.h"


//---------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerModuleGUI, "$Revision: 1.0 $");
vtkStandardNewMacro ( vtkSlicerModuleGUI );

//---------------------------------------------------------------------------
vtkSlicerModuleGUI::vtkSlicerModuleGUI ( ) {

    this->UIPanel = vtkKWUserInterfacePanel::New ( );
    this->HelpText = vtkKWTextWithScrollbars::New ( );
    this->HelpFrame = vtkSlicerModuleCollapsibleFrame::New ();
    this->ApplicationGUI = NULL;

    this->Logo = 0;
}




//---------------------------------------------------------------------------
vtkSlicerModuleGUI::~vtkSlicerModuleGUI ( ) {

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
    if ( this->HelpFrame != NULL )
      {
      this->HelpFrame->SetParent ( NULL );
      this->HelpFrame->Delete ( );
      this->HelpFrame = NULL;
      }

    this->SetApplicationGUI( NULL );
}



//---------------------------------------------------------------------------
void vtkSlicerModuleGUI::BuildHelpAndAboutFrame ( vtkKWWidget *parent,
                                                  const char *help,
                                                  const char *about)
{

  const int help_and_about_str_len = 2048;
  char txt[help_and_about_str_len];
  char *text = txt;
  
    // HELP FRAME
    this->HelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->HelpFrame->SetParent ( parent );
    this->HelpFrame->Create ( );
    this->HelpFrame->CollapseFrame ( );
    this->HelpFrame->SetLabelText ("Help & About");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->HelpFrame->GetWidgetName(), parent->GetWidgetName());

    if ( about != NULL )
      {
      sprintf ( text, "**Help:** %s\n\n**Acknowledgement:** %s", help, about );
      }
    else if ( about == NULL )
      {
      sprintf ( text, "**Help: %s", help);
      }
    
    // configure the parent classes help text widget
    this->HelpText->SetParent ( this->HelpFrame->GetFrame() );
    this->HelpText->Create ( );
    this->HelpText->SetHorizontalScrollbarVisibility ( 0 );
    this->HelpText->SetVerticalScrollbarVisibility ( 1 );
    this->HelpText->GetWidget()->SetText ( text );
    this->HelpText->GetWidget()->SetReliefToFlat ( );
    this->HelpText->GetWidget()->SetWrapToWord ( );
    this->HelpText->GetWidget()->ReadOnlyOn ( );
    this->HelpText->GetWidget()->QuickFormattingOn ( );
    this->HelpText->GetWidget()->SetBalloonHelpString ( "" );
    this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
                  this->HelpText->GetWidgetName ( ) );
  
}



//---------------------------------------------------------------------------
void vtkSlicerModuleGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerModuleGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "UIPanel: " << this->GetUIPanel ( ) << "\n";
    os << indent << "HelpText: " << this->GetHelpText ( ) << "\n";
    os << indent << "HelpFrame: " << this->GetHelpFrame ( ) << "\n";
    os << indent << "ApplicationGUI: " << this->GetApplicationGUI() << "\n";
    os << indent << "Logo: " << this->GetLogo() << "\n";
}

vtkKWIcon *
vtkSlicerModuleGUI::GetLogo() const
{
  return this->Logo.GetPointer();
}



