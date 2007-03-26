#include "vtkWFEngineModuleGUI.h"

#include "vtkMRMLWFEngineModuleNode.h"
//#include <WFDirectInterface.h>

#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>

#include <vtkKWWidget.h>
#include <vtkKWMenu.h>
#include <vtkKWMenuButtonWithLabel.h>
#include <vtkKWMultiColumnList.h>
#include <vtkKWMultiColumnListWithScrollbars.h>
#include <vtkKWPushButtonSet.h>
#include <vtkKWPushButton.h>
#include <vtkKWLabel.h>

#include <vtkKWWidget.h>
#include <vtkKWFrame.h>
#include "vtkKWMyWizardWidget.h"
#include <vtkKWWizardStep.h>
#include "vtkKWWizardWorkflow.h"
#include <vtkKWStateMachineInput.h>
#include <vtkKWSpinBoxWithLabel.h>
#include <vtkKWSpinBox.h>
#include <vtkKWScaleWithEntry.h>
#include <vtkKWCheckButtonWithLabel.h>
#include <vtkKWEntryWithLabel.h>
#include <vtkKWEntry.h>
#include <vtkSlicerNodeSelectorWidget.h>
#include <vtkKWLoadSaveButtonWithLabel.h>
#include <vtkKWRadioButtonSetWithLabel.h>
#include <vtkKWRadioButton.h>
#include <vtkSlicerModelsGUI.h>
#include <vtkSlicerApplication.h>
#include <vtkSlicerModuleLogic.h>
#include <vtkSlicerVisibilityIcons.h>
#include <vtkSlicerModuleCollapsibleFrame.h>

#include <ModuleDescription.h>
#include <ModuleDescriptionParser.h>
#include <vtkSlicerParameterWidget.h>
#include <vtkSlicerModuleLogic.h>

#include "WFStateConverter.h"
#include "vtkWFEngineHandler.h"
#include "vtkWFEngineEventHandler.h"

#include <vtkKWMessageDialog.h>

#include <map>
#include <iostream>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkWFEngineModuleGUI );
vtkCxxRevisionMacro ( vtkWFEngineModuleGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkWFEngineModuleGUI::vtkWFEngineModuleGUI ( )
{
    this->Logic = NULL;
    this->m_curStepID = -1;
    this->m_curWFStep = NULL;
    this->m_curWizWidg = NULL;
    this->m_selectedWF = -1;
    this->m_mclDW = NULL;
    this->m_pbtnSet = NULL;
//    this->m_wfDI = NULL;
    this->m_wizFrame = NULL;
    this->m_curNameToValueMap = NULL;
    this->m_wfEngineHandler = NULL;    
    this->m_wfEngineEventHandler = NULL;
    this->m_curParameterWidgets = NULL;
    //this->DebugOn();
    
}


//---------------------------------------------------------------------------
vtkWFEngineModuleGUI::~vtkWFEngineModuleGUI ( )
{
    this->deleteWizardWidgetContainer();
    
    if(this->Logic)
    {
        this->SetModuleLogic ( NULL );
    }    
    
    this->m_curStepID = -1;
    
//    delete(this->m_curWFStep)
    this->m_curWFStep = NULL;
    
    this->m_selectedWF = -1;
    
    if(this->m_mclDW)
    {
        this->m_mclDW->GetWidget()->RemoveAllObservers();
        this->m_mclDW->Delete();
        this->m_mclDW = NULL;        
    }
    
    if(this->m_pbtnSet)
    {
        for(int i = 0; i < this->m_pbtnSet->GetNumberOfWidgets(); i++)
        {
            this->m_pbtnSet->GetWidget(i)->RemoveAllObservers();
        }
        this->m_pbtnSet->Delete();
        this->m_pbtnSet = NULL;
    }    
//    this->m_wfDI = NULL;
        
}


//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "WFEngineModuleGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";

    // print widgets?
//    os << indent << "LoadSceneButton" << this->GetLoadSceneButton ( ) << "\n";
}



//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::RemoveGUIObservers ( )
{
  vtkDebugMacro("vtkWFEngineModuleGUI: RemoveGUIObservers\n");
}


//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkWFEngineModuleGUI: AddGUIObservers\n");      
}



//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData, void *clientData )
{
    // nothing to do here yet...
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
  
  vtkWFEngineModuleGUI *wfEngineModuleGUI = (vtkWFEngineModuleGUI*)callData;
  
  if(!wfEngineModuleGUI)
  {
      return;
  }
  
  if(m)
  {
      int menuIndex = *(int*)(clientData);
      std::cout<<"Menu item "<<menuIndex<<" was pressed!"<<std::endl;
      if(menuIndex == 5)
      {
          wfEngineModuleGUI->UpdateWorkflowList();
      }
  }
  
  return;
}

//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{    
}


//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::Enter ( )
{
    vtkDebugMacro("vtkWFEngineModuleGUI: Enter\n");
    std::cout<<"ENTER"<<std::endl;
}

//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::Exit ( )
{
    vtkDebugMacro("vtkWFEngineModuleGUI: Exit\n");
}


