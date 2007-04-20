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
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerVolumeDisplayWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

void vtkSlicerVolumeDisplayWidget::SetVolumeNode ( vtkMRMLVolumeNode *volumeNode )
{ 
  // Select this volume node
  this->VolumeNode = volumeNode; 

  // TODO: display node modified events are not being observed

  // 
  // Set the member variables and do a first process
  //
  if ( volumeNode != NULL)
    {  
    this->ProcessMRMLEvents(volumeNode, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
vtkMRMLVolumeNode * vtkSlicerVolumeDisplayWidget::GetVolumeNode ()
{ 
   vtkMRMLVolumeNode *volume = 
        vtkMRMLVolumeNode::SafeDownCast(this->VolumeNode);

   return volume;
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

}



//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{

}
//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::UpdateWidgetFromMRML ()
{

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

}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::RemoveWidgetObservers ( ) 
{

}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::CreateWidget ( )
{
  this->Superclass::CreateWidget();
}
