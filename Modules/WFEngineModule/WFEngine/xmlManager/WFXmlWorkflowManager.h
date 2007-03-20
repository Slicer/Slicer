#ifndef WFXMLWORKFLOWMANAGER_H_
#define WFXMLWORKFLOWMANAGER_H_

#include "WFXmlManager.h"
#include "WFStepObject.h"

#include <set>
using namespace WFEngine::nmWFXmlManager;

// Description:
// Basically this class handles the creation of a WFStepObject for each step in the workflow, if the step is requested.
// It is the interface class for every method which wants to get information out of the xml-workflow file

namespace WFEngine
{
    
 namespace nmWFXmlWorkflowManager
 {
     class WFEngine::nmWFStepObject::WFStepObject;
     
     class WFXmlWorkflowManager : public WFXmlManager
     {
     public:
         static WFXmlWorkflowManager* New();
         
         bool IsLoaded();         
         
         // Description:
         // Gets the number of unprocessed steps.
         int getNumberOfUnprocessedSteps(std::string &curStepID);         
         
         void Destroy();
     protected:
         WFXmlWorkflowManager();
         virtual ~WFXmlWorkflowManager();
         
         typedef WFEngine::nmWFStepObject::WFStepObject::variablePropertyStruct internalPropStruct;
         
         WFEngine::nmWFStepObject::WFStepObject *getNextWorkstepDescription(WFEngine::nmWFStepObject::WFStepObject *curWFStep);
         int loadWorkflowDescription(std::string &wfXmlFile);
         std::string getWorkflowName();
         std::string getWorkflowDescription();
         DOMElement *getParentWFElement();
         DOMElement *getMetadataWFElement();
                  
         DOMElement *getElementFromID(std::string &id);
         
         WFEngine::nmWFStepObject::WFStepObject::variablePropertyStruct *getNextVariableFromDecomposition(const XMLCh* decompositionName, std::set<std::string> *varSet, DOMElement *decomElem);                  
         
         void destroy();
     private:
         bool m_isLoaded;
         
         std::string specification;
         std::string decomposition;
         std::string schema;
         std::string processControlElements;
         std::string metaData;
         
         DOMElement *m_parentWFElement;
                  
     };
 }

}
 
#endif /*WFXMLWORKFLOWMANAGER_H_*/
