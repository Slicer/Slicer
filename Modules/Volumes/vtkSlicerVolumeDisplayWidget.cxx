
#include "vtkSlicerApplication.h"

#include "vtkSlicerVolumeDisplayWidget.h"

// To get at the colour logic to set a default color node
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkKWApplication.h"

// KWWidgets includes
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

// VTK includes
#include "vtkObject.h"
#include "vtkObjectFactory.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumeDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerVolumeDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::vtkSlicerVolumeDisplayWidget ( )
{

    this->VolumeNode = NULL;
    this->ColorIcons = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::~vtkSlicerVolumeDisplayWidget ( )
{

  this->SetAndObserveMRMLScene ( NULL );
  vtkSetMRMLNodeMacro(this->VolumeNode, NULL);
  if ( this->ColorIcons )
    {
    this->ColorIcons->Delete();
    this->ColorIcons = NULL;
    }
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();
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
void vtkSlicerVolumeDisplayWidget::ProcessWidgetEvents(vtkObject * vtkNotUsed(caller),
                                                       unsigned long event,
                                                       void *vtkNotUsed(callData))
{
  vtkDebugMacro("ProcessWidgetEvents: nothing here, event = " << event);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessMRMLEvents(vtkObject *caller,
                                                    unsigned long event,
                                                    void *vtkNotUsed(callData) )
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
  //this->RemoveMRMLObservers();
  //this->RemoveWidgetObservers();
  //this->SetAndObserveMRMLScene(NULL);
  //this->SetMRMLScene(this->MRMLScene);
  this->SetVolumeNode(NULL);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::AddMRMLObservers ( )
{
  if (this->MRMLScene)
    {
    //this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    if (!this->MRMLScene->HasObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand))
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }

    if (!this->MRMLScene->HasObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand) )
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }   
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::RemoveMRMLObservers ( )
{
  if (this->MRMLScene)
    {
    this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::AddWidgetObservers ( ) 
{
  vtkDebugMacro("vtkSlicerVolumeDisplayWidget: adding observer");
  if (!this->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand ) )
    {
    this->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
    }
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
  this->ColorIcons = vtkSlicerColorLUTIcons::New();
}



//---------------------------------------------------------------------------
void  vtkSlicerVolumeDisplayWidget::AddColorIcons(vtkSlicerNodeSelectorWidget *selector)
{
  if ( selector == NULL ||
       selector->GetWidget() == NULL ||
       selector->GetWidget()->GetWidget() == NULL )
    {
    vtkErrorMacro ( "Got NULL ColorSelectorWidget or component widgets." );
    return;
    }

  //---
  //--- This processes only one level of menu cascade.
  //--- assume nodes or node categories are in the main
  //--- menu and nodes are in any cascade. 
  //---
  vtkKWMenuButton *mb = selector->GetWidget()->GetWidget();
  vtkKWMenu *m = mb->GetMenu();
  vtkKWMenu *c = NULL;

  std::string s;
  std::string subs;

  int num = m->GetNumberOfItems();
  //don't process commands?
  for ( int i = 0; i < num; i++)
    {
    // is item a cascade?
    c = m->GetItemCascade ( i );
    if ( c != NULL )
      {
      // if cascade, process its cascade menu items.
      int numm = c->GetNumberOfItems();
      for ( int j=0; j < numm; j++ )
        {
        subs.clear();
        if ( c->GetItemLabel(j) != NULL )
          {
          subs = c->GetItemLabel ( j );

          // insert icon in cascade menu
          // TODO: get nodeid for this menu item and pass that as well.
          c->SetItemImageToIcon ( j, this->ColorIcons->GetIconByName ( subs.c_str() ));
          c->SetItemCompoundModeToLeft ( j );          
          }
        }
      continue;
      }
    // otherwise... insert icon in main menu
    // TODO: get nodeid for this menu item and pass that as well.
    s.clear();
    if ( m->GetItemLabel ( i) != NULL )
      {
      s = m->GetItemLabel( i );
      m->SetItemImageToIcon ( i, this->ColorIcons->GetIconByName ( s.c_str() ));
      m->SetItemCompoundModeToLeft ( i );      
      }
    }  
}


