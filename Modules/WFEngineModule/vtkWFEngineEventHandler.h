#ifndef VTKWFENGINEEVENTHANDLER_H_
#define VTKWFENGINEEVENTHANDLER_H_

#include <vtkKWObject.h>

class vtkObject;
class vtkCallbackCommand;
class vtkWFEngineHandler;
class vtkMRMLScene;

class vtkWFEngineEventHandler : public vtkKWObject
{
public:
    static vtkWFEngineEventHandler *New();
    
    void AddWorkflowObservers(vtkWFEngineHandler *curModuleGUI);
        
    void SetMRMLScene(vtkMRMLScene *scene);
    
    vtkMRMLScene *GetMRMLScene();
    
    void SetEventName(const char* name);
    void SetCurrentStepID(const char* id);
protected:
    vtkWFEngineEventHandler();
    virtual ~vtkWFEngineEventHandler();
    
    static void ProcessWorkflowLeaveEvents(vtkObject *caller, unsigned long event, void *callData, void *clientData);
    
private:
    
    vtkCallbackCommand *m_workflowCB;
    
    vtkMRMLScene *m_mrmlScene;
    
    const char* m_id;
    const char* m_eventName;
};

#endif /*VTKWFENGINEEVENTHANDLER_H_*/
