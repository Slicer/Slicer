#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerMRMLTreeWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformableNode.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMRMLTreeWidget );
vtkCxxRevisionMacro ( vtkSlicerMRMLTreeWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerMRMLTreeWidget::vtkSlicerMRMLTreeWidget ( )
{
  this->TreeWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerMRMLTreeWidget::~vtkSlicerMRMLTreeWidget ( )
{
  if (this->TreeWidget)
    {
    this->TreeWidget->Delete();
    this->TreeWidget = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerMRMLTreeWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
  
} 



//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdateTreeFromMRML();
    }
  
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::RemoveWidgetObservers ( ) {

}


//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // ---
  // FRAME            
  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  frame->SetLabelText ("MRML Tree");
  frame->SetDefaultLabelFontWeightToNormal( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );

  this->TreeWidget = vtkKWTree::New() ;
  this->TreeWidget->SetParent ( frame->GetFrame() );
  this->UpdateTreeFromMRML();
  this->TreeWidget->Create ( );

  this->TreeWidget->SelectionFillOn();
  this->TreeWidget->SetBalloonHelpString("MRML Tree");
  this->TreeWidget->SetBorderWidth(2);
  this->TreeWidget->SetReliefToGroove();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->TreeWidget->GetWidgetName());

  frame->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::UpdateTreeFromMRML()
  
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;
  vtkMRMLTransformableNode *transformableNode = NULL;
  vtkMRMLTransformNode *transformNode = NULL;
  
  scene->InitTraversal();
  while (node=scene->GetNextNode())
    {
    char *name = node->GetName();
    char *ID = node->GetID();
    
    this->TreeWidget->AddNode(NULL, ID, name);
    
    if (node->IsA("vtkMRMLTransformableNode") )
      {
      transformableNode = vtkMRMLTransformableNode::SafeDownCast(node);
      vtkMRMLTransformableNode *parent = transformableNode->GetParentTransformNode();
      }
    
    }
  
}




