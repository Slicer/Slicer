#ifndef WFSTEPOBJECT_H_
#define WFSTEPOBJECT_H_

#include <string>
#include <map>
#include <vector>

namespace WFEngine
{
    namespace nmWFStepObject
    {
        class WFStepObject
        {
        public:
            static WFStepObject* New();
            
            struct variablePropertyStruct {
                bool isInputParameter;
                bool isOutputParameter;
                std::string value;
                std::string name;
                std::string type;
                std::string mapsTo;
                std::string decomposesTo;
            };
            
            void SetID(std::string &ID);
            void SetName(std::string &name);
            void AddNextStepID(std::string &nextStepID);
//            void SetVariableMapping(std::string &from, std::string &to);
            void SetWFDescription(std::string &wfDesc);            
            void AddVariable(std::string &varName, variablePropertyStruct *propStruct);
            
            std::string GetNextStepID();
            std::string GetID();
            std::string GetName();
            std::string GetWFDescription();
            std::string GetStepDescription();
            std::string GetTCLValidationFunction();
            std::string GetTCLNextWorkstepFunction();
            std::string GetGUIDescription();
            variablePropertyStruct *GetVariableStructByName(std::string &variableName);
            bool ExistsEvent(std::string &eventName);
            
            std::vector<variablePropertyStruct*>* GetAllEvents();
            
            void Destroy();
            
        protected:
            WFStepObject();
            virtual ~WFStepObject();
        private:
            std::string m_ID;
            std::string m_name;
            std::string m_wfDesc;
            std::vector<std::string> m_nextSteps;
//            std::map<std::string, std::string> m_varMapping;
            std::map<std::string, variablePropertyStruct*> m_varMap;                        
        };
    }
}

#endif /*WFSTEPOBJECT_H_*/
