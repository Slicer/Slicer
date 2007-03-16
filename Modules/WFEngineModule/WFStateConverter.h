#ifndef WFSTATECONVERTER_H_
#define WFSTATECONVERTER_H_

#include "WFStepObject.h"

using namespace WFEngine::nmWFStepObject;

class vtkKWStateMachineState;
class vtkKWWizardStep;

class WFStateConverter
{
public:
    WFStateConverter();
    WFStateConverter(WFStepObject *wfStep);
    
//    static vtkKWStateMachineState *YAWL2KWState(WFEngine::nmWFStepObject::WFStepObject *yawlState);
//    static WFStepObject *KWState2YAWL(vtkKWStateMachineState *kwState);
    vtkKWStateMachineState *GetKWState();
    
    vtkKWWizardStep *GetKWWizardStep();
    
    void ConvertWFStep(WFStepObject *wfStep);
    
    bool IsLoaded();
    
    void Destroy();
protected:
    virtual ~WFStateConverter();
    
    vtkKWStateMachineState *createKWState(WFStepObject *wfStep);
    vtkKWWizardStep *createKWWizardStep(WFStepObject *wfStep);
private:
    bool m_isLoaded;
    vtkKWStateMachineState *m_KWState;
    vtkKWWizardStep *m_KWWizardStep;
};

#endif /*WFSTATECONVERTER_H_*/
