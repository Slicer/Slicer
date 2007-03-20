#include "WFXmlWorkflowManager.h"
#include "WFStepObject.h"

#include <iostream>

using namespace WFEngine::nmWFXmlWorkflowManager;
using namespace WFEngine::nmWFStepObject;

XERCES_CPP_NAMESPACE_USE;

WFXmlWorkflowManager::WFXmlWorkflowManager() : WFXmlManager()
{
    this->specification = "specification";
    this->decomposition = "decomposition";
    this->schema = "schema";
    this->processControlElements = "processControlElements";
    this->metaData = "metaData";
    
    this->m_parentWFElement = NULL;
}

WFXmlWorkflowManager::~WFXmlWorkflowManager()
{
}

WFXmlWorkflowManager *WFXmlWorkflowManager::New()
{
    return new WFXmlWorkflowManager();
}

int WFXmlWorkflowManager::loadWorkflowDescription(std::string &wfXmlFile)
{
    this->m_parentWFElement = NULL;
    
    if(this->loadXmlFile(wfXmlFile))
    {
        this->m_isLoaded = true;
        this->m_parentWFElement = this->getParentWFElement();
        return 1;
    }
    else return 0;
}

bool WFXmlWorkflowManager::IsLoaded()
{
    return this->m_isLoaded;
}

std::string WFXmlWorkflowManager::getWorkflowName()
{
    //std::cout<<"getWorkflowName from Node: ";
    if(this->m_parentWFElement)
    {
        //std::cout<<XMLString::transcode(this->m_parentWFElement->getTagName())<<std::endl;
        return XMLString::transcode(m_parentWFElement->getAttribute(XMLString::transcode("id")));
    }            
    //std::cout<<"not found!"<<std::endl;
    return "";
}

std::string WFXmlWorkflowManager::getWorkflowDescription()
{
    //std::cout<<"getWorkflowDescription from Node: ";
    DOMElement *metaDataElem = this->getMetadataWFElement();
    if(metaDataElem)
    {
        //std::cout<<XMLString::transcode(metaDataElem->getTagName())<<std::endl;
        std::string description = "description";
        DOMNodeList *metaEntries = this->getAllChildesByName(metaDataElem, description);
        if(metaEntries->getLength() > 0)
        {
            return XMLString::transcode(((DOMElement*)metaEntries->item(0))->getTextContent());
        }
    }            
    //std::cout<<"not found!"<<std::endl;
    return "";
}

void WFXmlWorkflowManager::destroy()
{
    this->m_isLoaded = false;    
    delete(this);
}

