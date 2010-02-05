#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLModelMirrorNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMRMLModelMirrorNode, "$Revision: 1.0 $");


//------------------------------------------------------------------------------
vtkMRMLModelMirrorNode* vtkMRMLModelMirrorNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelMirrorNode");
  if(ret)
    {
      return (vtkMRMLModelMirrorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelMirrorNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLModelMirrorNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelMirrorNode");
  if(ret)
    {
      return (vtkMRMLModelMirrorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelMirrorNode;
}

//----------------------------------------------------------------------------
vtkMRMLModelMirrorNode::vtkMRMLModelMirrorNode()
{
  this->HideFromEditors = true;
  this->MirrorPlane = 0; //AxialMirror
  this->InputModel = NULL;
  this->OutputModel = NULL;
  this->NewModelName = NULL;
}




//----------------------------------------------------------------------------
vtkMRMLModelMirrorNode::~vtkMRMLModelMirrorNode()
{
  this->InputModel = NULL;
  this->OutputModel = NULL;
  this->SetNewModelName ( NULL );
  this->MirrorPlane = 0;
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLModelMirrorNode::Copy(vtkMRMLNode *anode)
{
  //TODO
  Superclass::Copy(anode);
  //vtkMRMLModelMirrorNode *node = (vtkMRMLModelMirrorNode *) anode;
}


//----------------------------------------------------------------------------
void vtkMRMLModelMirrorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //TODO  
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkMRMLModelMirrorNode::WriteXML(ostream& vtkNotUsed(of), int vtkNotUsed(nIndent))
{
  //TODO
}


//----------------------------------------------------------------------------
void vtkMRMLModelMirrorNode::ReadXMLAttributes(const char** vtkNotUsed(atts))
{
  //TODO
}







