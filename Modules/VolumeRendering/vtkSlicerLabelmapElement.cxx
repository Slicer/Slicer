//Slicer
#include "vtkSlicerLabelmapElement.h"

//vtk
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"

//KWWidgets
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"

//Compiler
#include <sstream>
vtkCxxRevisionMacro(vtkSlicerLabelmapElement, "$Revision: 0.1 $");
vtkStandardNewMacro(vtkSlicerLabelmapElement);
vtkSlicerLabelmapElement::vtkSlicerLabelmapElement(void)
{
    this->LabelmapCallbackCommand=vtkCallbackCommand::New();
    this->LabelmapCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
    this->LabelmapCallbackCommand->SetCallback(vtkSlicerLabelmapElement::LabelmapCallback);
    this->ColorName=NULL;
    this->Color=NULL;
    for(int i=0;i<vtkSlicerLabelmapElement::NUMBER_LEVELS;i++)
    {
        this->Opacity[i]=NULL;
    }
}

vtkSlicerLabelmapElement::~vtkSlicerLabelmapElement(void)
{
    if(this->ColorName!=NULL)
    {
    this->ColorName->Delete();
    this->ColorName=NULL;
    }
    if(this->Color!=NULL)
    {
    this->Color->Delete();
    this->Color=NULL;
    }
    for(int i=0;i<vtkSlicerLabelmapElement::NUMBER_LEVELS;i++)
    {
        if(this->Opacity[i]!=NULL)
        {
            this->Opacity[i]->Delete();
            this->Opacity[i]=NULL;
        }
    }
    if(this->LabelmapCallbackCommand!=NULL)
    {
        this->LabelmapCallbackCommand->Delete();
        this->LabelmapCallbackCommand=NULL;
    }
}
void vtkSlicerLabelmapElement::CreateWidget(void)
{
    Superclass::CreateWidget();
    
    this->Color=vtkKWLabel::New();
    this->Color->SetParent(this);
    this->Color->SetText("     ");
    this->Color->Create();
    this->Script("pack %s -side left -anchor c -expand y",this->Color->GetWidgetName());

    for(int i=0;i<vtkSlicerLabelmapElement::NUMBER_LEVELS;i++)
    {
        this->Opacity[i]=vtkKWPushButton::New();
        this->Opacity[i]->SetParent(this);
        this->Opacity[i]->Create();
        std::stringstream command;
        command<<"ChangeOpacity ";
        command<<i;
        this->Opacity[i]->SetCommand(this,command.str().c_str()); 
        this->Script("pack %s -side left -anchor c -expand y",this->Opacity[i]->GetWidgetName());
    }

    this->ColorName=vtkKWLabel::New();
    this->ColorName->SetParent(this);
    this->ColorName->Create();
    this->ColorName->SetText("ColorName");
    this->Script("pack %s -side left -anchor c -expand y", this->ColorName->GetWidgetName());
}
void vtkSlicerLabelmapElement::Init(int id,std::string colorName,double color[3],int opacityLevel,int max)
{
    this->Id=id;
    std::stringstream expansion;
    expansion<<colorName;
    int sizeBefore=expansion.str().length();
    for(int i=sizeBefore;i<max;i++)
    {
        expansion<<" ";
    }

    this->ColorName->SetText(expansion.str().c_str());
    this->Color->SetBackgroundColor(color);
    this->FillButtons(opacityLevel);
}

void vtkSlicerLabelmapElement::LabelmapCallback( vtkObject *caller, unsigned long eid, void *clientData, void *callData )
{
    vtkSlicerLabelmapElement *self = reinterpret_cast<vtkSlicerLabelmapElement *>(clientData);


    if (self->GetInLabelmapCallbackFlag())
    {
#ifdef _DEBUG
        vtkDebugWithObjectMacro(self, "In vtkLabelmapCallback called recursively?");
#endif
        //return;
    }

    vtkDebugWithObjectMacro(self, "In vtkLabelmapCallback");

    self->SetInLabelmapCallbackFlag(1);
    self->ProcessLabelmapEvents(caller,eid,callData);
    self->SetInLabelmapCallbackFlag(0);
}
void vtkSlicerLabelmapElement::ProcessLabelmapEvents(vtkObject *caller, unsigned long eid, void *callData)
{
    vtkKWPushButton *callerPushButton=vtkKWPushButton::SafeDownCast(caller);
        for(int i=0;i<vtkSlicerLabelmapElement::NUMBER_LEVELS;i++)
    {
        if(callerPushButton=this->Opacity[i])
        {
            this->FillButtons(i);
            return;

        }
        this->Opacity[i]=vtkKWPushButton::New();
        this->Opacity[i]->SetParent(this);
        this->Opacity[i]->Create();
        }
}

void vtkSlicerLabelmapElement::FillButtons(int stage)
{
    for(int i=0;i<=stage;i++)
    {
        this->Opacity[i]->SetBackgroundColor(.5,.5,.5);
    }
    for(int i=stage+1;i<vtkSlicerLabelmapElement::NUMBER_LEVELS;i++)
    {
        this->Opacity[i]->SetBackgroundColor(1,1,1);
    }
}

void vtkSlicerLabelmapElement::ChangeOpacity(int stage)
{
    this->FillButtons(stage);
    int returnValue[2];
    returnValue[0]=this->Id;
    returnValue[1]=stage;
    this->InvokeEvent(vtkCommand::AnyEvent,returnValue);
}