WFStepObject *WFXmlWorkflowManager::getNextWorkstepDescription(WFStepObject *curWFStep)
{
    std::string nextID;
    if(curWFStep)
    {
        nextID = curWFStep->GetNextStepID();
    }
    else
    {
        nextID = "";
    }
    
    DOMElement *curTaskElement = this->getElementFromID(nextID);
    DOMElement *curProcessElement;
    if(curTaskElement)
    {
        WFStepObject *curWS = WFStepObject::New();

        std::string ID = XMLString::transcode(curTaskElement->getAttribute(XMLString::transcode("id")));
        curWS->SetID(ID);
        
        std::string desc = this->getWorkflowDescription();
        curWS->SetWFDescription(desc);
        
        std::string name = "name";
        curProcessElement = this->getFirstChildByName(curTaskElement, name);
        if(curProcessElement)
        {
            std::string newName = XMLString::transcode(curProcessElement->getTextContent());
            curWS->SetName(newName);
            curProcessElement = NULL;
        }
        
        
        std::string flowsInto = "flowsInto";
        std::string nextElementRef = "nextElementRef";
        curProcessElement = this->getFirstChildByName(curTaskElement, flowsInto);
        if(curProcessElement)
        {
            bool found = false;
            DOMNodeList *nextStepIDsElements = this->getAllChildesByName(curProcessElement, nextElementRef);
            for(int i = 0; i < nextStepIDsElements->getLength() && !found; i++)
            {              
                DOMElement *curElem = (DOMElement*)(nextStepIDsElements->item(i));
                std::string nextStepID = XMLString::transcode(curElem->getAttribute(XMLString::transcode("id")));
                curWS->AddNextStepID(nextStepID);                                
            }    
        }
        
//        look for GUI Description
        std::string decomposesTo = "decomposesTo";
        std::string mapping = "mapping";
        DOMElement *curDecomposeToElement = this->getFirstChildByName(curTaskElement, decomposesTo);
        if(curDecomposeToElement)
        {
            const XMLCh* decompositionName = curDecomposeToElement->getAttribute(XMLString::transcode("id"));
            DOMNodeList *mappingNL = this->getAllChildesByName(curTaskElement, mapping);
            std::string expression = "expression";
            std::string mapsTo = "mapsTo";
            bool found = false;
            
            WFStepObject::variablePropertyStruct *myCurVarStruct = NULL;
            DOMElement *decompElem = NULL;
            std::set<std::string> varNameSet;
            
            //set the properties of the variable
            myCurVarStruct = this->getNextVariableFromDecomposition(decompositionName, &varNameSet, decompElem);
            while(myCurVarStruct)
            {
                //get the value of the current Variable
                bool exit = false;
                //std::cout<<mappingNL->getLength()<<std::endl;
                for(int j = 0; j < mappingNL->getLength() && !exit; j++)
                {
                    DOMElement *mapsToElement = this->getFirstChildByName((DOMElement*)(mappingNL->item(j)), mapsTo);
                    
                    if(strcmp(XMLString::transcode(mapsToElement->getTextContent()), myCurVarStruct->name.c_str()) == 0)
                    {
                        DOMElement *exprToElement = this->getFirstChildByName((DOMElement*)(mappingNL->item(j)), expression);
                        std::string tmpValue = std::string(XMLString::transcode(exprToElement->getAttribute(XMLString::transcode("query"))));
                        
                        //strip off the first and last tag because this is just meta-information of YAWL
                        if(tmpValue.find("<" + myCurVarStruct->name +">",0) != std::string::npos)
                        {
                            int firstPos = tmpValue.find(">", 0);
                            int lastPos = tmpValue.rfind("<");
                            if(firstPos != std::string::npos && lastPos != std::string::npos)
                            {
                                int length = ((tmpValue.size() - (firstPos + 1)) - (tmpValue.size() - (lastPos)));                                                               
                                myCurVarStruct->value = tmpValue.substr(firstPos + 1, length);                                
                            }
                        }                        
                        exit = true;
                    }                                        
                }        
                curWS->AddVariable(myCurVarStruct->name, myCurVarStruct);
                myCurVarStruct = this->getNextVariableFromDecomposition(decompositionName, &varNameSet, decompElem);                
            }    
        }                

//        curWS->SetVariableMapping();
        return curWS;
    }
    else
        return NULL;
}

DOMElement *WFXmlWorkflowManager::getParentWFElement()
{
    if(this->IsLoaded())
    {
        DOMNodeList *decompositionList = this->getAllChildesByName(specification, decomposition);
        if(decompositionList->getLength() > 0)
        {
            return (DOMElement*)decompositionList->item(0);
        }
    }
    return NULL;
}

DOMElement *WFXmlWorkflowManager::getMetadataWFElement()
{
    if(this->IsLoaded())
    {
        DOMNodeList *metaDataList = this->getAllChildesByName(specification, metaData);
        if(metaDataList->getLength() > 0)
        {
            return (DOMElement*)metaDataList->item(0);
        }
    }
    return NULL;
}

DOMElement *WFXmlWorkflowManager::getElementFromID(std::string &id)
{
    DOMElement *rootWFElement = getParentWFElement();
    if(!rootWFElement)
    {
        return NULL;
    }
    
    DOMElement *processWFElement = this->getFirstChildByName(rootWFElement, this->processControlElements);
    if(!processWFElement)
        return NULL;
    
    std::string task = "task";
    DOMNodeList *taskNodes = this->getAllChildesByName(processWFElement, task);
    if(taskNodes == NULL || taskNodes->getLength() == 0)
    {
        return NULL;
    }
    if(id == "")
    {
        return (DOMElement*)(taskNodes->item(0));
    }
    else
    {
        for(int i = 0; i < taskNodes->getLength(); i++)
        {
            DOMElement *curElem = (DOMElement*)taskNodes->item(i);
            if(strcmp(XMLString::transcode(curElem->getAttribute(XMLString::transcode("id"))),id.c_str()) == 0)
            {
                //std::cout<<id<<" task found"<<std::endl;
                return curElem;
            }
        }
    }
    //std::cout<<id<<" task not found"<<std::endl;
    return NULL;
}

