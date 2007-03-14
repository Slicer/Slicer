#include "vtkKWMyWizardWidget.h"

#include <vtkObjectFactory.h>

#include <vtkKWWizardWorkflow.h>
#include "vtkKWMyWizardWorkflow.h"
#include <vtkKWPushButton.h>
#include <vtkCallbackCommand.h>

#include <vtkKWComboBoxWithLabel.h>
#include <vtkKWProgressGauge.h>

#include <vtkKWApplication.h>
#include <vtkKWSeparator.h>
#include <vtkKWComboBox.h>
#include <vtkKWLabel.h>
#include <vtkKWWizardStep.h>
#include <vtkKWEntry.h>

#include <vtkSlicerColor.h>

#include <sstream>

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
    
    this->ProgressGauge = NULL;
    this->ComboBox = NULL;
    
    this->m_itemToStepMap = NULL;    
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
    
  vtkSlicerColor *curColor = vtkSlicerColor::New();
  
  // Add some workflow related items into the wizard gui
  if(!this->ComboBox)
  {
      this->ComboBox = vtkKWComboBoxWithLabel::New();
  }

  this->ComboBox->SetParent(this->TitleFrame);
//  this->ComboBox->SetBackgroundColor(this->GetTitleAreaBackgroundColor());
//  this->ComboBox->GetWidget()->SetBackgroundColor(this->GetTitleAreaBackgroundColor());
  this->ComboBox->Create();
  
//  this->m_historyCBWL->SetBackgroundColor(this->GetTitleAreaBackgroundColor());
//  this->ComboBox->SetBackgroundColor(1.0,0,0);
//  this->ComboBox->GetWidget()->SetBackgroundColor(1.0,0,0);
//  this->ComboBox->GetLabel()->SetBackgroundColor(1.0,0,0);
//  this->m_historyCBWL->GetWidget()->SetBackgroundColor(this->GetTitleAreaBackgroundColor());
  
  this->ComboBox->SetLabelText("History:");
  this->ComboBox->GetWidget()->ReadOnlyOn();
  this->ComboBox->GetWidget()->SetCommandTriggerToAnyChange();
  
  this->ComboBox->GetLabel()->SetBackgroundColor(curColor->LightestGreyBlue);
  this->ComboBox->GetWidget()->SetBackgroundColor(curColor->LightestGreyBlue);
  this->ComboBox->SetBackgroundColor(curColor->LightestGreyBlue);
  
  this->Script("grid %s -row 0 -column 2 -sticky nswe -padx 8",
          this->ComboBox->GetWidgetName());

  this->Script("grid columnconfigure %s 2 -weight 0",
          this->TitleFrame->GetWidgetName());
  
  this->ComboBox->GetWidget()->SetCommand(this, "ComboBoxEntryChanged");
  
//  this->GetApplication()->Script("pack %s -side right -anchor ne -expand y -fill both -padx 2 -pady 2", 
//          cbStepHistory->GetWidgetName());
  
  // Add some workflow related items into the wizard gui
  if(!this->ProgressGauge)
  {
      this->ProgressGauge = vtkKWProgressGauge::New();
  }
  
  this->ProgressGauge->SetParent(this->TitleFrame);
  this->ProgressGauge->Create();  
  this->ProgressGauge->SetWidth(175);
  this->ProgressGauge->SetBackgroundColor(curColor->LightestGreyBlue);
  
  this->Script("grid %s -row 1 -column 2 -sticky nwe -padx 8 -pady 2",
          this->ProgressGauge->GetWidgetName());

  this->Script("grid columnconfigure %s 2 -weight 0",
          this->TitleFrame->GetWidgetName());
//  
//  if (this->ButtonsPosition == vtkKWWizardWidget::ButtonsPositionBottom) {
//        this->Script("pack %s -side left -fill x -padx 0 -pady 0",
//                pgWorkflow->GetWidgetName());
//    } else {
//        
//        this->Script("pack %s -side left -fill x -padx 0 -pady 0 -before %s",
//                pgWorkflow->GetWidgetName(),
//                this->SeparatorBeforeButtons->GetWidgetName());
//    }

////  cbStepHistory->SetLabelText("History:");
//  
//  this->GetApplication()->Script("pack %s -side left -anchor ne -expand y -fill both -padx 2 -pady 2", 
//          pgWorkflow->GetWidgetName());
  
  
  vtkCallbackCommand *nextBtnClicked = vtkCallbackCommand::New();
  nextBtnClicked->SetClientData(this);
  nextBtnClicked->SetCallback(&vtkKWMyWizardWidget::NextButtonClicked);
  this->NextButton->AddObserver(vtkKWPushButton::InvokedEvent, nextBtnClicked);
  
  vtkCallbackCommand *backBtnClicked = vtkCallbackCommand::New();
  backBtnClicked->SetClientData(this);
  backBtnClicked->SetCallback(&vtkKWMyWizardWidget::BackButtonClicked);
  this->BackButton->AddObserver(vtkKWPushButton::InvokedEvent, backBtnClicked);
  
  vtkCallbackCommand *navStackChanged = vtkCallbackCommand::New();
  navStackChanged->SetClientData(this);
  navStackChanged->SetCallback(&vtkKWMyWizardWidget::NavigationStackChanged);
  this->AddObserver(vtkKWWizardWorkflow::NavigationStackedChangedEvent, navStackChanged);
  
  if(!this->m_itemToStepMap)
  {
      this->m_itemToStepMap = new std::map<int, vtkKWWizardStep*>;
  }
    
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

