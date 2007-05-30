#include "vtkMRMLEMSTargetNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>
#include <iterator>

vtkMRMLEMSTargetNode* 
vtkMRMLEMSTargetNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTargetNode");
  if(ret)
    {
    return (vtkMRMLEMSTargetNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTargetNode;
}

vtkMRMLNode* 
vtkMRMLEMSTargetNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTargetNode");
  if(ret)
    {
    return (vtkMRMLEMSTargetNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTargetNode;
}

vtkMRMLEMSTargetNode::vtkMRMLEMSTargetNode()
{
  // nothing to do here
}

vtkMRMLEMSTargetNode::~vtkMRMLEMSTargetNode()
{
  // nothing to do here
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTargetNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  for (IntensityNormalizationParameterListIterator i = 
         this->IntensityNormalizationParameterList.begin(); 
       i != this->IntensityNormalizationParameterList.end(); ++i)
    {
    if (oldID && newID && *i == vtksys_stl::string(oldID))
      {
      *i = newID;
      }
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTargetNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  for (IntensityNormalizationParameterListIterator i = 
         this->IntensityNormalizationParameterList.begin(); 
       i != this->IntensityNormalizationParameterList.end(); ++i)
    {
    if (this->Scene->GetNodeByID((*i).c_str()) == NULL)
      {
      *i = "None";
      }
    }
}

void vtkMRMLEMSTargetNode::WriteXML(ostream& of, int indent)
{
  Superclass::WriteXML(of, indent);

  of << indent << "IntensityNormalizationParameterNodeIDs=\"";
  vtksys_stl::copy(this->IntensityNormalizationParameterList.begin(),
                   this->IntensityNormalizationParameterList.end(),
                   vtksys_stl::ostream_iterator<vtksys_stl::string>(of, " "));
  of << "\" ";
}

void vtkMRMLEMSTargetNode::ReadXMLAttributes(const char** attrs)
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

    if (!strcmp(key, "IntensityNormalizationParameterNodeIDs"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      vtksys_stl::string s;
      while (ss >> s)
        {
        this->IntensityNormalizationParameterList.push_back(s);
        this->Scene->AddReferencedNodeID(s.c_str(), this);
        }
      }
    }
}

void vtkMRMLEMSTargetNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);

  vtkMRMLEMSTargetNode* node = 
    (vtkMRMLEMSTargetNode*) rhs;

  this->IntensityNormalizationParameterList = 
    node->IntensityNormalizationParameterList;
}

void vtkMRMLEMSTargetNode::PrintSelf(ostream& os, 
                                     vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "IntensityNormalizationParameterNodeIDs: ";
  vtksys_stl::copy(this->IntensityNormalizationParameterList.begin(),
                   this->IntensityNormalizationParameterList.end(),
                   vtksys_stl::ostream_iterator<vtksys_stl::string>(os, " "));
  os << "\n";
}

void
vtkMRMLEMSTargetNode::
AddVolume(const char* key, const char* volumeNodeID)
{
  Superclass::AddVolume(key, volumeNodeID);
  this->IntensityNormalizationParameterList.push_back("None");
}

void
vtkMRMLEMSTargetNode::
RemoveAllVolumes()
{
  Superclass::RemoveAllVolumes();
  this->IntensityNormalizationParameterList.clear();
}

void
vtkMRMLEMSTargetNode::
RemoveVolumeByKey(const char* key)
{
  int index = this->GetIndexByKey(key);
  Superclass::RemoveVolumeByKey(key);
  
  this->IntensityNormalizationParameterList.
    erase(this->IntensityNormalizationParameterList.begin() + index);
}

void
vtkMRMLEMSTargetNode::
RemoveVolumeByNodeID(const char* nodeID)
{
  int index = this->GetIndexByVolumeNodeID(nodeID);
  Superclass::RemoveVolumeByNodeID(nodeID);
  
  this->IntensityNormalizationParameterList.
    erase(this->IntensityNormalizationParameterList.begin() + index);
}

void
vtkMRMLEMSTargetNode::
RemoveNthVolume(int n)
{
  Superclass::RemoveNthVolume(n);
  
  this->IntensityNormalizationParameterList.
    erase(this->IntensityNormalizationParameterList.begin() + n);
}

void
vtkMRMLEMSTargetNode::
MoveNthVolume(int n, int toIndex)
{
  if (toIndex == n)
    {
    return;
    }

  Superclass::MoveNthVolume(n, toIndex);

  IntensityNormalizationParameterListIterator b = 
    this->IntensityNormalizationParameterList.begin();
  vtksys_stl::swap(*(b+n), *(b+toIndex));
}
