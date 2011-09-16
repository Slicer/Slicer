#include "vtkMRMLEMSVolumeCollectionNode.h"
#include "vtkSlicerVolumesLogic.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <vtkObjectFactory.h>
#include <algorithm>
#include "vtkMRMLScalarVolumeNode.h"
#include <vtksys/stl/string>

vtkMRMLEMSVolumeCollectionNode* 
vtkMRMLEMSVolumeCollectionNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSVolumeCollectionNode");
  if(ret)
    {
    return (vtkMRMLEMSVolumeCollectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSVolumeCollectionNode;
}

vtkMRMLNode* 
vtkMRMLEMSVolumeCollectionNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSVolumeCollectionNode");
  if(ret)
    {
    return (vtkMRMLEMSVolumeCollectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSVolumeCollectionNode;
}

vtkMRMLEMSVolumeCollectionNode::vtkMRMLEMSVolumeCollectionNode()
{
  // nothing to do here
}

vtkMRMLEMSVolumeCollectionNode::~vtkMRMLEMSVolumeCollectionNode()
{
  // nothing to do here
}

void vtkMRMLEMSVolumeCollectionNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    
    if (!strcmp(key, "VolumeNodeIDs") || !strcmp(key, "NodeIDs"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      vtksys_stl::string k1;
      vtksys_stl::string k2;
      vtksys_stl::string v1;
      vtksys_stl::string v2;
      
      while (ss >> k1 && ss >> v1 && ss >> k2 && ss >> v2)
        {
        // moving to AddVolume
        //this->Scene->AddReferencedNodeID(v2.c_str(), this);
        this->AddNode(v1.c_str(), v2.c_str());
        }
      }
    }
}


void vtkMRMLEMSVolumeCollectionNode::CloneVolumes(const vtkMRMLNode *rhs, const char* addPostFix)
{
  const vtkMRMLEMSVolumeCollectionNode* node = 
    (const vtkMRMLEMSVolumeCollectionNode*) rhs;

  this->KeyToNodeIDMap  = node->KeyToNodeIDMap;
  this->NodeIDToKeyMap  = node->NodeIDToKeyMap;
  this->KeyList               = node->KeyList;

  // clone each image
  vtkSlicerVolumesLogic* volumeLogic = vtkSlicerVolumesLogic::New();
  volumeLogic->SetMRMLScene(this->GetScene());

  for (int i = 0; i < node->GetNumberOfVolumes(); ++i)
  {    
    
    vtkMRMLScalarVolumeNode* vnode =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetNthNode(i));
    if (!vnode) {
      vtkErrorMacro("Could not clone volume node bc " << i << "th node in list was null or not a volume node ");
      volumeLogic->Delete();
      return;
    }

    vtksys_stl::string cloneName(vnode->GetName());
    
    if (addPostFix) 
      {
    cloneName +=  vtksys_stl::string(addPostFix);
      }
    // If you change name later than it is not correctly shown in the volume list of the viewers  
    vtkMRMLScalarVolumeNode* clonedVolume = volumeLogic->CloneVolume(this->GetScene(),vnode, cloneName.c_str());
    this->SetNthNodeID(i, clonedVolume->GetID());
  }
  volumeLogic->Delete();
}

vtkMRMLVolumeNode*
vtkMRMLEMSVolumeCollectionNode::
GetNthVolumeNode(int n) const
{
  vtkMRMLNode* snode = this->GetNthNode(n);
  if (snode)
    {
      return vtkMRMLVolumeNode::SafeDownCast(snode);
   }
  return NULL;  
}