void vtkKWMyWizardWidget::NavigationStackChanged(vtkObject* obj, unsigned long,void* callbackData, void* clientData)
{
    vtkKWMyWizardWidget *myWizWidg = (vtkKWMyWizardWidget*)callbackData;            
    myWizWidg->UpdateNavigationGUI();
}

int vtkKWMyWizardWidget::GetNumberOfUnprocessedSteps()
{
    return this->m_numberOfUnprocessedSteps;
}

void vtkKWMyWizardWidget::SetNumberOfUnprocessedSteps(int steps)
{
    this->m_numberOfUnprocessedSteps = steps;
}

void vtkKWMyWizardWidget::SetNumberOfProcessedSteps(int steps)
{
    this->m_numberOfProcessedSteps = steps;
}

void vtkKWMyWizardWidget::UpdateNavigationGUI()
{
    double percent = 0;
    if(this->WizardWorkflow->GetCurrentStep() == this->WizardWorkflow->GetFinishStep())
    {
        percent = 100;
    }
    else if(this->WizardWorkflow->GetCurrentStep() == this->WizardWorkflow->GetInitialStep())
    {
        percent = 0;
    }
    else
    {
        // subtract 2 from the actual navigation stack because of the intial and last step
        int stepAmount = this->m_numberOfUnprocessedSteps + this->m_numberOfProcessedSteps;
        int processedSteps = this->m_numberOfProcessedSteps;
        percent = (processedSteps * 100 / stepAmount);        
    }
            
    this->ProgressGauge->SetValue(percent);
    
    // Update ComboBox
    
    std::map<int, vtkKWWizardStep*>::iterator iter;
    bool found = false;
    int selectedIndex = 0;
    
    std::cout<<this->m_numberOfProcessedSteps + 1 <<" < "<<this->ComboBox->GetWidget()->GetNumberOfValues()<<std::endl;
    if(this->m_numberOfProcessedSteps < this->ComboBox->GetWidget()->GetNumberOfValues())
    {
            found = true;
            selectedIndex = this->m_numberOfProcessedSteps;
    }    
    
    if(!found)
    {
        selectedIndex = this->ComboBox->GetWidget()->GetNumberOfValues();
        
        std::ostringstream strvalue;                
        strvalue << selectedIndex;
        strvalue << ends;
        std::string value = strvalue.str().c_str();
        value.append(": ");
        std::string name = this->GetTitle();
        value.append(name);
        this->ComboBox->GetWidget()->AddValue(value.c_str());
        this->m_itemToStepMap->insert(std::make_pair(selectedIndex, this->WizardWorkflow->GetCurrentStep()));        
    }
    std::string temp = this->ComboBox->GetWidget()->GetValueFromIndex(selectedIndex);
    std::cout<<temp<<std::endl;
    this->ComboBox->GetWidget()->SetValue(temp.c_str());
}

void vtkKWMyWizardWidget::Delete()
{
    if(this->ProgressGauge)
    {
        this->ProgressGauge->Unpack();
        this->ProgressGauge->Delete();
        this->ProgressGauge = NULL;
    }
    
    if(this->ComboBox)
    {
        this->ComboBox->Unpack();
        this->ComboBox->Delete();
        this->ComboBox = NULL;
    }
    
    this->Superclass::Delete();
}

void vtkKWMyWizardWidget::ComboBoxEntryChanged(const char* value)
{
    this->InvokeEvent(vtkKWMyWizardWidget::comboBoxEntryChanged);    
}

void vtkKWMyWizardWidget::RemoveAllObservers()
{
    
}

const char* vtkKWMyWizardWidget::GetCurrentComboBoxValue()
{
    return this->ComboBox->GetWidget()->GetValue();
}

int vtkKWMyWizardWidget::GetCurrentComboBoxIndex()
{
    return this->ComboBox->GetWidget()->GetValueIndex(this->GetCurrentComboBoxValue());
}

vtkKWWizardStep *vtkKWMyWizardWidget::GetGotoWFStep()
{
    std::map<int, vtkKWWizardStep*>::iterator iter = this->m_itemToStepMap->find(this->GetCurrentComboBoxIndex());
    if(iter != this->m_itemToStepMap->end())
    {
        return iter->second;
    }
    return NULL;
}
