#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkObjectFactory.h"
#include "vtkKWGuideWidget.h"
#include "vtkKWInternationalization.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerApplication.h"

#include <vtksys/stl/string>

//-------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWGuideWidget );
vtkCxxRevisionMacro(vtkKWGuideWidget, "$Revision: 1.47 $");

//----------------------------------------------------------------------------
vtkKWGuideWidget::vtkKWGuideWidget()
{
  this->ButtonFrame = vtkKWFrame::New();

  this->FiducialButton = vtkKWPushButton::New();
  this->EditorButton = vtkKWPushButton::New();
  this->ExitButton = vtkKWPushButton::New();

  this->SlicerAppGUI = NULL;
}


//----------------------------------------------------------------------------
vtkKWGuideWidget::~vtkKWGuideWidget()
{
  this->ButtonFrame->Delete();
  this->ButtonFrame = NULL;

  this->FiducialButton->Delete();
  this->FiducialButton = NULL;

  this->EditorButton->Delete();
  this->EditorButton = NULL;

  this->ExitButton->Delete();
  this->ExitButton = NULL;
}

//----------------------------------------------------------------------------
void vtkKWGuideWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->ButtonFrame->SetParent(this);
  this->ButtonFrame->Create();
  this->Script("pack %s -side bottom -fill both -expand 1 -pady 1",
               this->ButtonFrame->GetWidgetName());

  this->FiducialButton->SetParent(this->ButtonFrame);
  this->FiducialButton->Create();
  this->FiducialButton->SetCommand(this, "FiducialButtonCallback");
  this->FiducialButton->SetText(ks_("IGT Module Guide|Button|Fiducial"));
 
  this->EditorButton->SetParent(this->ButtonFrame);
  this->EditorButton->Create();
  this->EditorButton->SetCommand(this, "EditorButtonCallback");
  this->EditorButton->SetText(ks_("IGT Module Guide|Button|Editor"));

  this->ExitButton->SetParent(this->ButtonFrame);
  this->ExitButton->Create();
  this->ExitButton->SetCommand(this, "ExitButtonCallback");
  this->ExitButton->SetText(ks_("IGT Module Guide|Button|Exit"));
 
  this->Script("pack %s %s %s -side left -expand 1 -fill x",
               this->FiducialButton->GetWidgetName(),
               this->EditorButton->GetWidgetName(),
               this->ExitButton->GetWidgetName()
               );
  
  this->Withdraw();
}



//----------------------------------------------------------------------------
void vtkKWGuideWidget::FiducialButtonCallback()
{
  this->RaiseModule(std::string("Fiducials").c_str());
}



//----------------------------------------------------------------------------
void vtkKWGuideWidget::EditorButtonCallback()
{
  this->RaiseModule(std::string("Editor").c_str());
}



//----------------------------------------------------------------------------
void vtkKWGuideWidget::ExitButtonCallback()
{
  this->RaiseModule(std::string("Liver Ablation").c_str());
  this->Withdraw();
}



void vtkKWGuideWidget::RaiseModule(const char *moduleName)
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->SlicerAppGUI->GetApplication());
  vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName(moduleName);        
  if ( currentModule )
    {
    currentModule->Enter( );
    currentModule->GetUIPanel()->Raise();
    this->SlicerAppGUI->GetMainSlicerWindow()->SetStatusText (moduleName);
    }
}



//----------------------------------------------------------------------------
void vtkKWGuideWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->ButtonFrame);
  this->PropagateEnableState(this->EditorButton);
  this->PropagateEnableState(this->ExitButton);

}


//----------------------------------------------------------------------------
void vtkKWGuideWidget::Display(int x, int y)
{
  this->Superclass::Display();

/*
  if (!this->IsCreated())
    {
    return;
    }

  // Position the toplevel.
  this->SetPosition(x, y);
  this->DeIconify();
  this->Raise();
  this->Focus();
*/

  this->InvokeEvent(vtkKWTopLevel::DisplayEvent);
}



//----------------------------------------------------------------------------
void vtkKWGuideWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

