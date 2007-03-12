#ifndef WFSTATECONVERTER_H_
#define WFSTATECONVERTER_H_

#include "WFStepObject.h"

using namespace WFEngine::nmWFStepObject;

class vtkKWStateMachineState;
class vtkKWStateMachineInput;
class vtkKWStateMachineTransition;
class vtkKWWizardStep;

class WFStateConverter
{
public:
    WFStateConverter();
    WFStateConverter(WFStepObject *wfStep);
    
//    static vtkKWStateMachineState *YAWL2KWState(WFEngine::nmWFStepObject::WFStepObject *yawlState);
//    static WFStepObject *KWState2YAWL(vtkKWStateMachineState *kwState);
    vtkKWStateMachineState *GetKWState();
    vtkKWStateMachineInput *GetKWInput();
    vtkKWStateMachineTransition *GetKWTransition();
    
    vtkKWWizardStep *GetKWWizardStep();
    
    void ConvertWFStep(WFStepObject *wfStep);
    
    bool IsLoaded();
protected:
    virtual ~WFStateConverter();
    
    vtkKWStateMachineState *createKWState(WFStepObject *wfStep);
    vtkKWStateMachineInput *createKWInput(WFStepObject *wfStep);
    vtkKWStateMachineInput *createKWTransition(WFStepObject *wfStep);
    vtkKWWizardStep *createKWWizardStep(WFStepObject *wfStep);
private:
    bool m_isLoaded;
    vtkKWStateMachineState *m_KWState;
    vtkKWStateMachineInput *m_KWInput;
    vtkKWStateMachineInput *m_KWTransition;
    vtkKWWizardStep *m_KWWizardStep;
};

#endif /*WFSTATECONVERTER_H_*/
