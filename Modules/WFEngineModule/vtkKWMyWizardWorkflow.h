#ifndef VTKKWMYWIZARDWORKFLOW_H_
#define VTKKWMYWIZARDWORKFLOW_H_

#include <vtkKWWizardWorkflow.h>

class vtkKWWizardStep;
class vtkObject;
class vtkKWStateMachineInput;

class vtkKWMyWizardWorkflow : public vtkKWWizardWorkflow
{
public:
    static vtkKWMyWizardWorkflow *New();
    
    vtkTypeRevisionMacro(vtkKWMyWizardWorkflow,vtkKWWizardWorkflow);
    
//    vtkKWWizardWorkflow *GetWizardWorkflow();
        
    int CreateBackTransition(vtkKWWizardStep *origin, vtkKWWizardStep *destination);
    
    int CreateNextTransition(vtkKWWizardStep *origin, vtkKWStateMachineInput *next_input, vtkKWWizardStep *destination);
    //BTX
//    enum{
//        nextTransitionStartEvent = 10000,
//        nextTransitionEndEvent,
//        backTransitionStartEvent,
//        backTransitionEndEvent
//    };
    //ETX
protected:    
    vtkKWMyWizardWorkflow();
    virtual ~vtkKWMyWizardWorkflow();
    
    static void nextTransitionStartEventCB(vtkObject* obj, unsigned long,void*, void*);
    static void nextTransitionEndEventCB(vtkObject* obj, unsigned long,void*, void*);
    static void backTransitionStartEventCB(vtkObject* obj, unsigned long,void*, void*);
    static void backTransitionEndEventCB(vtkObject* obj, unsigned long,void*, void*);
};

#endif /*VTKKWMYWIZARDWORKFLOW_H_*/
