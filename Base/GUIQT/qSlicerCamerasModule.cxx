#include "qSlicerCamerasModule.h" 
#include "ui_qSlicerCamerasModule.h" 

#include "vtkMRMLViewNode.h"
#include "vtkMRMLCameraNode.h"

#include <vector>

//-----------------------------------------------------------------------------
class qSlicerCamerasModule::qInternal : public Ui::qSlicerCamerasModule
{
public:
  qInternal()
    {
    }

};

//-----------------------------------------------------------------------------
qSlicerCamerasModule::qSlicerCamerasModule(QWidget *parent) : Superclass(parent)
{
}

//-----------------------------------------------------------------------------
qSlicerCamerasModule::~qSlicerCamerasModule()
{
  if (this->initialized()) { delete this->Internal; }
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::initializer()
{
  this->Superclass::initializer();
  
  this->Internal = new qInternal;
  this->Internal->setupUi(this);

  connect(this->Internal->ViewNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onViewNodeSelected(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QString qSlicerCamerasModule::helpText()
{
  // TODO Format text properly .. see transform module for example
  //return "**Cameras Module:** Create new views and cameras. The view pulldown menu below can be used to create new views and select the active view. Switch the layout to \"Tabbed 3D Layout\" from the layout icon in the toolbar to access multiple views. The view selected in \"Tabbed 3D Layout\" becomes the active view and replaces the 3D view in all other layouts. The camera pulldown menu below can be used to set the active camera for the selected view. WARNING: this is rather experimental at the moment (fiducials, IO/data, closing the scene are probably broken for new views). ";
  QString help = 
    "To be updated %1";
    
  return help.arg(this->slicerWikiUrl()); 
}

//-----------------------------------------------------------------------------
QString qSlicerCamerasModule::aboutText()
{
  QString about = 
    "To be updated %1"; 
    
  return about.arg(this->slicerWikiUrl()); 
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::onViewNodeSelected(vtkMRMLNode* mrmlNode)
{
  vtkMRMLViewNode* selectedViewNode = vtkMRMLViewNode::SafeDownCast(mrmlNode);
  
  if (!selectedViewNode)
    {
    return;
    }

  vtkMRMLCameraNode *found_camera_node = NULL;
  std::vector<vtkMRMLNode*> snodes;
  int nnodes = this->mrmlScene()->GetNodesByClass("vtkMRMLCameraNode", snodes);
  for (int n = 0; n < nnodes; n++)
    {
    vtkMRMLCameraNode *camera_node = vtkMRMLCameraNode::SafeDownCast(snodes[n]);
    if (camera_node && 
        camera_node->GetActiveTag() && 
        !strcmp(camera_node->GetActiveTag(), selectedViewNode->GetID()))
      {
      found_camera_node = camera_node;
      break;
      }
    }
  this->Internal->CameraNodeSelector->setCurrentNode(found_camera_node);
}

