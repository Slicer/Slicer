#include "vtkKWMyWizardWidget.h"

#include <vtkObjectFactory.h>

#include <vtkKWWizardWorkflow.h>
#include "vtkKWMyWizardWorkflow.h"
#include <vtkKWPushButton.h>
#include <vtkCallbackCommand.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyWizardWidget );
vtkCxxRevisionMacro(vtkKWMyWizardWidget, "$Revision: 1.3 $");

vtkKWMyWizardWidget::vtkKWMyWizardWidget()
{
    this->TitleFrame              = NULL;
    this->TitleLabel              = NULL;
    this->SubTitleLabel           = NULL;
    this->TitleIconLabel          = NULL;

    this->SeparatorAfterTitleArea = NULL;

    this->LayoutFrame             = NULL;
    this->PreTextLabel            = NULL;
    this->ClientArea              = NULL;
    this->PostTextLabel           = NULL;
    this->ErrorTextLabel          = NULL;

    this->SeparatorBeforeButtons  = NULL;

    this->ButtonFrame             = NULL;
    this->BackButton              = NULL;
    this->NextButton              = NULL;
    this->FinishButton            = NULL;
    this->CancelButton            = NULL;
    this->HelpButton              = NULL;
    this->OKButton                = NULL;

    this->WizardWorkflow          = vtkKWMyWizardWorkflow::New();

    this->BackButtonVisibility    = 1;
    this->NextButtonVisibility    = 1;
    this->FinishButtonVisibility  = 1;
    this->CancelButtonVisibility  = 1;
    this->HelpButtonVisibility    = 0;
    this->OKButtonVisibility      = 1;

    this->ButtonsPosition   = vtkKWWizardWidget::ButtonsPositionBottom;
}

vtkKWMyWizardWidget::~vtkKWMyWizardWidget()
{
}

vtkKWWizardWorkflow *vtkKWMyWizardWidget::GetWizardWorkflow()
{
    return this->WizardWorkflow;
}

vtkKWMyWizardWorkflow *vtkKWMyWizardWidget::GetMyWizardWorkflow()
{
    return this->WizardWorkflow;
}

//----------------------------------------------------------------------------
void vtkKWMyWizardWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }
  
  if(!this->WizardWorkflow)
  {
      this->WizardWorkflow = vtkKWMyWizardWorkflow::New();
  }
  if(!this->WizardWorkflow->GetApplication())
  {
      this->WizardWorkflow->SetApplication(this->GetApplication());
  }

  // Call the superclass to create the whole widget
  this->Superclass::WizardWorkflow = this->WizardWorkflow;
  this->Superclass::CreateWidget();
  
  vtkCallbackCommand *nextBtnClicked = vtkCallbackCommand::New();
  nextBtnClicked->SetClientData(this);
  nextBtnClicked->SetCallback(&vtkKWMyWizardWidget::NextButtonClicked);
  this->NextButton->AddObserver(vtkKWPushButton::InvokedEvent, nextBtnClicked);
  
  vtkCallbackCommand *backBtnClicked = vtkCallbackCommand::New();
  backBtnClicked->SetClientData(this);
  backBtnClicked->SetCallback(&vtkKWMyWizardWidget::BackButtonClicked);
  this->BackButton->AddObserver(vtkKWPushButton::InvokedEvent, backBtnClicked);
}

void vtkKWMyWizardWidget::NextButtonClicked(vtkObject* obj, unsigned long,void* callbackData, void*)
{
    std::cout<<"nextButtonClicked"<<std::endl;
    vtkKWMyWizardWidget *myWizWidg = (vtkKWMyWizardWidget*)callbackData;
    if(myWizWidg)
    {
        myWizWidg->InvokeEvent(vtkKWMyWizardWidget::nextButtonClicked);
    }
}

void vtkKWMyWizardWidget::BackButtonClicked(vtkObject* obj, unsigned long,void* callbackData, void*)
{
    std::cout<<"backButtonClicked"<<std::endl;
    vtkKWMyWizardWidget *myWizWidg = (vtkKWMyWizardWidget*)callbackData;
    if(myWizWidg)
    {
        myWizWidg->InvokeEvent(vtkKWMyWizardWidget::backButtonClicked);
    }    
}