WFStepObject::variablePropertyStruct *WFXmlWorkflowManager::getNextVariableFromDecomposition(const XMLCh *decompositionName, std::set<std::string> *varSet, DOMElement *decomElem)
{
    WFStepObject::variablePropertyStruct *tempPropStruct = NULL;
    if(!decomElem)
    {
        DOMNodeList *decompositionList = this->getAllChildesByName(specification, decomposition);
        
        bool found = false;
        for(int i = 0; i < decompositionList->getLength() && !found; i++)
        {
            DOMElement *curElem = (DOMElement*)(decompositionList->item(i));
            //std::cout<<XMLString::transcode(curElem->getAttribute(XMLString::transcode("id")))<<" == "<<XMLString::transcode(decompositionName)<<std::endl;
            if(strcmp(XMLString::transcode(curElem->getAttribute(XMLString::transcode("id"))), XMLString::transcode(decompositionName)) == 0)
            {
                decomElem = curElem;
                found = true;
            }
        }
    }
    
    std::string inputParam = "inputParam";
    std::string outputParam = "outputParam";
    std::string name = "name";
    std::string type = "type";
    
    DOMNodeList *inputNodes = this->getAllChildesByName(decomElem, inputParam);
    DOMNodeList *outputNodes = this->getAllChildesByName(decomElem, outputParam);
    
    bool exit = false;
    for(int i = 0; i < inputNodes->getLength() && !exit; i++)
    {
        DOMElement *curElem = (DOMElement*)(inputNodes->item(i));
        
        std::string varName = "";
        DOMElement* tmpElement = this->getFirstChildByName(curElem, name);
        const XMLCh* tmpCh = tmpElement->getTextContent();
        varName = std::string(XMLString::transcode(tmpCh));
        
        std::pair<std::set<std::string>::iterator,bool> success;
        // Element einfügen
        success = varSet->insert(varName);
        // Abprüfen ob Element eingefügt wurde
        if (success.second)        
        {
            //create the values for the struct
            tempPropStruct = new WFStepObject::variablePropertyStruct;
            tempPropStruct->name = varName;
            tempPropStruct->isInputParameter = true;
            tempPropStruct->isOutputParameter = false;
            tempPropStruct->type = XMLString::transcode(this->getFirstChildByName(curElem, type)->getTextContent());    
            exit = true;
        }
    }
    
    exit = false;
    for(int i = 0; i < outputNodes->getLength() && !exit; i++)
    {
        DOMElement *curElem = (DOMElement*)outputNodes->item(i);
        
        std::string varName = XMLString::transcode(this->getFirstChildByName(curElem, name)->getTextContent());
        
        if(tempPropStruct)
        {
            if(tempPropStruct->name == varName)
            {
                tempPropStruct->isOutputParameter = true;
                exit = true;
            }
        }
        else
        {
            std::pair<std::set<std::string>::iterator,bool> success;
            // Element einfügen
            success = varSet->insert(varName);
            // Abprüfen ob Element eingefügt wurde
            if (success.second)
            {
                //insert this as outparameter only
                tempPropStruct = new WFStepObject::variablePropertyStruct;
                tempPropStruct->name = varName;
                tempPropStruct->isInputParameter = false;
                tempPropStruct->isOutputParameter = true;
                tempPropStruct->type = XMLString::transcode(this->getFirstChildByName(curElem, type)->getTextContent());    
                exit = true;                
            }
        }
    }
    
    return tempPropStruct;
}

int WFXmlWorkflowManager::getNumberOfUnprocessedSteps(std::string &curStepID)
{
    int m_approxUnprocessedSteps = 0;
    std::string flowsInto = "flowsInto";
    std::string nextElementRef = "nextElementRef";
    
    DOMElement *curTaskIteratorElement = this->getElementFromID(curStepID);
    while(curTaskIteratorElement)
    {
        m_approxUnprocessedSteps++;
        DOMElement *flowsIntoElement =  this->getFirstChildByName(curTaskIteratorElement, flowsInto);
        curStepID = XMLString::transcode(this->getFirstChildByName(flowsIntoElement, nextElementRef)->getAttribute(XMLString::transcode("id")));
        if(curStepID != "")
        {
            curTaskIteratorElement = this->getElementFromID(curStepID);
        }
        else
        {
            curTaskIteratorElement = NULL;
        }
    }
    
    return m_approxUnprocessedSteps;
}

void WFXmlWorkflowManager::Destroy()
{
    delete(this);
}
