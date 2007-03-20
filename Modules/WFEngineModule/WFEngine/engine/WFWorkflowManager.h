#ifndef WFWORKFLOWMANAGER_H_
#define WFWORKFLOWMANAGER_H_

#include "WFXmlWorkflowManager.h"

#include <string>
#include <vector>

namespace WFEngine
{
    namespace nmWFWorkflowManager
    {
        class WFEngine::nmWFStepObject::WFStepObject;
        class WFWorkflowManager : public WFEngine::nmWFXmlWorkflowManager::WFXmlWorkflowManager
        {
        public:
            static WFWorkflowManager *New();
            
            int SetWorkflowFile(std::string wfDescFile);
            std::string GetWorkflowFile();
            
            std::string GetWorkflowName();
            
            void Close();
            
//            bool IsLoaded();
            
            // Description:
            // This function gets the next available workstep from the WorkflowXmlManager.
            // If this step was already processed then it erases all further steps after the current, because they become obsolent.
            // Returns NULL if no next workstep is available. This means we are at the finish state.
            WFEngine::nmWFStepObject::WFStepObject *GetNextWFStep();
            
            // Description:
            // This function gets the previous workstep from the internal workstep stack.
            // Returns NULL if no previous workstep is available. This means we are at the initial state.
            WFEngine::nmWFStepObject::WFStepObject *GetPreviousWFStep();                        
            
            // Description:
            // This function gets the Nth workstep from the internal workstep stack.
            // Returns NULL if not found.
            WFEngine::nmWFStepObject::WFStepObject *GetNthWFStep(int pos);
            
            // Description:
            // This function gets the workstep by the given ID from the internal workstep stack.
            // Returns NULL if not found.
            WFEngine::nmWFStepObject::WFStepObject *GetWFStepByID(std::string ID);
            
            int getNumberOfProcessedSteps();
            int getNumberOfUnprocessedSteps();
                        
            void Destroy();
        protected:
            WFWorkflowManager();
            virtual ~WFWorkflowManager();
            
            int LoadWorkflowFile(std::string wfDescFile);
            
        private:
//            bool m_isLoaded;
            std::string m_wfDescFile;
            
            std::vector<WFEngine::nmWFStepObject::WFStepObject*> m_workSteps;
            
            int m_curPos;
            
        };
    }
}

#endif /*WFWORKFLOWMANAGER_H_*/