//---------------------------------------------------------------------------
void vtkWFEngineModuleGUI::BuildGUI ( )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    // Define your help text here.


    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "WFEngineModule", "WFEngineModule", NULL );

    const char *help = "The WFEngine Module helps users with an easy implementation of a workflow driven user interface.";
    const char *about = "This work was supported by BIRN, NA-MIC, NAC, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "WFEngineModule" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    
    //Insert here the WFEngine GUI
    // ---
    // LOAD FRAME            
    vtkSlicerModuleCollapsibleFrame *loadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadFrame->SetParent ( page );
    loadFrame->Create ( );
    loadFrame->SetLabelText ("Loadable Workflows");
//    loadFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  loadFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("WFEngineModule")->GetWidgetName());
    
    this->m_mclDW = vtkKWMultiColumnListWithScrollbars::New();    
    this->m_mclDW->SetParent(loadFrame->GetFrame());
    this->m_mclDW->Create();
    
    this->m_mclDW->GetWidget()->AddColumn("Workflow-Name");
    this->m_mclDW->GetWidget()->AddColumn("File-Name");
    this->m_mclDW->GetWidget()->AddColumn("Created");
    
    vtkCallbackCommand *mclDWSelectionChangedCmd = vtkCallbackCommand::New();
    mclDWSelectionChangedCmd->SetClientData(this);
    mclDWSelectionChangedCmd->SetCallback(&vtkWFEngineModuleGUI::mclDWSelectionChangedCallback);
    
    this->m_mclDW->GetWidget()->AddObserver(vtkKWMultiColumnList::SelectionChangedEvent, mclDWSelectionChangedCmd);
    
    mclDWSelectionChangedCmd->Delete();
    mclDWSelectionChangedCmd = NULL;
    
    
    app->Script("pack %s -side top -anchor nw -expand y -fill both -padx 2 -pady 2", 
            m_mclDW->GetWidgetName());
    
    vtkKWFrame *buttonFrame = vtkKWFrame::New();
    buttonFrame->SetParent(page);
    buttonFrame->Create();
    buttonFrame->SetBorderWidth(2);
    buttonFrame->SetReliefToFlat();
    
    
    app->Script("pack %s -side top -anchor se -expand n -fill x -padx 2 -pady 2", 
            buttonFrame->GetWidgetName());
        
    vtkKWMenuButtonWithLabel *mbWFOpts = vtkKWMenuButtonWithLabel::New();
    mbWFOpts->SetParent(buttonFrame);
    mbWFOpts->Create();
    mbWFOpts->SetBalloonHelpString("Workflow Module Options");
    
    mbWFOpts->SetReliefToFlat();
    mbWFOpts->GetWidget()->IndicatorVisibilityOff();
    mbWFOpts->SetLabelText("Options:");
    mbWFOpts->GetWidget()->GetMenu()->AddCommand("Load configuration..", this, "ProcessGUIEvents");
    mbWFOpts->GetWidget()->GetMenu()->AddCommand("Save configuration..");
    mbWFOpts->GetWidget()->GetMenu()->AddCommand("Save configuration as..");
    mbWFOpts->GetWidget()->GetMenu()->AddSeparator();
    mbWFOpts->GetWidget()->GetMenu()->AddCommand("Add workflow description..");
    mbWFOpts->GetWidget()->GetMenu()->AddCommand("Update workflow list..");
    
    mbWFOpts->GetWidget()->SetValue("WorkflowEngine options");
    
    vtkCallbackCommand *menuBtnItemInvoked = vtkCallbackCommand::New();
    menuBtnItemInvoked->SetClientData(this);
    menuBtnItemInvoked->SetCallback(&vtkWFEngineModuleGUI::ProcessGUIEvents);
    
    mbWFOpts->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, menuBtnItemInvoked);
    
    menuBtnItemInvoked->Delete();
    menuBtnItemInvoked = NULL;
    
    app->Script("pack %s -side left -anchor nw -expand n -fill none -padx 2 -pady 2", 
            mbWFOpts->GetWidgetName());
    
    if(mbWFOpts)
    {
        mbWFOpts->Delete();
        mbWFOpts = NULL;
    }    

    this->m_pbtnSet = vtkKWPushButtonSet::New();
    this->m_pbtnSet->SetParent(buttonFrame);
    this->m_pbtnSet->PackHorizontallyOn();
    
    this->m_pbtnSet->Create();
    
    vtkKWPushButton *pbtn = this->m_pbtnSet->AddWidget(0);
    pbtn->SetText("Load");
    pbtn->SetEnabled(0);
    
    vtkCallbackCommand *loadBtnPushCmd = vtkCallbackCommand::New();
    loadBtnPushCmd->SetCallback(&vtkWFEngineModuleGUI::loadBtnPushCmdCallback);
    loadBtnPushCmd->SetClientData(this);
        
    pbtn->AddObserver(vtkKWPushButton::InvokedEvent, loadBtnPushCmd);
    
    loadBtnPushCmd->Delete();
    loadBtnPushCmd = NULL;
//    pbtn = m_pbtnSet->AddWidget(1);
//    pbtn->SetText("Close");
//    pbtn->SetEnabled(1);
    
       
    app->Script("pack %s -side right -anchor se -expand n -fill none -padx 2 -pady 2", 
            m_pbtnSet->GetWidgetName()); 
        
    if(buttonFrame)
    {
        buttonFrame->Delete();
        buttonFrame = NULL;
    }
    
    if(loadFrame)
    {
        loadFrame->Delete();
        loadFrame = NULL;
    }
        
