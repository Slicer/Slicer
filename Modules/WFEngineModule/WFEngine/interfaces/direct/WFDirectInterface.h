#ifndef WFDIRECTINTERFACE_H_
#define WFDIRECTINTERFACE_H_

#define WFDIRECTINTERFACE

#include <string>
#include <vector>
#include "WFXmlWorkflowManager.h"
#include "WFBaseEngine.h"


class WFEngine::nmWFStepObject::WFStepObject;

class WFDirectInterface : public WFEngine::WFBaseEngine
{
public:
    static WFDirectInterface *New();
    
    int InitializeWFEngine();
    
    struct workflowDesc 
    {
        std::string fileName;
        std::string workflowName;
        int createTime; 
    };
    
    std::vector<WFDirectInterface::workflowDesc*> *GetAllKnownWorkflows();
    
    int loadWorkflowFromFile(std::string fileName);
    
    int IsLoaded();
    
    void SaveState();
    void CloseWorkflowManager();
    
    WFEngine::nmWFStepObject::WFStepObject *getNextWorkStep();
    WFEngine::nmWFStepObject::WFStepObject *getBackWorkStep();
    WFEngine::nmWFStepObject::WFStepObject *getWorkStepByIndex(int index);
    
    int getNumberOfProcessedSteps();
    
    // Description:
    // Gets the number of unprocessed Steps in the workflow. This is just a rough approximation
    // because it just looks through one path through the workflow graph.
    int getNumberOfUnprocessedSteps();
    
    void Destroy();
protected:
    WFDirectInterface();
    virtual ~WFDirectInterface();
    
    int getModifiedDateFromFile(std::string fileName);
    typedef WFEngine::WFBaseEngine Superclass;
private:
    WFEngine::WFBaseEngine *m_wfBE;
    WFEngine::nmWFWorkflowManager::WFWorkflowManager *m_wfMgr;
    
    bool m_isLoaded;
};

#endif /*WFDIRECTINTERFACE_H_*/
