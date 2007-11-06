#include "vtkSlicerLabelMapWidget.h"

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWTree.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerLabelmapTree.h"
#include "vtkSlicerLabelmapElement.h"


vtkStandardNewMacro (vtkSlicerLabelMapWidget);
//vtkCxxRevisionMacro (vtkSlicerLabelMapWidget, "$Revision: 1.0 $");
vtkSlicerLabelMapWidget::vtkSlicerLabelMapWidget(void)
{
    this->Tree=NULL;
    this->ChangeAll=NULL;
}

vtkSlicerLabelMapWidget::~vtkSlicerLabelMapWidget(void)
{
    if(this->Tree!=NULL)
    {
        this->Tree->SetParent(NULL);
        this->Tree->Delete();
        this->Tree=NULL;
    }
    if(this->ChangeAll!=NULL)
    {
        this->ChangeAll->SetParent(NULL);
        this->ChangeAll->Delete();
        this->ChangeAll=NULL;
    }
}

void vtkSlicerLabelMapWidget::CreateWidget(void)
{
    this->Superclass::CreateWidget();
    this->ChangeAll=vtkSlicerLabelmapElement::New();
    this->ChangeAll->SetParent(this);
    this->ChangeAll->Create();
    double color[3];
    color[0]=0.5;
    color[1]=0.5;
    color[2]=0.5;
    this->ChangeAll->Init("ALL COLORS",color,.2,20);
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->ChangeAll->GetWidgetName());
    this->ChangeAll->AddObserver(vtkCommand::AnyEvent,(vtkCommand *)this->GUICallbackCommand);
    this->Tree=vtkSlicerLabelmapTree::New();
    this->Tree->SetParent(this);
    this->Tree->Create();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->Tree->GetWidgetName());
    
 
}
void vtkSlicerLabelMapWidget::ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData)
{
    vtkSlicerLabelmapElement *callerLabelmap=vtkSlicerLabelmapElement::SafeDownCast(caller);
    if(callerLabelmap=this->ChangeAll)
    {
        int opacities=*((int*) callData);
        this->Tree->ChangeAllOpacities(opacities);
        vtkErrorMacro("got a labelmapwidget event");
    }
}