//    // add button to load a scene. this is wrong widget, but for now let it sit.
//    this->LoadSceneButton = vtkKWPushButton::New ( );
//    this->LoadSceneButton->SetParent ( loadFrame->GetFrame() );
//    this->LoadSceneButton->Create();
//    this->LoadSceneButton->SetText ( "LoadScene" );
//    this->LoadSceneButton->SetBalloonHelpString ( "Select all search terms for use");
//    app->Script ( "pack %s -side top -padx 3 -pady 3", this->LoadSceneButton->GetWidgetName() );
    this->ConnectToWFEngine();
}

int vtkWFEngineModuleGUI::ConnectToWFEngine()
{
    this->m_wfEngineHandler = vtkWFEngineHandler::New();
    if(this->m_wfEngineHandler->InitializeWFEngine() == vtkWFEngineHandler::SUCC)
    {
        this->m_wfEngineHandler->SetApplication(this->GetApplication());
        this->UpdateWorkflowList();
    }
}

void vtkWFEngineModuleGUI::addWorkflowToList(const char* workflowName, const char* fileName, int date)
{
    int id = this->m_mclDW->GetWidget()->GetNumberOfRows();
    this->m_mclDW->GetWidget()->InsertCellText(id,0, workflowName);
    this->m_mclDW->GetWidget()->InsertCellText(id,1, fileName);
    
    time_t tim = date; 
    this->m_mclDW->GetWidget()->InsertCellText(id,2, ctime(&tim));    
//    this->listToWFMap.insert(std::make_pair(id, fileName));
}

void vtkWFEngineModuleGUI::SaveState()
{
    
}

void vtkWFEngineModuleGUI::mclDWSelectionChangedCallback(vtkObject* obj, unsigned long,void* param, void*)
{
    vtkWFEngineModuleGUI *myDW = (vtkWFEngineModuleGUI*)param;
    
    if(myDW->m_mclDW->GetWidget()->GetIndexOfFirstSelectedRow() != -1)
    {
        myDW->m_pbtnSet->GetWidget(0)->SetEnabled(1);
    }
    
    if(myDW->m_mclDW->GetWidget()->GetIndexOfFirstSelectedRow() == myDW->m_selectedWF)
    {
        myDW->m_pbtnSet->GetWidget(0)->SetText("Unload");
    }
    else
        myDW->m_pbtnSet->GetWidget(0)->SetText("Load");
}

void vtkWFEngineModuleGUI::createWizard()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "WFEngineModule" );
    //Insert here the WFEngine GUI
    // ---
    // LOAD FRAME
    
    // get Name for selected work-flow
    std::string name = this->m_mclDW->GetWidget()->GetCellText(this->m_mclDW->GetWidget()->GetIndexOfFirstSelectedRow(),0);
    
    this->m_wizFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    m_wizFrame->SetParent ( page );
    m_wizFrame->Create ( );
    m_wizFrame->SetLabelText (name.c_str());
//    loadFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
            m_wizFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("WFEngineModule")->GetWidgetName());
    
    m_curWizWidg = vtkKWMyWizardWidget::New();
    
    m_curWizWidg->SetParent(m_wizFrame->GetFrame());
    m_curWizWidg->Create();
    m_curWizWidg->SetClientAreaMinimumHeight(300);
    
    m_curWizWidg->SetNumberOfUnprocessedSteps(this->m_wfEngineHandler->GetUnprocessedSteps());
    m_curWizWidg->SetNumberOfProcessedSteps(this->m_wfEngineHandler->GetProcessedSteps());
    
    this->GetApplication()->Script("pack %s -side top -anchor ne -expand y -fill both -padx 2 -pady 2", 
            m_curWizWidg->GetWidgetName());
    
    vtkKWWizardWorkflow *wizWorkflow = m_curWizWidg->GetWizardWorkflow();
                  
//    create a virtual first and last step to take cover for the input and out vars in that both steps
    
    vtkKWWizardStep *virtFirstStep = vtkKWWizardStep::New();
