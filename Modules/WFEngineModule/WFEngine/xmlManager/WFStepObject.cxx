#include "WFStepObject.h"
#include <iostream>
using namespace WFEngine::nmWFStepObject;

WFStepObject::WFStepObject()
{
    this->m_ID = "";
    this->m_name = "";
    this->m_wfDesc = "";    
}

WFStepObject::~WFStepObject()
{
    this->m_nextSteps.clear();
    this->m_varMap.clear();
    this->m_ID = "";
    this->m_name = "";
    this->m_wfDesc = "";
}

WFStepObject *WFStepObject::New()
{
    return new WFStepObject();
}

void WFStepObject::Destroy()
{
    delete(this);
}

void WFStepObject::SetID(std::string &ID)
{
    this->m_ID = ID;
}

void WFStepObject::SetName(std::string &name)
{
    this->m_name = name;
}

void WFStepObject::AddNextStepID(std::string &nextStepID)
{
    this->m_nextSteps.push_back(nextStepID);
}

std::string WFStepObject::GetID()
{
    return this->m_ID;
}

std::string WFStepObject::GetName()
{
    return this->m_name;
}

std::string WFStepObject::GetNextStepID()
{
    if(this->m_nextSteps.size() > 0)
        return this->m_nextSteps[0];
    else
        return "";           
}

std::string WFStepObject::GetStepDescription()
{
    std::string description = "description";
    variablePropertyStruct *tempPropStruct = this->GetVariableStructByName(description);
    
    if(tempPropStruct)
    {
        return tempPropStruct->value;
    }
    return "";
}

std::string WFStepObject::GetWFDescription()
{
    return this->m_wfDesc;
}

void WFStepObject::SetWFDescription(std::string &wfDesc)
{
    this->m_wfDesc = wfDesc;
}

std::string WFStepObject::GetGUIDescription()
{
    std::string gui = "gui";
    variablePropertyStruct *tempPropStruct = this->GetVariableStructByName(gui);
    
    if(tempPropStruct)
    {   
        std::string stepGUIDesc = tempPropStruct->value;

        if (strcmp(stepGUIDesc.c_str(), "")!= 0) {
            if (strcmp(stepGUIDesc.substr(0,5).c_str(), "<?xml")!= 0) {
                std::string
                        xmlHeader = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
                xmlHeader.append(stepGUIDesc);
                stepGUIDesc = xmlHeader;
            }
            return stepGUIDesc;
        }
    }
    return "";
}

void WFStepObject::AddVariable(std::string &varName, variablePropertyStruct *propStruct)
{
    this->m_varMap.insert(std::make_pair(varName, propStruct));
}

bool WFStepObject::ExistsEvent(std::string &eventName)
{
    std::string curEventName = "event_";
    curEventName.append(eventName);
    
    variablePropertyStruct *tempPropStruct = this->GetVariableStructByName(eventName);
    if(tempPropStruct)
    {
        return true;
    }
    return false;
}

std::vector<WFStepObject::variablePropertyStruct*>* WFStepObject::GetAllEvents()
{
    std::vector<WFStepObject::variablePropertyStruct*> *stepEvents = new std::vector<WFStepObject::variablePropertyStruct*>;
    
    std::map<std::string, WFStepObject::variablePropertyStruct*>::iterator mIter;
    for(mIter = this->m_varMap.begin(); mIter != this->m_varMap.end(); mIter++)
    {        
        if(std::strcmp((*mIter).first.substr(0,5).c_str(), "event") == 0)
        {
            //std::cout<<(*mIter).first.substr(6,(*mIter).first.size()-1)<<std::endl;
            stepEvents->push_back((*mIter).second);
        }
    }
    return stepEvents;
}

WFStepObject::variablePropertyStruct* WFStepObject::GetVariableStructByName(std::string &variableName)
{
    std::map<std::string, variablePropertyStruct*>::iterator curIter = this->m_varMap.find(variableName);
    if(curIter != this->m_varMap.end())
    {
        return curIter->second;
    }
    else
    {
        return NULL;
    }
}

std::string WFStepObject::GetTCLNextWorkstepFunction()
{
    std::string nextWorkstepVarName = "tclNextStepFunc";
    
    variablePropertyStruct *varStruct = this->GetVariableStructByName(nextWorkstepVarName);
    if(varStruct)
        return varStruct->value;
    else
        return "";            
}

std::string WFStepObject::GetTCLValidationFunction()
{
    std::string validationVarName = "tclValidationFunc";
    
    variablePropertyStruct *varStruct = this->GetVariableStructByName(validationVarName);
    if(varStruct)
        return varStruct->value;
    else
        return "";
}
