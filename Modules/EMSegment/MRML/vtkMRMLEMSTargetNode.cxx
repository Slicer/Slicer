#include "vtkMRMLEMSTargetNode.h"
#include <sstream>
#include "vtkMRMLScene.h"

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
  this->InputChannelNames.clear();
}

vtkMRMLEMSTargetNode::~vtkMRMLEMSTargetNode()
{
  this->InputChannelNames.clear();
}

void vtkMRMLEMSTargetNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
   vtkIndent indent(nIndent);
  of << indent << " InputChannelNames=\"";
  for (std::vector<std::string>::iterator i = this->InputChannelNames.begin(); i != this->InputChannelNames.end(); 
       ++i)
    {
      of << *i << " " ;
    }
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
    if (!strcmp(key, "InputChannelNames"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      vtksys_stl::string name;
      while (ss >> name)
         {
          this->AddInputChannelName(name.c_str());
        }
      }

    }
}

void vtkMRMLEMSTargetNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSTargetNode* node = (vtkMRMLEMSTargetNode*) rhs;

  this->InputChannelNames= node->InputChannelNames;
}

void vtkMRMLEMSTargetNode::PrintSelf(ostream& os, 
                                     vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << " InputChannelNames: \n";
  for (std::vector<std::string>::iterator i = this->InputChannelNames.begin(); i != this->InputChannelNames.end(); 
       ++i)
    {
      os << indent << "   " << *i << "\n" ;
    }
}

int vtkMRMLEMSTargetNode::AddInputChannelName(const char* name)
{
  // enforce unique keys
  int n = this->InputChannelNames.size();
  this->InputChannelNames.resize(n+1);
  this->InputChannelNames[n] = name;
  return n;
}

void vtkMRMLEMSTargetNode::SetNumberOfInputChannelName(vtkIdType newSize)
{
  // enforce unique keys
  int oldSize = this->InputChannelNames.size();
  this->InputChannelNames.resize(newSize);
  for (int i = oldSize ; i < newSize; i++) 
    {
      this->InputChannelNames[i] = "";
    }
}


int vtkMRMLEMSTargetNode::GetNumberOfInputChannelNames()
{
  return  this->InputChannelNames.size();
}

const char* vtkMRMLEMSTargetNode::GetNthInputChannelName(vtkIdType index)
{
  if (index >= this->GetNumberOfInputChannelNames())
    {
      return NULL;
    }
  return this->InputChannelNames[index].c_str();
}

void vtkMRMLEMSTargetNode::SetNthInputChannelName(vtkIdType index,  const char* newName)
{
  if (index >= this->GetNumberOfInputChannelNames())
    {
      return;
    }
  this->InputChannelNames[index] = newName;
}
