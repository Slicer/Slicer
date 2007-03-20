#include "vtkKWMyWizardWorkflow.h"

#include <vtkObjectFactory.h>

#include <vtkKWWizardStep.h>
#include <vtkKWStateMachineTransition.h>
#include <vtkKWStateMachineInput.h>

#include <vtkObject.h>
#include <vtkCallbackCommand.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyWizardWorkflow );
vtkCxxRevisionMacro(vtkKWMyWizardWorkflow, "$Revision: 1.3 $");
//----------------------------------------------------------------------------

vtkKWMyWizardWorkflow::vtkKWMyWizardWorkflow()
{
}

vtkKWMyWizardWorkflow::~vtkKWMyWizardWorkflow()
{
    this->RemoveAllObservers();       
}

vtkKWWizardWorkflow *vtkKWMyWizardWorkflow::GetWizardWorkflow()
{
    return this;    
}

int vtkKWMyWizardWorkflow::CreateBackTransition(vtkKWWizardStep *origin, vtkKWWizardStep *destination)
{
    if (!origin || !destination)
    {
    return 0;
    }

  vtkKWStateMachineTransition *transition;

  // Transition from destination back to origin

  if (!this->FindTransition(destination->GetInteractionState(),
                            origin->GetGoBackToSelfInput(),
                            origin->GetInteractionState()))
    {
    if (!this->HasInput(origin->GetGoBackToSelfInput()))
      {
      this->AddInput(origin->GetGoBackToSelfInput());
      }
    
    transition = this->CreateTransition(
      destination->GetInteractionState(),
      origin->GetGoBackToSelfInput(),
      origin->GetInteractionState());

    if (transition)
      {
        transition->SetStartCommand(destination, "HideUserInterface");
      
        std::cout<<"BackTransition Callbacks"<<std::endl;
        
//        vtkCallbackCommand *transCB = vtkCallbackCommand::New();
////        transCB->SetClientData(this);
//        transCB->SetCallback(&vtkKWMyWizardWorkflow::backTransitionStartEventCB);
//        transition->AddObserver(vtkKWStateMachineTransition::StartEvent, transCB);
//        
//        transCB->Delete();
//        transCB = NULL;
//        transCB = vtkCallbackCommand::New();
//        transCB->SetClientData(this);
//        transCB->SetCallback(&vtkKWMyWizardWorkflow::backTransitionEndEventCB);
//        transition->AddObserver(vtkKWStateMachineTransition::EndEvent, transCB);
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMyWizardWorkflow::CreateNextTransition(
  vtkKWWizardStep *origin, 
  vtkKWStateMachineInput *next_input,
  vtkKWWizardStep *destination)
{
  if (!origin || !destination)
    {
    return 0;
    }

  vtkKWStateMachineTransition *transition;

  // Transition from origin to destination

  transition = this->CreateTransition(
    origin->GetValidationState(),
    next_input,
    destination->GetInteractionState());

  if (transition)
  {
    transition->SetStartCommand(origin, "HideUserInterface");
    
    std::cout<<"NextTransition Callbacks"<<std::endl;
    
//    vtkCallbackCommand *transCB = vtkCallbackCommand::New();
////    transCB->SetClientData(this);
//    transCB->SetCallback(&vtkKWMyWizardWorkflow::nextTransitionStartEventCB);
//    transition->AddObserver(vtkKWStateMachineTransition::StartEvent, transCB);
//    
//    transCB->Delete();
//    transCB = NULL;
//    transCB = vtkCallbackCommand::New();
//    transCB->SetClientData(this);
//    transCB->SetCallback(&vtkKWMyWizardWorkflow::nextTransitionEndEventCB);
//    transition->AddObserver(vtkKWStateMachineTransition::EndEvent, transCB);
  }

  return  1;
}

void vtkKWMyWizardWorkflow::nextTransitionStartEventCB(vtkObject *obj, unsigned long int, void *callBackData, void*)
{
    std::cout<<"vtkKWMyWizardWorkflow::nextTransitionStartEventCB"<<std::endl;
//    vtkKWMyWizardWorkflow *myWizWF = (vtkKWMyWizardWorkflow*)callBackData;
//    if(myWizWF)
//    {
////        myWizWF->InvokeEvent(vtkKWMyWizardWorkflow::nextTransitionStartEvent);    
//    }
}

void vtkKWMyWizardWorkflow::nextTransitionEndEventCB(vtkObject *obj, unsigned long int, void *callBackData, void*)
{
    std::cout<<"vtkKWMyWizardWorkflow::nextTransitionEndEventCB"<<std::endl;
//    vtkKWMyWizardWorkflow *myWizWF = (vtkKWMyWizardWorkflow*)callBackData;
//    if(myWizWF)
//    {
////        myWizWF->InvokeEvent(vtkKWMyWizardWorkflow::nextTransitionEndEvent);
//    }
}

void vtkKWMyWizardWorkflow::backTransitionStartEventCB(vtkObject *obj, unsigned long int, void *callBackData, void*)
{
    std::cout<<"vtkKWMyWizardWorkflow::backTransitionStartEventCB"<<std::endl;
//    vtkKWMyWizardWorkflow *myWizWF = (vtkKWMyWizardWorkflow*)callBackData;
//    if(myWizWF)
//    {
////        myWizWF->InvokeEvent(vtkKWMyWizardWorkflow::backTransitionStartEvent);
//    }
}

void vtkKWMyWizardWorkflow::backTransitionEndEventCB(vtkObject *obj, unsigned long int, void *callBackData, void*)
{
    std::cout<<"vtkKWMyWizardWorkflow::backTransitionEndEventCB"<<std::endl;
//    vtkKWMyWizardWorkflow *myWizWF = (vtkKWMyWizardWorkflow*)callBackData;
//    if(myWizWF)
//    {
////        myWizWF->InvokeEvent(vtkKWMyWizardWorkflow::backTransitionEndEvent);
//    }
}

