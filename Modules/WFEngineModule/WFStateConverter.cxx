#include "WFStateConverter.h"
#include "vtkKWStateMachineState.h"
#include "vtkKWWizardStep.h"


#include <iostream>

using namespace WFEngine::nmWFStepObject;

WFStateConverter::WFStateConverter()
{
    this->m_isLoaded = false;    
    this->m_KWState = NULL;    
    this->m_KWWizardStep = NULL;
}

WFStateConverter::WFStateConverter(WFStepObject *wfStep)
{
    this->m_isLoaded = false;
    this->ConvertWFStep(wfStep);
}


WFStateConverter::~WFStateConverter()
{
    if(this->m_KWState)
    {
        this->m_KWState->Delete();
        this->m_KWState = NULL;
    }
    
    if(this->m_KWWizardStep)
    {
        this->m_KWWizardStep->Delete();
        this->m_KWWizardStep = NULL;
    }
    
    this->m_isLoaded = false;
}

void WFStateConverter::ConvertWFStep(WFStepObject *wfStep)
{
    if(wfStep)
    {
        this->m_KWState = this->createKWState(wfStep);
        this->m_KWWizardStep = this->createKWWizardStep(wfStep);
        this->m_isLoaded = true;
    }
}

vtkKWStateMachineState *WFStateConverter::createKWState(WFStepObject *wfStep)
{
    vtkKWStateMachineState *curState = vtkKWStateMachineState::New();
    
    curState->SetName(wfStep->GetName().c_str());
    
    if(wfStep->GetStepDescription() == "")
        curState->SetDescription(wfStep->GetWFDescription().c_str());       
    else
        curState->SetDescription(wfStep->GetStepDescription().c_str());
    
    return curState;
}

vtkKWWizardStep *WFStateConverter::createKWWizardStep(WFStepObject *wfStep)
{
    vtkKWWizardStep *curWS = vtkKWWizardStep::New();
    
    curWS->SetName(wfStep->GetName().c_str());
    if(wfStep->GetStepDescription() == "")
        curWS->SetDescription(wfStep->GetWFDescription().c_str());       
    else
        curWS->SetDescription(wfStep->GetStepDescription().c_str());
    
    return curWS;
}

vtkKWStateMachineState *WFStateConverter::GetKWState()
{
    if(IsLoaded())
        return this->m_KWState;
    else
        std::cout<<"WFStateConverter not loaded!"<<std::endl;            
}

vtkKWWizardStep *WFStateConverter::GetKWWizardStep()
{
    if(IsLoaded())
        return this->m_KWWizardStep;
    else
        std::cout<<"WFStateConverter not loaded!"<<std::endl;            
}

bool WFStateConverter::IsLoaded()
{
    return this->m_isLoaded;
}

void WFStateConverter::Destroy()
{
    delete(this);
}

//vtkKWStateMachineState *WFStateConverter::YAWL2KWState(WFStepObject *wfState)
//{
//    vtkKWStateMachineState *kwState = vtkKWStateMachineState::New();
//    
//    kwState->SetName(wfState->GetName);
//    kwState->SetDescription()
//}
//
//WFStepObject *WFStateConverter::KWState2YAWL(vtkKWStateMachineState *kwState)
//{
//    
//}



