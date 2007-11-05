#include "vtkSlicerLabelMapWidget.h"

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWTree.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerLabelmapTree.h"


vtkStandardNewMacro (vtkSlicerLabelMapWidget);
//vtkCxxRevisionMacro (vtkSlicerLabelMapWidget, "$Revision: 1.0 $");
vtkSlicerLabelMapWidget::vtkSlicerLabelMapWidget(void)
{
    this->Tree=NULL;
}

vtkSlicerLabelMapWidget::~vtkSlicerLabelMapWidget(void)
{
}

void vtkSlicerLabelMapWidget::CreateWidget(void)
{
    this->Superclass::CreateWidget();
    this->Tree=vtkSlicerLabelmapTree::New();
    this->Tree->SetParent(this);
    this->Tree->Create();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->Tree->GetWidgetName());
    
 
}
