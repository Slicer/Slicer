#include "vtkSlicerBaseTree.h"
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
vtkCxxRevisionMacro(vtkSlicerBaseTree, "$Revision: 0.1 $");
vtkStandardNewMacro(vtkSlicerBaseTree);
vtkSlicerBaseTree::vtkSlicerBaseTree(void)
{
    this->BaseTreeCallbackCommand=vtkCallbackCommand::New();
    this->BaseTreeCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
    this->BaseTreeCallbackCommand->SetCallback(vtkSlicerBaseTree::BaseTreeCallback);
}

vtkSlicerBaseTree::~vtkSlicerBaseTree(void)
{
    if(this->BaseTreeCallbackCommand!=NULL)
    {
        this->BaseTreeCallbackCommand->Delete();
        this->BaseTreeCallbackCommand=NULL;
    }
}
void vtkSlicerBaseTree::CreateWidget(void)
{
    Superclass::CreateWidget();
}
void vtkSlicerBaseTree::BaseTreeCallback( vtkObject *caller, unsigned long eid, void *clientData, void *callData )
{
    vtkSlicerBaseTree *self = reinterpret_cast<vtkSlicerBaseTree *>(clientData);


    if (self->GetInBaseTreeCallbackFlag())
    {
        #ifdef _DEBUG
                vtkDebugWithObjectMacro(self, "In vtkLabelmapCallback called recursively?");
        #endif
    }

    self->SetInBaseTreeCallbackFlag(1);
    self->ProcessBaseTreeEvents(caller,eid,callData);
    self->SetInBaseTreeCallbackFlag(0);
}
void vtkSlicerBaseTree::ProcessBaseTreeEvents(vtkObject *caller,unsigned long eid,void *callData)
{
    vtkErrorMacro("Overwrite this method in the subclass");
}