//    vtkKWStateMachineState *virtFirstState = vtkKWStateMachineState::New();
    virtFirstStep->SetName("Welcome");
    virtFirstStep->SetDescription("");
    
    wizWorkflow->AddStep(virtFirstStep);
        
    vtkKWWizardStep *virtLastStep = vtkKWWizardStep::New();
    virtLastStep->SetName("Good Bye");
    virtLastStep->SetDescription("");
    
    wizWorkflow->AddNextStep(virtLastStep);
    
    wizWorkflow->SetInitialStep(virtFirstStep);
    wizWorkflow->SetFinishStep(virtLastStep);
    wizWorkflow->CreateGoToTransitionsToFinishStep();
    
    this->m_curWizWidg->UpdateNavigationGUI();
    // Comment:
    // listen to the workflow next and back Events to handle the workflow dynamically
    // 
    
    vtkCallbackCommand *wizCB = vtkCallbackCommand::New();
    
    wizCB->SetCallback(&vtkWFEngineModuleGUI::nextTransitionCallback);
    wizCB->SetClientData(this);
    this->m_curWizWidg->AddObserver(vtkKWMyWizardWidget::nextButtonClicked, wizCB);
    wizCB->Delete();
    wizCB = NULL;
    
    
    wizCB = vtkCallbackCommand::New();
    
    wizCB->SetCallback(&vtkWFEngineModuleGUI::backTransitionCallback);
    wizCB->SetClientData(this);
    this->m_curWizWidg->AddObserver(vtkKWMyWizardWidget::backButtonClicked, wizCB);
    wizCB->Delete();
    wizCB = NULL;
    
    wizCB = vtkCallbackCommand::New();
    
    wizCB->SetCallback(&vtkWFEngineModuleGUI::jumpToStepCallback);
    wizCB->SetClientData(this);
    this->m_curWizWidg->AddObserver(vtkKWMyWizardWidget::comboBoxEntryChanged, wizCB);
    wizCB->Delete();
    wizCB = NULL;
    // create a new MRMLNode to get track of all workflow variables
    
    vtkMRMLWFEngineModuleNode *myWFENode = vtkMRMLWFEngineModuleNode::New();
    myWFENode->SetName(name.c_str());
    myWFENode->SetDescription("WFEngineModule: Current selected work-flow node");
    myWFENode->SetScene(this->Logic->GetMRMLScene());
    this->Logic->GetMRMLScene()->AddNode(myWFENode);
    this->SetWFEngineModuleNode(myWFENode);
    
    // create WFEngineEventHandler and create all Observers
    
    this->m_wfEngineEventHandler = vtkWFEngineEventHandler::New();
    this->m_wfEngineEventHandler->SetApplication(this->GetApplication());
//    this->m_wfEngineEventHandler->SetMRMLScene(this->Logic->GetMRMLScene());
    this->m_wfEngineEventHandler->AddWorkflowObservers(this->m_wfEngineHandler);
    
    
    this->m_wfEngineHandler->SetWFMRMLNode(myWFENode);
    
    this->m_wfEngineHandler->SetWizardClientArea(this->m_curWizWidg->GetClientArea());
        
    virtFirstStep->Delete();
    virtFirstStep = NULL;
    
    virtLastStep->Delete();
    virtLastStep = NULL;
    
    myWFENode->Delete();
    myWFENode = NULL;        
}

void vtkWFEngineModuleGUI::closeBtnPushCmdCallback(vtkObject* obj, unsigned long, void* param, void*)
{
    vtkWFEngineModuleGUI *myDW = (vtkWFEngineModuleGUI*)param;
    myDW->SaveState();
    myDW->closeWorkflow();
//    myDW->OK();
}

void vtkWFEngineModuleGUI::loadBtnPushCmdCallback(vtkObject* obj, unsigned long, void* param, void*)
{
    vtkWFEngineModuleGUI *myDW = (vtkWFEngineModuleGUI*)param;
    std::string selectedWF = myDW->m_mclDW->GetWidget()->GetCellText(myDW->m_mclDW->GetWidget()->GetIndexOfFirstSelectedRow(), 1); 
    vtkKWPushButton *loadBtn = vtkKWPushButton::SafeDownCast(obj);
    if(loadBtn != NULL && myDW->m_wizFrame != NULL)
    {        
        myDW->deleteWizardWidgetContainer();
        
        vtkKWWidget *page = myDW->UIPanel->GetPageWidget ( "WFEngineModule" );
        for(int i = 0; i < page->GetNumberOfChildren(); i++)
        {
            vtkSlicerModuleCollapsibleFrame *curFrame = vtkSlicerModuleCollapsibleFrame::SafeDownCast(page->GetNthChild(i));
            if(curFrame)
            {
                curFrame->ExpandFrame();
            }
        }
        
        loadBtn->SetText("Load");
//        myDW->m_nbDW->SetPageEnabled(myDW->m_selWF,0);
        myDW->m_wfEngineHandler->CloseWorkflowSession();
        myDW->m_selectedWF = -1;
    }
    else if(loadBtn != NULL && myDW->m_wizFrame == NULL)
    {
        myDW->m_wfEngineHandler->LoadNewWorkflowSession(selectedWF);
        
        vtkKWWidget *page = myDW->UIPanel->GetPageWidget ( "WFEngineModule" );
        for(int i = 0; i < page->GetNumberOfChildren(); i++)
        {
            vtkSlicerModuleCollapsibleFrame *curFrame = vtkSlicerModuleCollapsibleFrame::SafeDownCast(page->GetNthChild(i));
            if(curFrame)
            {
                curFrame->CollapseFrame ( );
            }
        }
        myDW->createWizard();
//        myDW->m_nbDW->RaisePage(myDW->m_selWF);
        myDW->m_mclDW->GetWidget()->SetRowBackgroundColor(myDW->m_mclDW->GetWidget()->GetIndexOfFirstSelectedRow(),128,255,128);
        myDW->m_selectedWF = myDW->m_mclDW->GetWidget()->GetIndexOfFirstSelectedRow();
        myDW->m_mclDW->GetWidget()->InvokeEvent(vtkKWMultiColumnList::SelectionChangedEvent, myDW);        
    }    
}

