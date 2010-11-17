#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLAnnotationDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationDisplayNode* vtkMRMLAnnotationDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationDisplayNode");
  if(ret)
    {
    return (vtkMRMLAnnotationDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationDisplayNode::CreateNodeInstance()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationDisplayNode");
  if(ret)
    {
    return (vtkMRMLAnnotationDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLAnnotationDisplayNode::vtkMRMLAnnotationDisplayNode()
{
  this->SetVisibility(0);
  this->SetVectorVisibility(0);
  this->SetScalarVisibility(0);
  this->SetTensorVisibility(0);
  this->Color[0]=1.0; this->Color[1]=1.0; this->Color[2]=1.0;
  this->SelectedColor[0]=1.0; this->SelectedColor[1]=0.0; this->SelectedColor[2]=0.0;
  this->SetName("");
  this->Opacity = 1.0;
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;

}

//----------------------------------------------------------------------------
vtkMRMLAnnotationDisplayNode::~vtkMRMLAnnotationDisplayNode()
{


}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayNode::WriteXML(ostream& of, int nIndent)
{
  
  Superclass::WriteXML(of, nIndent);
}



//----------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLAnnotationDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLAnnotationDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//----------------------------------------------------------------------------
// Create a backup of this node and store it with the node.
void vtkMRMLAnnotationDisplayNode::CreateBackup()
{

  if (this->IsA("vtkMRMLAnnotationTextDisplayNode"))
    {
    vtkMRMLAnnotationTextDisplayNode * backupNode = vtkMRMLAnnotationTextDisplayNode::New();

    backupNode->Copy(this);

    this->m_Backup = backupNode;
    }

  // TODO other displayNodes

}

//----------------------------------------------------------------------------
// Returns the backup of this node.
vtkMRMLAnnotationDisplayNode * vtkMRMLAnnotationDisplayNode::GetBackup()
{

  return this->m_Backup;

}






