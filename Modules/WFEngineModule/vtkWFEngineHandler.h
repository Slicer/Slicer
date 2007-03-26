#ifndef WFENGINEHANDLER_H_
#define WFENGINEHANDLER_H_

#include <string>
#include <vector>
#include <map>

#include <vtkKWObject.h>

// TODO: is probably replaceable with tcl.h (tk.h) 
#include <vtkKWTkUtilities.h>

#include <WFDirectInterface.h>

struct Tcl_Interpr;

//BTX
class ModuleDescription;
class vtkMRMLNode;
class vtkKWFrame;
namespace WFEngine {
    namespace nmWFStepObject {
        class WFStepObject;
    }
}
//ETX

class vtkWFEngineHandler : public vtkKWObject
{
public:
    static vtkWFEngineHandler *New();
    
    int InitializeWFEngine();
    int CloseWorkflowSession();
    
    ModuleDescription *GetCurrentModuleDescription();
    
    // Description:
    // to add a new parameter to the tcl functions
    // just needed to be initialized once, is then available for validation and nextStep function
    void AddParameter(const char* name, const char* value);
    //BTX
    enum{
        ERR = -1,
        FAIL = 0,
        SUCC = 1
    };
    
    // Descritpion:
    // Event that handles all events declared in the workflow xml file    
    enum{
        WorkflowStepEnterEvent = 10000,
        WorkflowStepLeaveEvent
    };
    
    std::string GetLastError();
    
    std::map<std::string,std::string> *GetValidationErrorMap();
    
    typedef WFDirectInterface::workflowDesc workflowDesc;    
    std::vector<workflowDesc*>* GetKnownWorkflowDescriptions();
    
    int LoadBackWorkStep();
    int LoadNextWorkStep();
    int LoadWorkStepByIndex(int index);
    
    WFEngine::nmWFStepObject::WFStepObject *GetLoadedWFStep();
    void LoadNewWorkflowSession(std::string workflowFilename);
    //ETX
    
    int GetUnprocessedSteps();
    int GetProcessedSteps();
    const char* GetCurrentStepID();
    
    void SetWFMRMLNode(vtkMRMLNode *node);
    
    vtkMRMLNode *GetWFMRMLNode();
    
    void SetWizardClientArea(vtkKWFrame *clientArea);
    
    vtkKWFrame *GetWizardClientArea();        
protected:
    vtkWFEngineHandler();
    virtual ~vtkWFEngineHandler();
    
    void LoadStepValidationFunction(const char* tclFunc);
    
    void LoadNextStepFunction(const char* tclFunc);
    
    int ValidateStep();
    
    int GetNextStepID();
    
    void InvokeEnterEvents();
    
    void InvokeLeaveEvents();
    
private:
    
    bool m_initialized;
    
    bool m_validated;
    
    bool m_validationFuncLoaded;
    bool m_nextStepFuncLoaded;
    
    const char* m_nextStepFuncTCL;
    const char* m_validationFuncTCL;
    
    Tcl_Interp *m_tclInterp;
    
    ModuleDescription *m_curModuleDescription;
    vtkMRMLNode *m_curWFMRMLNode;
    vtkKWFrame *m_clientArea;
    
    //BTX
    WFDirectInterface *m_wfDI;
    std::string m_errorMSG;
    std::string m_paramNames;
    std::string m_paramValues;
    std::map<std::string, std::string> m_parameterToValueMap;
    std::map<std::string, std::string> m_parameterToErrorMap;
    WFEngine::nmWFStepObject::WFStepObject *m_curWFStepObject;
    //ETX
    
    
};

#endif /*WFENGINEHANDLER_H_*/