void vtkWFEngineModuleGUI::closeWorkflow()
{
    this->m_wfEngineHandler->CloseWorkflowSession();
}

void vtkWFEngineModuleGUI::workStepValidationCallBack(WFEngine::nmWFStepObject::WFStepObject *nextWS)
{
    vtkKWWizardWorkflow *curWF = this->m_curWizWidg->GetWizardWorkflow();    
    std::cout<<"Steps in navigation stack: "<<curWF->GetNumberOfStepsInNavigationStack()<<std::endl;
    
    if(nextWS)
    {
        //initialize TCL Conditions
//        this->initializeTCLConditions(nextWS);
        
        WFStateConverter *wfSC = new WFStateConverter(nextWS);
        vtkKWWizardStep *nextStep = wfSC->GetKWWizardStep();
                
//        nextStep->SetValidateCommand(this, "workStepValidationCallBack");
        nextStep->SetShowUserInterfaceCommand(this, "workStepGUICallBack");
        curWF->AddStep(nextStep);
        
        vtkKWStateMachineInput *validStepInput = vtkKWStateMachineInput::New();
        validStepInput->SetName("nextStepValidationInput");
        
        curWF->AddInput(validStepInput);
        
        curWF->CreateNextTransition(curWF->GetCurrentStep(), validStepInput, nextStep);
        curWF->CreateBackTransition(curWF->GetCurrentStep(), nextStep);
        
        curWF->CreateNextTransition(
                nextStep,
                vtkKWWizardStep::GetValidationSucceededInput(),
                curWF->GetFinishStep());
        curWF->CreateBackTransition(nextStep, curWF->GetFinishStep());
        
        curWF->CreateGoToTransitionsToFinishStep();
        
        curWF->PushInput(validStepInput);
        
        this->m_curWFStep = nextWS;
        
        if(validStepInput)
        {
            validStepInput->Delete();
            validStepInput = NULL;
        }
        
        if(wfSC)
        {
            wfSC->Destroy();
            wfSC = NULL;
        }
    }
    else
    {
        curWF->CreateNextTransition(
                curWF->GetCurrentStep(),
                vtkKWWizardStep::GetValidationSucceededInput(),
                curWF->GetFinishStep());
        curWF->CreateBackTransition(curWF->GetFinishStep(),
                curWF->GetCurrentStep());
        curWF->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
        this->m_curWFStep = NULL;
    }
    curWF->ProcessInputs();
    this->m_curStepID = curWF->GetCurrentStep()->GetId();
}

void vtkWFEngineModuleGUI::nextTransitionCallback(vtkObject* obj, unsigned long id,void* callBackData, void*)
{
    std::cout<<"nextTransitionCallback: "<<id<<std::endl;
    vtkWFEngineModuleGUI *wfEngineModule = (vtkWFEngineModuleGUI*)callBackData;
    if(wfEngineModule)
    {
        int result = vtkKWMessageDialog::StatusOK;
        if(wfEngineModule->m_wfEngineHandler->GetProcessedSteps() > wfEngineModule->m_curWizWidg->GetCurrentComboBoxIndex())
        {
            std::cout<<"pressed next not on the last known step - ask if this is right!"<<std::endl;
            vtkKWMessageDialog *warnDialog = vtkKWMessageDialog::New();
            
            warnDialog->SetParent(wfEngineModule->UIPanel->GetPageWidget ( "WFEngineModule" ));
            warnDialog->SetMasterWindow(wfEngineModule->GetApplication()->GetNthWindow(0));
            warnDialog->SetStyleToYesNo();
            warnDialog->Create();
            //warnDialog->SetPosition(10, 10);
            //warnDialog->SetSize(300, 300);
            warnDialog->SetTitle("Workflow warning!");
            warnDialog->SetText("If you proceed you loose all your data beyond the current Step!");
            warnDialog->Invoke();
            result = warnDialog->GetStatus();
            
            warnDialog->Delete();
            warnDialog = NULL;
        }
        wfEngineModule->UpdateMRML();
        wfEngineModule->UpdateParameter();
        
        if(wfEngineModule->m_curWizWidg)
        {
            // Destroy all ClientAreaChildren!
            wfEngineModule->m_curWizWidg->ClearPage();        
        }
        
        //check step validation   
        if(result == vtkKWMessageDialog::StatusOK && wfEngineModule->m_wfEngineHandler->LoadNextWorkStep() == vtkWFEngineHandler::SUCC )
        {            
            wfEngineModule->m_curWizWidg->SetErrorText("");
            wfEngineModule->m_curWizWidg->SetNumberOfUnprocessedSteps(wfEngineModule->m_wfEngineHandler->GetUnprocessedSteps());
            wfEngineModule->m_curWizWidg->SetNumberOfProcessedSteps(wfEngineModule->m_wfEngineHandler->GetProcessedSteps());            
            wfEngineModule->workStepValidationCallBack(wfEngineModule->m_wfEngineHandler->GetLoadedWFStep());          
            wfEngineModule->m_curWizWidg->UpdateNavigationGUI();
        }
        else if(result == vtkKWMessageDialog::StatusOK)
        {
            wfEngineModule->m_curWizWidg->SetErrorText(wfEngineModule->m_wfEngineHandler->GetLastError().c_str());
            wfEngineModule->UpdateGUI();
            
            wfEngineModule->m_curWizWidg->GetWizardWorkflow()->PushInput(vtkKWWizardStep::GetValidationFailedInput());
        } else {
            wfEngineModule->m_curWizWidg->SetErrorText("Jump to the last step to store the input data!");
            wfEngineModule->UpdateGUI();
            
            wfEngineModule->m_curWizWidg->GetWizardWorkflow()->PushInput(vtkKWWizardStep::GetValidationFailedInput());
        }
    }
}

