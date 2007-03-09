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
            
            WFEngine::nmWFStepObject::WFStepObject *GetNextWFStep();
            WFEngine::nmWFStepObject::WFStepObject *GetPreviousWFStep();
        protected:
            WFWorkflowManager();
            virtual ~WFWorkflowManager();
            
            int LoadWorkflowFile(std::string wfDescFile);
            
        private:
//            bool m_isLoaded;
            std::string m_wfDescFile;
            
            std::vector<WFEngine::nmWFStepObject::WFStepObject*> m_workSteps;
            
        };
    }
}

#endif /*WFWORKFLOWMANAGER_H_*/
