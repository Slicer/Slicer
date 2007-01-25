#include "vtkMRMLEMSTreeParametersNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>
#include <iterator>
#include "vtkMRMLEMSGlobalParametersNode.h"

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersNode* 
vtkMRMLEMSTreeParametersNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTreeParametersNode");
  if(ret)
    {
    return (vtkMRMLEMSTreeParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeParametersNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSTreeParametersNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTreeParametersNode");
  if(ret)
    {
    return (vtkMRMLEMSTreeParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeParametersNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersNode::vtkMRMLEMSTreeParametersNode()
{
  this->LeafParametersNodeID   = NULL;
  this->ParentParametersNodeID = NULL;
  this->GlobalParametersNodeID = NULL;

  this->ColorRGB[0] = 1.0;
  this->ColorRGB[1] = 0.0;
  this->ColorRGB[2] = 0.0;

  this->SpatialPriorVolumeName     = NULL;
  this->SpatialPriorWeight         = 0.0;
  this->ClassProbability           = 0.0;
  this->ExcludeFromIncompleteEStep = 0;
  this->PrintWeights               = 0;

  this->NumberOfTargetInputChannels = 0;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersNode::~vtkMRMLEMSTreeParametersNode()
{
  this->SetLeafParametersNodeID(NULL);
  this->SetParentParametersNodeID(NULL);
  this->SetGlobalParametersNodeID(NULL);
  this->SetSpatialPriorVolumeName(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "GlobalParametersNodeID=\"" 
     << (this->GlobalParametersNodeID ? this->GlobalParametersNodeID : "NULL")
     << "\" ";

  of << indent << "ParentParametersNodeID=\"" 
     << (this->ParentParametersNodeID ? this->ParentParametersNodeID : "NULL")
     << "\" ";

  of << indent << "LeafParametersNodeID=\"" 
     << (this->LeafParametersNodeID ? this->LeafParametersNodeID : "NULL")
     << "\" ";

    {
    vtksys_stl::stringstream ss;
    ss << this->ColorRGB[0] << " " 
       << this->ColorRGB[1] << " " 
       << this->ColorRGB[2];
    of << indent << "ColorRGB=\"" << ss.str() << "\" ";
    }
    
  of << indent << "InputChannelWeights=\"";
  vtksys_stl::copy(this->InputChannelWeights.begin(),
                   this->InputChannelWeights.end(),
                   vtksys_stl::ostream_iterator<double>(of, " "));
  of << "\" ";
  
  of << indent << "SpatialPriorVolumeName=\"" 
     << (this->SpatialPriorVolumeName ? this->SpatialPriorVolumeName : "")
     << "\" ";  
  
  of << indent << "SpatialPriorWeight=\"" << this->SpatialPriorWeight 
     << "\" ";
  of << indent << "ClassProbability=\"" << this->ClassProbability << "\" ";
  of << indent << "ExcludeFromIncompleteEStep=\"" 
     << this->ExcludeFromIncompleteEStep << "\" ";
  of << indent << "PrintWeights=\"" << this->PrintWeights << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->LeafParametersNodeID && !strcmp(oldID, this->LeafParametersNodeID))
    {
    this->SetLeafParametersNodeID(newID);
    }
  if (this->ParentParametersNodeID && 
      !strcmp(oldID, this->ParentParametersNodeID))
    {
    this->SetParentParametersNodeID(newID);
    }
  if (this->GlobalParametersNodeID && 
      !strcmp(oldID, this->GlobalParametersNodeID))
    {
    this->SetGlobalParametersNodeID(newID);
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTreeParametersNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->LeafParametersNodeID != NULL && 
      this->Scene->GetNodeByID(this->LeafParametersNodeID) == NULL)
    {
    this->SetLeafParametersNodeID(NULL);
    }
  if (this->ParentParametersNodeID != NULL && 
      this->Scene->GetNodeByID(this->ParentParametersNodeID) == NULL)
    {
    this->SetParentParametersNodeID(NULL);
    }
  if (this->GlobalParametersNodeID != NULL && 
      this->Scene->GetNodeByID(this->GlobalParametersNodeID) == NULL)
    {
    this->SetGlobalParametersNodeID(NULL);
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  //
  // we can assume an even number of elements
  //
  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;

    if (!strcmp(key, "GlobalParametersNodeID"))
      {
      this->SetGlobalParametersNodeID(val);
      this->Scene->AddReferencedNodeID(this->GlobalParametersNodeID, this);   
      }
    else if (!strcmp(key, "LeafParametersNodeID"))
      {
      this->SetLeafParametersNodeID(val);
      this->Scene->AddReferencedNodeID(this->LeafParametersNodeID, this);   
      }
    else if (!strcmp(key, "ParentParametersNodeID"))
      {
      this->SetParentParametersNodeID(val);
      this->Scene->AddReferencedNodeID(this->ParentParametersNodeID, this);   
      }
    else if (!strcmp(key, "ColorRGB"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      for (unsigned int i = 0; i < 3; ++i)
        {
        double d;
        if (ss >> d)
          { 
          this->ColorRGB[i] = d;
          }
        }
      }
    else if (!strcmp(key, "InputChannelWeights"))
      {
      // read data into a temporary vector
      vtksys_stl::stringstream ss;
      ss << val;
      double d;
      vtksys_stl::vector<double> tmpVec;
      while (ss >> d)
        {
        tmpVec.push_back(d);
        }

      // update number of input channels
      if (this->NumberOfTargetInputChannels != tmpVec.size())
        {
        this->SetNumberOfTargetInputChannels(tmpVec.size());
        }
      
      // copy data
      vtksys_stl::copy(tmpVec.begin(), tmpVec.end(), 
                       this->InputChannelWeights.begin());
      }
    else if (!strcmp(key, "SpatialPriorVolumeName"))
      {
      this->SetSpatialPriorVolumeName(val);
      }
    else if (!strcmp(key, "SpatialPriorWeight"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->SpatialPriorWeight;
      }
    else if (!strcmp(key, "ClassProbability"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->ClassProbability;
      }
    else if (!strcmp(key, "ExcludeFromIncompleteEStep"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->ExcludeFromIncompleteEStep;
      }
    else if (!strcmp(key, "PrintWeights"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->PrintWeights;
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSTreeParametersNode* node = 
    (vtkMRMLEMSTreeParametersNode*) rhs;

  this->NumberOfTargetInputChannels = node->NumberOfTargetInputChannels;

  this->SetLeafParametersNodeID(node->LeafParametersNodeID);
  this->SetParentParametersNodeID(node->ParentParametersNodeID);
  this->SetGlobalParametersNodeID(node->GlobalParametersNodeID);
  this->SetColorRGB(node->ColorRGB);

  this->InputChannelWeights = node->InputChannelWeights;
  
  this->SetSpatialPriorVolumeName(node->SpatialPriorVolumeName);
  this->SetSpatialPriorWeight(node->SpatialPriorWeight);

  this->SetClassProbability(node->ClassProbability);
  this->SetExcludeFromIncompleteEStep(node->ExcludeFromIncompleteEStep);
  this->SetPrintWeights(node->PrintWeights);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersNode::PrintSelf(ostream& os, 
                                             vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "LeafParametersNodeID: " 
     << (this->LeafParametersNodeID ? this->LeafParametersNodeID : "(none)")
     << "\n";
  os << indent << "ParentParametersNodeID: " 
     << (this->ParentParametersNodeID ? this->ParentParametersNodeID : "(none)")
     << "\n";
  os << indent << "GlobalParametersNodeID: " 
     << (this->GlobalParametersNodeID ?this->GlobalParametersNodeID :"(none)")
     << "\n";

  os << indent << "ColorRGB: " 
     << this->ColorRGB[0] << this->ColorRGB[1] << this->ColorRGB[2] << "\n";

  os << indent << "InputChannelWeights: ";
  vtksys_stl::copy(this->InputChannelWeights.begin(),
                   this->InputChannelWeights.end(),
                   vtksys_stl::ostream_iterator<double>(os, " "));
  os << "\n";

  os << indent << "SpatialPriorVolumeName: " 
     << (this->SpatialPriorVolumeName ? this->SpatialPriorVolumeName :"(none)")
     << "\n";  

  os << indent << "SpatialPriorWeight: " << this->SpatialPriorWeight << "\n";

  os << indent << "ClassProbability: " << this->ClassProbability << "\n";
  os << indent << "ExcludeFromIncompleteEStep: " 
     << this->ExcludeFromIncompleteEStep << "\n";
  os << indent << "PrintWeights: " << this->PrintWeights << "\n";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersNode::
SetNumberOfTargetInputChannels(unsigned int n)
{
  if (n != this->NumberOfTargetInputChannels)
    {
    this->NumberOfTargetInputChannels = n;
      
    // resize InputChannelWeights, don't preserve data!
    this->InputChannelWeights.resize(n);
    vtksys_stl::fill(this->InputChannelWeights.begin(), 
                     this->InputChannelWeights.end(), 1.0);
    }
}

//-----------------------------------------------------------------------------
double
vtkMRMLEMSTreeParametersNode::
GetInputChannelWeight(int index) const
{
  return this->InputChannelWeights[index];
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersNode::
SetInputChannelWeight(int index, double value)
{
  this->InputChannelWeights[index] = value;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersNode::
SynchronizeNumberOfTargetInputChannels()
{
  //
  // get global parameters node
  //
  vtkMRMLEMSGlobalParametersNode* node = vtkMRMLEMSGlobalParametersNode::
    SafeDownCast(this->GetScene()->GetNodeByID(this->GlobalParametersNodeID));

  int numChannels = node == NULL ? 0 : node->GetNumberOfTargetInputChannels();
  this->SetNumberOfTargetInputChannels(numChannels);
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode*
vtkMRMLEMSTreeParametersNode::
GetLeafParametersNode()
{
  vtkMRMLEMSTreeParametersLeafNode* node = NULL;
  if (this->GetScene() && this->GetLeafParametersNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->
      GetNodeByID(this->LeafParametersNodeID);
    node = vtkMRMLEMSTreeParametersLeafNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersParentNode*
vtkMRMLEMSTreeParametersNode::
GetParentParametersNode()
{
  vtkMRMLEMSTreeParametersParentNode* node = NULL;
  if (this->GetScene() && this->GetParentParametersNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->
      GetNodeByID(this->ParentParametersNodeID);
    node = vtkMRMLEMSTreeParametersParentNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSGlobalParametersNode*
vtkMRMLEMSTreeParametersNode::
GetGlobalParametersNode()
{
  vtkMRMLEMSGlobalParametersNode* node = NULL;
  if (this->GetScene() && this->GetGlobalParametersNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->
      GetNodeByID(this->GlobalParametersNodeID);
    node = vtkMRMLEMSGlobalParametersNode::SafeDownCast(snode);
    }
  return node;
}