void vtkWFEngineModuleGUI::backTransitionCallback(vtkObject* obj, unsigned long id,void* callBackData, void*)
{
    std::cout<<"backTransitionCallback: "<<id<<std::endl;
    vtkWFEngineModuleGUI *wfEngineModule = (vtkWFEngineModuleGUI*)callBackData;
    if(wfEngineModule)
    {
        if(wfEngineModule->m_curWizWidg)
        {
            // Destroy all ClientAreaChildren!
            wfEngineModule->m_curWizWidg->ClearPage();        
        }
        
        vtkKWWizardWorkflow *wizWF = wfEngineModule->m_curWizWidg->GetWizardWorkflow();
        if(wfEngineModule->m_wfEngineHandler->LoadBackWorkStep() == vtkWFEngineHandler::SUCC)
        {                  
            wfEngineModule->workStepValidationCallBack(wfEngineModule->m_wfEngineHandler->GetLoadedWFStep());
            wfEngineModule->m_curWizWidg->UpdateNavigationGUI();
        }
        else
        {
            wfEngineModule->m_curWFStep = NULL;
            wfEngineModule->m_curWizWidg->GetWizardWorkflow()->AttemptToGoToPreviousStep();            
            wfEngineModule->m_curWizWidg->UpdateNavigationGUI();
        }
        
        wfEngineModule->workStepGUICallBack();
        //because there is no GUI callback in a backtransition we call this manually
        
        wfEngineModule->m_curWizWidg->Update();
    }
}

void vtkWFEngineModuleGUI::jumpToStepCallback(vtkObject* obj, unsigned long id,void* callBackData, void*)
{
    std::cout<<"jumpToStepCallback: "<<id<<std::endl;
    vtkWFEngineModuleGUI *wfEngineModule = (vtkWFEngineModuleGUI*)callBackData;
    if(wfEngineModule)
    {
        if(wfEngineModule->m_curWizWidg)
        {
            std::cout<<"Value: "<<wfEngineModule->m_curWizWidg->GetCurrentComboBoxValue();
            std::cout<<" Index: "<<wfEngineModule->m_curWizWidg->GetCurrentComboBoxIndex()<<" == "<<wfEngineModule->m_wfEngineHandler->GetProcessedSteps()<<std::endl;
            
            // Destroy all ClientAreaChildren!
            wfEngineModule->m_curWizWidg->ClearPage();
            
            if(wfEngineModule->m_wfEngineHandler->LoadWorkStepByIndex(wfEngineModule->m_curWizWidg->GetCurrentComboBoxIndex() - 1) == vtkWFEngineHandler::SUCC)
            {
                wfEngineModule->m_curWFStep = wfEngineModule->m_wfEngineHandler->GetLoadedWFStep();
                vtkKWWizardStep *gotoStep = wfEngineModule->m_curWizWidg->GetGotoWFStep();
                vtkKWWizardStep *currentStep = wfEngineModule->m_curWizWidg->GetWizardWorkflow()->GetCurrentStep();
                if(gotoStep)
                {
                    std::cout<<"GotoStep: "<<gotoStep->GetName()<<" - "<<gotoStep->GetDescription()<<std::endl;
                    wfEngineModule->m_curWizWidg->GetWizardWorkflow()->CreateGoToTransition(currentStep, gotoStep);
                    wfEngineModule->m_curWizWidg->GetWizardWorkflow()->TryToGoToStepCallback(gotoStep, currentStep);
                }                
                wfEngineModule->m_curWizWidg->UpdateNavigationGUI();
            }
            else
            {
                wfEngineModule->m_curWFStep = NULL;                
                vtkKWWizardStep *gotoStep = NULL;
                if(wfEngineModule->m_curWizWidg->GetCurrentComboBoxIndex() == wfEngineModule->m_wfEngineHandler->GetProcessedSteps())
                {
                    gotoStep = wfEngineModule->m_curWizWidg->GetWizardWorkflow()->GetFinishStep();
                    std::cout<<"FinishStep: "<<gotoStep->GetName()<<" - "<<gotoStep->GetDescription()<<std::endl;
                }
                else
                {
                    gotoStep = wfEngineModule->m_curWizWidg->GetWizardWorkflow()->GetInitialStep();
                    std::cout<<"InitialStep: "<<gotoStep->GetName()<<" - "<<gotoStep->GetDescription()<<std::endl;
                }                
                vtkKWWizardStep *currentStep = wfEngineModule->m_curWizWidg->GetWizardWorkflow()->GetCurrentStep();                
                wfEngineModule->m_curWizWidg->GetWizardWorkflow()->CreateGoToTransition(currentStep, gotoStep);
                wfEngineModule->m_curWizWidg->GetWizardWorkflow()->TryToGoToStepCallback(gotoStep, currentStep);                
                wfEngineModule->m_curWizWidg->UpdateNavigationGUI();
            }
        }
        wfEngineModule->workStepGUICallBack();
        //because there is no GUI callback in a backtransition we call this manually
        
        wfEngineModule->m_curWizWidg->Update();
    }
    
}


