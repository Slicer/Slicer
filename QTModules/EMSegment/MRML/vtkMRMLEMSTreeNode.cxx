#include "vtkMRMLEMSTreeNode.h"
#include "vtkMRMLScene.h"
#include <sstream>
#include <vtkObjectFactory.h>
#include <iterator>

#include <vtksys/ios/sstream>
#include "vtkMRMLEMSTreeParametersNode.h"
#include "vtkMRMLEMSTreeParametersParentNode.h"
#include "vtkMRMLEMSTreeParametersLeafNode.h"

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode* 
vtkMRMLEMSTreeNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTreeNode");
  if(ret)
    {
    return (vtkMRMLEMSTreeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSTreeNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTreeNode");
  if(ret)
    {
    return (vtkMRMLEMSTreeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode::vtkMRMLEMSTreeNode()
{
  this->ParentNodeID           = NULL;
  this->TreeParametersNodeID   = NULL;

  this->LeafParametersNodeID   = NULL;
  this->ParentParametersNodeID = NULL;

  this->ColorRGB[0] = 1.0;
  this->ColorRGB[1] = 0.0;
  this->ColorRGB[2] = 0.0;

  this->SpatialPriorVolumeName     = NULL;
  this->SpatialPriorWeight         = 1.0;
  this->ClassProbability           = 0.0;
  this->ExcludeFromIncompleteEStep = 0;
  this->PrintWeights               = 0;

  this->NumberOfTargetInputChannels = 0;

}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode::~vtkMRMLEMSTreeNode()
{
  this->SetParentNodeID(NULL);
  this->SetTreeParametersNodeID(NULL);
  // !!! ??? child nodes? unreference???

  this->SetLeafParametersNodeID(NULL);
  this->SetParentParametersNodeID(NULL);
  this->SetSpatialPriorVolumeName(NULL);

}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " ParentNodeID=\"" 
     << (this->ParentNodeID ? this->ParentNodeID : "NULL") << "\" ";
  // Only for legacy purposes 
    of << indent << "TreeParametersNodeID=\"" 
     << (this->TreeParametersNodeID ? this->TreeParametersNodeID : "NULL")
      << "\" ";
  of << indent << "ChildNodeIDs=\"";
  for (unsigned int i = 0; i < this->ChildNodeIDs.size(); ++i)
    {
    of << this->ChildNodeIDs[i];
    if (i < this->ChildNodeIDs.size() - 1)
      {
      of << " ";
      }
    }
  of << "\" ";
 of << indent << " ParentParametersNodeID=\"" 
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
vtkMRMLEMSTreeNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->ParentNodeID && !strcmp(oldID, this->ParentNodeID))
    {
    this->SetParentNodeID(newID);
    }
  if (this->TreeParametersNodeID && !strcmp(oldID, this->TreeParametersNodeID))
    {
    this->SetTreeParametersNodeID(newID);
    }
  for (unsigned int i = 0; i < this->ChildNodeIDs.size(); ++i)
    {
    if (oldID && newID && (this->ChildNodeIDs[i] == std::string(oldID)))
      {
      this->ChildNodeIDs[i] = newID;
      }
    }

  if (this->LeafParametersNodeID && !strcmp(oldID, this->LeafParametersNodeID))
    {
    this->SetLeafParametersNodeID(newID);
    }
  if (this->ParentParametersNodeID && 
      !strcmp(oldID, this->ParentParametersNodeID))
    {
    this->SetParentParametersNodeID(newID);
    }

}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTreeNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->ParentNodeID != NULL && 
      this->Scene->GetNodeByID(this->ParentNodeID) == NULL)
    {
    this->SetParentNodeID(NULL);
    }
  if (this->TreeParametersNodeID != NULL && 
      this->Scene->GetNodeByID(this->TreeParametersNodeID) == NULL)
    {
    this->SetTreeParametersNodeID(NULL);
    }

  std::vector<std::string> childIDsCopy(this->ChildNodeIDs);
  this->ChildNodeIDs.clear();
  for (unsigned int i = 0; i < childIDsCopy.size(); ++i)
    {
    if (childIDsCopy[i].empty() ||
        this->Scene->GetNodeByID(childIDsCopy[i].c_str()) != NULL)
      {
      this->ChildNodeIDs.push_back(childIDsCopy[i]);
      }
    }

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
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  //
  // we assume an even number of elements
  //
  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    
    if (!strcmp(key, "ParentNodeID"))
      {
      this->SetParentNodeID(val);
      }
    else if (!strcmp(key, "TreeParametersNodeID"))
      {
      this->SetTreeParametersNodeID(val);
      }
    else if (!strcmp(key, "ChildNodeIDs"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      vtksys_stl::string currentID;
      while (ss >> currentID)
        {
        this->AddChildNode(currentID.c_str());
        }
      }
   if (!strcmp(key, "LeafParametersNodeID"))
      {
      this->SetLeafParametersNodeID(val);
      //this->Scene->AddReferencedNodeID(this->LeafParametersNodeID, this);   
      }
    else if (!strcmp(key, "ParentParametersNodeID"))
      {
      this->SetParentParametersNodeID(val);
      //this->Scene->AddReferencedNodeID(this->ParentParametersNodeID, this);   
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
      this->SetNumberOfTargetInputChannels((int)tmpVec.size());
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
void vtkMRMLEMSTreeNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSTreeNode* node = (vtkMRMLEMSTreeNode*) rhs;

  this->SetParentNodeID(node->ParentNodeID);
  this->SetTreeParametersNodeID(node->TreeParametersNodeID);
  this->ChildNodeIDs = node->ChildNodeIDs;

  // !!! is it correct to add references here???
  for (unsigned int i = 0; i < this->ChildNodeIDs.size(); ++i)
    {
    this->Scene->AddReferencedNodeID(this->ChildNodeIDs[i].c_str(), this);
    }

    this->NumberOfTargetInputChannels = node->NumberOfTargetInputChannels;

  this->SetLeafParametersNodeID(node->LeafParametersNodeID);
  this->SetParentParametersNodeID(node->ParentParametersNodeID);
  this->SetColorRGB(node->ColorRGB);

  this->InputChannelWeights = node->InputChannelWeights;
  
  this->SetSpatialPriorVolumeName(node->SpatialPriorVolumeName);
  this->SetSpatialPriorWeight(node->SpatialPriorWeight);

  this->SetClassProbability(node->ClassProbability);
  this->SetExcludeFromIncompleteEStep(node->ExcludeFromIncompleteEStep);
  this->SetPrintWeights(node->PrintWeights);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "ParentNodeID: " <<
    (this->ParentNodeID ? this->ParentNodeID : "(none)") << "\n";
  
  os << indent << "TreeParametersNodeID: " <<
    (this->TreeParametersNodeID ? this->TreeParametersNodeID : "(none)")
     << "\n";

  os << indent << "ChildNodeIDs: " << "\n";
  for (unsigned int i = 0; i < this->ChildNodeIDs.size(); ++i)
    {
    std::string mrmlID = this->ChildNodeIDs[i];
    os << indent << "  " << mrmlID << "\n";
    }

   os << indent << "LeafParametersNodeID: " 
     << (this->LeafParametersNodeID ? this->LeafParametersNodeID : "(none)")
     << "\n";
  os << indent << "ParentParametersNodeID: " 
     << (this->ParentParametersNodeID ? this->ParentParametersNodeID : "(none)")
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
int
vtkMRMLEMSTreeNode::
GetNumberOfChildNodes()
{
  return (int)this->ChildNodeIDs.size();
}

//-----------------------------------------------------------------------------
const char*
vtkMRMLEMSTreeNode::
GetNthChildNodeID(int n)
{
  if (n >= 0 && n < static_cast<int>(this->ChildNodeIDs.size()))
    {
    return this->ChildNodeIDs[n].c_str();
    }
  else
    {
    return NULL;
    }
}

//-----------------------------------------------------------------------------
int
vtkMRMLEMSTreeNode::
GetChildIndexByMRMLID(const char* childID)
{
  std::string searchID(childID);
  for (int i = 0; i < static_cast<int>(this->ChildNodeIDs.size()); ++i)
    {
    if (this->ChildNodeIDs[i] == searchID)
      {
      return i;
      }
    }
  // didn't find it
  return -1;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkMRMLEMSTreeNode::
GetParentNode()
{
  vtkMRMLEMSTreeNode* node = NULL;
  if (this->GetScene() && this->GetParentNodeID())
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ParentNodeID);
    node = vtkMRMLEMSTreeNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkMRMLEMSTreeNode::
GetNthChildNode(int n)
{
  vtkMRMLEMSTreeNode* node = NULL;
  if (this->GetScene() && this->GetNthChildNodeID(n))
    {
    vtkMRMLNode* snode = this->GetScene()->
      GetNodeByID(this->GetNthChildNodeID(n));
    node = vtkMRMLEMSTreeNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
MoveNthChildNode(int fromIndex, int toIndex)
{
  std::string movingParam = this->ChildNodeIDs[fromIndex];
  this->ChildNodeIDs.erase(this->ChildNodeIDs.begin() + 
                                  fromIndex);
  this->ChildNodeIDs.insert(this->ChildNodeIDs.begin() + 
                                   toIndex, movingParam);
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
RemoveNthChildNode(int n)
{
  std::string removedChildNodeID = this->ChildNodeIDs[n];
  this->ChildNodeIDs.erase(ChildNodeIDs.begin() + n);
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
AddChildNode(const char* mrmlID)
{
  this->ChildNodeIDs.push_back(mrmlID);
  this->Scene->AddReferencedNodeID(mrmlID, this);  
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
SetNthChildNode(int n, const char* mrmlID)
{
  this->ChildNodeIDs[n] = mrmlID;
  this->Scene->AddReferencedNodeID(mrmlID, this);  
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
SetNumberOfTargetInputChannels(unsigned int n)
{
  if (n != this->NumberOfTargetInputChannels)
    {
    this->NumberOfTargetInputChannels = n;
      
    // resize InputChannelWeights, don't preserve data!
    this->InputChannelWeights.resize(n);
    vtksys_stl::fill(this->InputChannelWeights.begin(), 
                     this->InputChannelWeights.end(), 1.0);

    if (this->GetLeafParametersNode() != NULL)
      {
      this->GetLeafParametersNode()->SetNumberOfTargetInputChannels(n);
      }
    if (this->GetParentParametersNode() != NULL)
      {
      this->GetParentParametersNode()->SetNumberOfTargetInputChannels(n);
      }  
    }
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
AddTargetInputChannel()
{
  ++this->NumberOfTargetInputChannels;
  this->InputChannelWeights.push_back(1.0);

  if (this->GetLeafParametersNode() != NULL)
    {
    this->GetLeafParametersNode()->AddTargetInputChannel();
    }
  if (this->GetParentParametersNode() != NULL)
    {
    this->GetParentParametersNode()->AddTargetInputChannel();
    }  
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
RemoveNthTargetInputChannel(int index)
{
  --this->NumberOfTargetInputChannels;
  this->InputChannelWeights.erase(InputChannelWeights.begin() + index);

  if (this->GetLeafParametersNode() != NULL)
    {
    this->GetLeafParametersNode()->RemoveNthTargetInputChannel(index);
    }
  if (this->GetParentParametersNode() != NULL)
    {
    this->GetParentParametersNode()->RemoveNthTargetInputChannel(index);
    }  
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
MoveNthTargetInputChannel(int fromIndex, int toIndex)
{
  double movingParam = this->InputChannelWeights[fromIndex];
  this->InputChannelWeights.erase(this->InputChannelWeights.begin() + 
                                  fromIndex);
  this->InputChannelWeights.insert(this->InputChannelWeights.begin() + 
                                   toIndex, movingParam);

  if (this->GetLeafParametersNode() != NULL)
    {
    this->GetLeafParametersNode()->MoveNthTargetInputChannel(fromIndex, toIndex);
    }
  if (this->GetParentParametersNode() != NULL)
    {
    this->GetParentParametersNode()->MoveNthTargetInputChannel(fromIndex, toIndex);
    }  
}
//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode*
vtkMRMLEMSTreeNode::
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
vtkMRMLEMSTreeNode::
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
double
vtkMRMLEMSTreeNode::
GetInputChannelWeight(int index) const
{
  return this->InputChannelWeights[index];
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
SetInputChannelWeight(int index, double value)
{
  this->InputChannelWeights[index] = value;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersNode*
vtkMRMLEMSTreeNode::
GetTreeParametersNode()
{
  vtkMRMLEMSTreeParametersNode* node = NULL;
  if (this->GetScene() && this->GetTreeParametersNodeID())
    {
    vtkMRMLNode* snode = this->GetScene()->
      GetNodeByID(this->TreeParametersNodeID);
    node = vtkMRMLEMSTreeParametersNode::SafeDownCast(snode);
    }
  return node;
}
