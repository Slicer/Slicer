#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerVolumeDisplayWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

// to get at the colour logic to set a default color node
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLScalarVolumeNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumeDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerVolumeDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::vtkSlicerVolumeDisplayWidget ( )
{

    this->VolumeNode = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::~vtkSlicerVolumeDisplayWidget ( )
{

  this->SetMRMLScene ( NULL );
  vtkSetMRMLNodeMacro(this->VolumeNode, NULL);

}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerVolumeDisplayWidget: " << this->GetClassName ( ) << "\n";

    if (this->VolumeNode)
      {
      os << indent << "VolumeNode:" << "\n";
      this->VolumeNode->PrintSelf(os, indent.GetNextIndent());
      }
    // print widgets?
}

//---------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode * vtkSlicerVolumeDisplayWidget::GetVolumeDisplayNode ()
{ 
   vtkMRMLVolumeDisplayNode *display = NULL;
   vtkMRMLVolumeNode *volume = this->GetVolumeNode();
   if (volume != NULL)
      {
      display = vtkMRMLVolumeDisplayNode::SafeDownCast(volume->GetDisplayNode());
      }
   return display;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkDebugMacro("ProcessWidgetEvents: nothing here, event = " << event);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{

  vtkDebugMacro("ProcessMRMLEvents: event = " << event);
  // when the volume node changes, just get a modified on this
  if (vtkSlicerVolumeDisplayWidget::SafeDownCast(caller) != NULL &&
      event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("ProcessMRMLEvents: got modified event, hopefully from my volume node, calling myupdate widget from mrml");
    // check that the display node is correct
    this->UpdateWidgetFromMRML();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::UpdateWidgetFromMRML ()
{
  vtkDebugMacro("UpdateWidgetFromMRML: subclass has not implemented this method!");
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::TearDownWidget ()
{
  this->RemoveWidgetObservers();
  this->SetMRMLScene(NULL);
  this->SetAndObserveMRMLScene(NULL);
  this->SetVolumeNode(NULL);
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::AddWidgetObservers ( ) 
{
  vtkDebugMacro("vtkSlicerVolumeDisplayWidget: adding observer");
  this->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::RemoveWidgetObservers ( ) 
{
  this->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::CreateWidget ( )
{
  this->Superclass::CreateWidget();
}