void vtkWFEngineModuleGUI::workStepGUICallBack()
{
    this->UpdateGUI();
}

void vtkWFEngineModuleGUI::deleteWizardWidgetContainer()
{
    std::cout<<"deleteWizardWidgetContainer"<<std::endl;
    
    if(this->m_curWizWidg)
    {
        this->m_curWizWidg->RemoveAllObservers();
    }
    
    if(this->m_wizFrame)
    {     
        this->m_wizFrame->UnpackChildren();
//        this->m_wizFrame->Unpack();
    }
    
    if(this->m_curParameterWidgets)
    {        
        this->m_curParameterWidgets->Delete();
        this->m_curParameterWidgets = NULL;
        std::cout<<"m_curParameterWidgets"<<std::endl;
    }       
    
    if(this->m_curWizWidg)
    {
        this->m_curWizWidg->Delete();
        this->m_curWizWidg = NULL;
        std::cout<<"m_curWizWidg"<<std::endl;
    }    
    
    if(this->m_wizFrame)
    {     
        this->m_wizFrame->RemoveAllChildren();
        this->m_wizFrame->Delete();
        this->m_wizFrame = NULL;
        std::cout<<"m_wizFrame"<<std::endl;
    }
    
    if(this->m_wfEngineEventHandler)
    {
        this->m_wfEngineEventHandler->Delete();
        this->m_wfEngineEventHandler = NULL;
        std::cout<<"m_wfEngineEventHandler"<<std::endl;
    }
    
    this->m_curNameToValueMap = NULL;       
}

void vtkWFEngineModuleGUI::UpdateMRML()
{
    //check if information is available
    if(!this->m_curNameToValueMap)
    {
        return;
    }
    
    vtkMRMLWFEngineModuleNode* n = this->GetWFEngineModuleNode();
    bool createdNode = false;
    if (n == NULL)
    {
        n = vtkMRMLWFEngineModuleNode::New();
        
        if (n == NULL)
        {
//        this->InUpdateMRML = false;
        vtkDebugMacro("No CommandLineModuleNode available");
        return;
        }
      
      // set the a module description for this node
//      n->SetModuleDescription( this->ModuleDescriptionObject );
      
      // set an observe new node in Logic
//      this->Logic->SetCommandLineModuleNode(n);
      vtkSetAndObserveMRMLNodeMacro(this->WFEngineModuleNode,n);

      createdNode = true;
    }
    
    std::map<std::string, std::string>::iterator iter;        
    
    for(iter = this->m_curNameToValueMap->begin(); iter != this->m_curNameToValueMap->end(); iter++)
    {
        
    }//for
    
    //set the map back to null
//    this->m_curNameToWidgetMap = NULL;
}

void vtkWFEngineModuleGUI::SetWFEngineModuleNode(vtkMRMLWFEngineModuleNode *node)
{
    this->WFEngineModuleNode = node;
}

void vtkWFEngineModuleGUI::widgetChangedCallback(vtkObject* obj, unsigned long eid, void* clientData, void* callData)
{
    std::cout<<"something Changed Called Back;-)"<<std::endl;
    
    vtkWFEngineModuleGUI *wfEngineModule = (vtkWFEngineModuleGUI*)obj;
    if(wfEngineModule)
    {
        wfEngineModule->m_ParameterWidgetChanged = true;
    }    
}

const char *vtkWFEngineModuleGUI::getStepInputValueByName(std::string name)
{
    //check if information is available
    if(!this->WFEngineModuleNode)
    {
        return "";
    }
    
    if(!this->m_curWFStep)
    {
        return "";
    }
    
    std::string curAttributeName = this->m_curWFStep->GetID();
    curAttributeName.append("." + name);
    
    std::cout<<curAttributeName<<std::endl;
    return this->WFEngineModuleNode->GetAttribute(curAttributeName.c_str());
                   
}

void vtkWFEngineModuleGUI::UpdateWorkflowList()
{
    std::vector<vtkWFEngineHandler::workflowDesc*> *tmpWFDescList = this->m_wfEngineHandler->GetKnownWorkflowDescriptions();
    std::vector<vtkWFEngineHandler::workflowDesc*>::iterator iter;
    
    if(this->m_mclDW)
    {
        this->m_mclDW->GetWidget()->DeleteAllRows();
        for(iter = tmpWFDescList->begin(); iter != tmpWFDescList->end(); iter++)
        {
            vtkWFEngineHandler::workflowDesc *tmpWFDesc = *iter;
            this->addWorkflowToList(tmpWFDesc->workflowName.c_str(), tmpWFDesc->fileName.c_str(), tmpWFDesc->createTime);
        }
    }        
}

