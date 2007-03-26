#ifndef VTKWFENGINEEVENTHANDLER_H_
#define VTKWFENGINEEVENTHANDLER_H_

#include <vtkKWObject.h>

class vtkObject;
class vtkWFEngineHandler;
class vtkMRMLScene;
class vtkMRMLFiducialListNode;
class vtkKWWidget;

class vtkWFEngineEventHandler : public vtkKWObject
{
public:
    static vtkWFEngineEventHandler *New();
    
    vtkTypeRevisionMacro ( vtkWFEngineEventHandler, vtkKWObject );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    void AddWorkflowObservers(vtkWFEngineHandler *curModuleGUI);
            
    void SetEventName(const char* name);
    void SetCurrentStepID(const char* id);
       
protected:
    vtkWFEngineEventHandler();
    virtual ~vtkWFEngineEventHandler();
    
    static void ProcessWorkflowLeaveEvents(vtkObject *caller, unsigned long event, void *callData, void *clientData);
    static void ProcessWorkflowEnterEvents(vtkObject *caller, unsigned long event, void *callData, void *clientData);
    
private:
    
    const char* m_id;
    const char* m_eventName;
    
    vtkMRMLFiducialListNode *m_fiducialList;        
};

#endif /*VTKWFENGINEEVENTHANDLER_H_*/
