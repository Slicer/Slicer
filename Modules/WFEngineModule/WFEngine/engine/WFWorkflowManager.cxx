#include "WFWorkflowManager.h"
#include "WFStepObject.h"
#include <iostream>

namespace WFEngine
{

namespace nmWFWorkflowManager
{

using namespace WFEngine::nmWFXmlWorkflowManager;
using namespace WFEngine::nmWFStepObject;
    
WFWorkflowManager::WFWorkflowManager() : WFXmlWorkflowManager()
{
//    this->m_curWorkStep = NULL;
    this->m_workSteps.clear();
}

WFWorkflowManager::~WFWorkflowManager()
{
}

WFWorkflowManager *WFWorkflowManager::New()
{
    return new WFWorkflowManager;
}

int WFWorkflowManager::LoadWorkflowFile(std::string wfDescFile)
{
    int retVal = 1;
// this->m_WFXmlWorkflowManager = WFXmlWorkflowManager::New();
    retVal = this->loadWorkflowDescription(wfDescFile);
 
    return retVal;
}

int WFWorkflowManager::SetWorkflowFile(std::string wfDescFile)
{
 if(this->LoadWorkflowFile(wfDescFile))
 {
  this->m_wfDescFile = wfDescFile;
 }
 return IsLoaded();
}

std::string WFWorkflowManager::GetWorkflowFile()
{
 return this->m_wfDescFile;
}

std::string WFWorkflowManager::GetWorkflowName()
{
    return this->getWorkflowName();
}

void WFWorkflowManager::Close()
{
    if(IsLoaded())
    {
        this->destroy();
        this->m_wfDescFile = "";
//        this->m_isLoaded = false;   
    }
}

WFStepObject *WFWorkflowManager::GetNextWFStep()
{
    if(m_workSteps.size() > 0)
        this->m_workSteps.push_back(this->getNextWorkstepDescription(this->m_workSteps[m_workSteps.size()-1]));
    else
        this->m_workSteps.push_back(this->getNextWorkstepDescription(NULL));
        
    if(m_workSteps[m_workSteps.size()-1] != NULL)
        std::cout<<m_workSteps[m_workSteps.size()-1]->GetID()<<" "<<m_workSteps[0]->GetID()<<std::endl;
        
    return this->m_workSteps[m_workSteps.size()-1];
}

WFStepObject *WFWorkflowManager::GetPreviousWFStep()
{
    this->m_workSteps.pop_back();
    if(this->m_workSteps.size() > 0)
    {
        return this->m_workSteps[this->m_workSteps.size()-1];
    }
    return NULL;
}

}//namespace

}//namespace