void vtkWFEngineModuleGUI::UpdateParameter()
{
    if(this->m_wfEngineHandler && this->m_curNameToValueMap)
    {
        std::map<std::string, std::string>::iterator iter;
        for(iter = this->m_curNameToValueMap->begin(); iter != this->m_curNameToValueMap->end(); iter++)
        {
            const char* value = this->getStepInputValueByName(iter->first);
            std::string strValue = value;                       
            this->m_wfEngineHandler->AddParameter(iter->first.c_str(), value);
        }        
    }
}

void vtkWFEngineModuleGUI::UpdateGUI()
{
    if(!this->m_wfEngineHandler)
    {
        return;
    }
    
    if(!this->m_curWFStep)
    {
        return;
    }
    
    
    this->m_ParameterWidgetChanged = false;
//    ModuleDescription curModuleDesc;
//    
//    ModuleDescriptionParser curMDParser;
//    
//    std::string guiDesc = this->m_curWFStep->GetGUIDescription();
//    std::cout<<"guiParser: ";
//    std::cout<<curMDParser.Parse(guiDesc, curModuleDesc)<<std::endl;
    
    if(!this->m_curParameterWidgets)
    {
        this->m_curParameterWidgets = vtkSlicerParameterWidget::New();
    }
        
    this->m_curParameterWidgets->SetApplication(this->GetApplication());
    vtkSlicerModuleLogic *myModuleLogic = vtkSlicerModuleLogic::New();
    
    this->m_curParameterWidgets->SetParent(this->m_curWizWidg->GetClientArea());
    this->m_curParameterWidgets->SetSlicerModuleLogic(myModuleLogic);
    
    // set ModuleNode to the new step
    this->m_curParameterWidgets->SetWidgetID(this->m_curWFStep->GetID());
    this->m_curParameterWidgets->SetMRMLNode(this->GetWFEngineModuleNode());
    
    this->m_curParameterWidgets->SetModuleDescription(this->m_wfEngineHandler->GetCurrentModuleDescription());
    this->m_curParameterWidgets->SetErrorMap(this->m_wfEngineHandler->GetValidationErrorMap());
    
    if(this->m_curParameterWidgets->CreateWidgets() != vtkSlicerParameterWidget::SUCC 
            || this->m_curParameterWidgets->size() == 0)
    {
        if(this->m_curNameToValueMap)
        {
            this->m_curNameToValueMap->clear();
            this->m_curNameToValueMap = NULL;
        }        
        this->m_curParameterWidgets->Delete();
        this->m_curParameterWidgets = NULL;
        return;
    }
    
    vtkCallbackCommand *widgetChangedCBC = vtkCallbackCommand::New();    
    widgetChangedCBC->SetClientData(this);
    widgetChangedCBC->SetCallback(vtkWFEngineModuleGUI::widgetChangedCallback);
    this->m_curParameterWidgets->AddObserver(vtkSlicerParameterWidget::ParameterWidgetChangedEvent, widgetChangedCBC);
    
    widgetChangedCBC->Delete();
    widgetChangedCBC = NULL;
    std::cout<<"vector size "<<this->m_curParameterWidgets->size()<<std::endl;        
    
    // Build GUI out of the widget list
    vtkKWWidget *curGroupWidget = this->m_curParameterWidgets->GetNextWidget();
    this->m_curNameToValueMap = new std::map<std::string, std::string>;
    while(curGroupWidget)
    {
        this->GetApplication()->Script("pack %s -side top -anchor ne -expand y -fill both -padx 2 -pady 2",
                curGroupWidget->GetWidgetName());
        std::vector<ModuleParameter> *curParameterList = this->m_curParameterWidgets->GetCurrentParameters();
        
        std::vector<ModuleParameter>::iterator iter;
        for(iter = curParameterList->begin(); iter != curParameterList->end(); iter++)
        {
            this->m_curNameToValueMap->insert(std::make_pair(iter->GetName(),iter->GetDefault()));
        }
        curGroupWidget = this->m_curParameterWidgets->GetNextWidget();  
    }       
}

//void vtkWFEngineModuleGUI::initializeTCLConditions(WFEngine::nmWFStepObject::WFStepObject* curWS)
//{
//    this->m_condChecker = vtkWFStepConditionChecker::New();
//    this->m_condChecker->SetApplication(this->GetApplication());
//    this->m_condChecker->LoadStepValidationFunction(curWS->GetTCLValidationFunction().c_str());
//    this->m_condChecker->LoadNextStepFunction(curWS->GetTCLNextWorkstepFunction().c_str());
////    condChecker->SetTCLInterp(this->GetApplication()->)    
//}

//void vtkWFEngineModuleGUI::validateStep(WFEngine::nmWFStepObject::WFStepObject* curWS)
//{
//    if(this->m_condChecker)
//    {
//        std::map<std::string, std::string>::iterator iter;
//        for(iter = this->m_curNameToValueMap->begin(); iter != this->m_curNameToValueMap->end(); iter++)
//        {
//            std::cout<<"name: "<<iter->first<<" value: "<<iter->second<<std::endl;
//            this->m_condChecker->AddParameter(iter->first.c_str(), iter->second.c_str());
//        }
//        int result = this->m_condChecker->ValidateStep();
//    }
//}
