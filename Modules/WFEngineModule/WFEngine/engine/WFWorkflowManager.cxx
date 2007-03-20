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
    
    this->m_curPos = -1;
}

WFWorkflowManager::~WFWorkflowManager()
{
    this->m_workSteps.clear();        
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
    this->m_workSteps.clear();  
    this->m_curPos = -1;
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
        this->m_curPos = -1;
//        this->m_isLoaded = false;   
    }
}

WFStepObject *WFWorkflowManager::GetNextWFStep()
{
    if(m_workSteps.size() > this->m_curPos + 1)
    {
        this->m_workSteps.erase(this->m_workSteps.begin() + this->m_curPos + 1, this->m_workSteps.end());
    }
    
    if(m_workSteps.size() > 0)
        this->m_workSteps.push_back(this->getNextWorkstepDescription(this->m_workSteps[this->m_curPos]));
    else
        this->m_workSteps.push_back(this->getNextWorkstepDescription(NULL));
        
//    if(m_workSteps[m_workSteps.size()-1] != NULL)
        //std::cout<<"GetNextWFStep: "<<m_workSteps[m_workSteps.size()-1]->GetID()<<" "<<m_workSteps[0]->GetID()<<std::endl;
        
    this->m_curPos = m_workSteps.size()-1;
    return this->m_workSteps[this->m_curPos];
}

WFStepObject *WFWorkflowManager::GetPreviousWFStep()
{
    if(this->m_curPos > 0)
    {
        this->m_curPos = this->m_curPos - 1;
        return this->m_workSteps[this->m_curPos];
    }
    this->m_curPos = -1;
    return NULL;
}

WFStepObject *WFWorkflowManager::GetNthWFStep(int pos)
{
    if(this->m_workSteps.size() > pos && pos >= 0)
    {
        this->m_curPos = pos;
        return this->m_workSteps[this->m_curPos];
    }
    this->m_curPos = -1;
    return NULL;
}

WFStepObject *WFWorkflowManager::GetWFStepByID(std::string ID)
{   
    for(int i = 0; i < this->m_workSteps.size(); i++)
    {
        if(this->m_workSteps[i]->GetID() == ID)
        {
            this->m_curPos = i;
            return this->m_workSteps[i];
        }
    }
    return NULL;
}

int WFWorkflowManager::getNumberOfProcessedSteps()
{
    //std::cout<<"getNumberOfProcessedSteps: "<<m_workSteps.size()<<std::endl;
//    return this->m_curPos + 1;
    return this->m_workSteps.size();
}
int WFWorkflowManager::getNumberOfUnprocessedSteps()
{
    std::string ID = "";
    if(this->m_workSteps.size() > 0)
    {
        WFStepObject *curObj = this->m_workSteps[this->m_workSteps.size() - 1];
        if(curObj)
        {
            ID = curObj->GetID();
        }
        else
        {
            return 0;
        }       
    }        
    return WFXmlWorkflowManager::getNumberOfUnprocessedSteps(ID);
}

void WFWorkflowManager::Destroy()
{
    delete(this);
}

}//namespace

}//namespace
