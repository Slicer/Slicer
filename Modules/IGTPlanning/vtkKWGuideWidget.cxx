#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkObjectFactory.h"
#include "vtkKWGuideWidget.h"
#include "vtkKWInternationalization.h"

#include <vtksys/stl/string>

//-------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWGuideWidget );
vtkCxxRevisionMacro(vtkKWGuideWidget, "$Revision: 1.47 $");

//----------------------------------------------------------------------------
vtkKWGuideWidget::vtkKWGuideWidget()
{
  this->ButtonFrame = vtkKWFrame::New();

  this->EditorButton = vtkKWPushButton::New();
  this->HomeButton = vtkKWPushButton::New();


}


//----------------------------------------------------------------------------
vtkKWGuideWidget::~vtkKWGuideWidget()
{
  this->ButtonFrame->Delete();
  this->ButtonFrame = NULL;
 
  this->EditorButton->Delete();
  this->EditorButton = NULL;

  this->HomeButton->Delete();
  this->HomeButton = NULL;
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
  
  this->EditorButton->SetParent(this->ButtonFrame);
  this->EditorButton->Create();
  this->EditorButton->SetCommand(this, "EditorButtonCallback");
  this->EditorButton->SetText(ks_("IGT Module Guide|Button|Editor"));

  this->HomeButton->SetParent(this->ButtonFrame);
  this->HomeButton->Create();
  this->HomeButton->SetCommand(this, "HomeButtonCallback");
  this->HomeButton->SetText(ks_("IGT Module Guide|Button|Home"));
 
  this->Script("pack %s %s -side left -expand 1 -fill x",
               this->EditorButton->GetWidgetName(),
               this->HomeButton->GetWidgetName()
               );
  
  this->Withdraw();
}

//----------------------------------------------------------------------------
void vtkKWGuideWidget::EditorButtonCallback()
{
cout << "Editor button." << endl;
}

//----------------------------------------------------------------------------
void vtkKWGuideWidget::HomeButtonCallback()
{
cout << "Home button." << endl;
}


//----------------------------------------------------------------------------
void vtkKWGuideWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->ButtonFrame);
  this->PropagateEnableState(this->EditorButton);
  this->PropagateEnableState(this->HomeButton);

}


//----------------------------------------------------------------------------
void vtkKWGuideWidget::Display(int x, int y)
{
  if (!this->IsCreated())
    {
    return;
    }

  // Position the toplevel.
  this->SetPosition(x, y);
  this->DeIconify();
  this->Raise();
  this->Focus();

  this->InvokeEvent(vtkKWTopLevel::DisplayEvent);
}
  

//----------------------------------------------------------------------------
void vtkKWGuideWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

