#include "vtkMRMLEMSTreeParametersLeafNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include <algorithm>

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode* 
vtkMRMLEMSTreeParametersLeafNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTreeParametersLeafNode");
  if(ret)
  {
    return (vtkMRMLEMSTreeParametersLeafNode*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeParametersLeafNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSTreeParametersLeafNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTreeParametersLeafNode");
  if(ret)
  {
    return (vtkMRMLEMSTreeParametersLeafNode*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeParametersLeafNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode::vtkMRMLEMSTreeParametersLeafNode()
{
  this->NumberOfTargetInputChannels = 0;
  this->GlobalParametersNodeID      = NULL;
  this->PrintQuality                = 0;
  this->IntensityLabel              = 0;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode::~vtkMRMLEMSTreeParametersLeafNode()
{
  this->SetGlobalParametersNodeID(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "GlobalParametersNodeID=\"" 
     << (this->GlobalParametersNodeID ? this->GlobalParametersNodeID : "NULL")
     << "\" ";

  of << indent << "PrintQuality=\""
     << this->PrintQuality << "\" ";

  of << indent << "IntensityLabel=\""
     << this->IntensityLabel << "\" ";

  of << indent << "LogMean=\"";
  for (unsigned int i = 0; i < this->GetNumberOfTargetInputChannels(); ++i)
  {
    of << this->LogMean[i] << " ";
  }
  of << "\" ";

  of << indent << "LogCovariance=\"";
  for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
  {
    for (unsigned int c = 0; c < this->GetNumberOfTargetInputChannels(); ++c)  
    {
      of << this->LogCovariance[r][c] << " ";
    }
  }
  of << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->GlobalParametersNodeID && 
      !strcmp(oldID, this->GlobalParametersNodeID))
  {
    this->SetGlobalParametersNodeID(newID);
  }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTreeParametersLeafNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->GlobalParametersNodeID != NULL && 
      this->Scene->GetNodeByID(this->GlobalParametersNodeID) == NULL)
  {
    this->SetGlobalParametersNodeID(NULL);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  //
  // we assume an even number of attrs
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
    else if (!strcmp(key, "PrintQuality"))
    {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->PrintQuality;
    }
    else if (!strcmp(key, "IntensityLabel"))
    {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->IntensityLabel;
    }
    else if (!strcmp(key, "LogMean"))
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
      if (this->GetNumberOfTargetInputChannels() != tmpVec.size())
      {
        this->SetNumberOfTargetInputChannels(tmpVec.size());
      }
      
      // copy data
      vtksys_stl::copy(tmpVec.begin(), tmpVec.end(), this->LogMean.begin());
    }
    else if (!strcmp(key, "LogCovariance"))
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
      // assume square matrix
      unsigned int side = (unsigned int) sqrt((double)tmpVec.size());
      if (this->GetNumberOfTargetInputChannels() != side)
      {
        this->SetNumberOfTargetInputChannels(side);
      }

      // copy data
      unsigned int i = 0;
      for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
      {
        for (unsigned int c = 0; c<this->GetNumberOfTargetInputChannels(); ++c)
        {
          this->LogCovariance[r][c] = tmpVec[i++];
        }   
      }
    }
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSTreeParametersLeafNode* node = 
    (vtkMRMLEMSTreeParametersLeafNode*) rhs;

  this->SetGlobalParametersNodeID(node->GlobalParametersNodeID);
  this->SetNumberOfTargetInputChannels(node->GetNumberOfTargetInputChannels());
  this->SetPrintQuality(node->PrintQuality);
  this->SetIntensityLabel(node->IntensityLabel);
  this->LogMean = node->LogMean;
  this->LogCovariance = node->LogCovariance;
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::PrintSelf(ostream& os, 
                                                 vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "GlobalParametersNodeID: "         
     << (this->GlobalParametersNodeID ? this->GlobalParametersNodeID : "(none)" )
     << "\n";

  os << indent << "PrintQuality: "         
     << this->PrintQuality
     << "\n";

  os << indent << "IntensityLabel: "         
     << this->IntensityLabel
     << "\n";

  os << indent << "LogMean: ";
  for (unsigned int i = 0; i < this->GetNumberOfTargetInputChannels(); ++i)
  {
    os << this->LogMean[i] << " ";
  }
  os << "\n";

  os << indent << "LogCovariance: ";
  for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
  {
    for (unsigned int c = 0; c < this->GetNumberOfTargetInputChannels(); ++c)
    {
      os << this->LogCovariance[r][c] << " ";
    }
  }
  os << "\n";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
SetNumberOfTargetInputChannels(unsigned int n)
{
  if (n != this->GetNumberOfTargetInputChannels())
  {
    this->NumberOfTargetInputChannels = n;
      
    // resize LogMean, don\"t preserve data!
    this->LogMean.resize(n);
    vtksys_stl::fill(this->LogMean.begin(), this->LogMean.end(), 0.0);

    // resize LogCovariance, don't preserve data!
    this->LogCovariance.clear();
    this->LogCovariance.resize(n);
    for (unsigned int i = 0; i < n; ++i)
    {
      this->LogCovariance[i].resize(n, 0.0);
    }
  }
}

//-----------------------------------------------------------------------------
double
vtkMRMLEMSTreeParametersLeafNode::
GetLogMean(int index) const
{
  return this->LogMean[index];
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
SetLogMean(int index, double value)
{
  this->LogMean[index] = value;
}

//-----------------------------------------------------------------------------
double
vtkMRMLEMSTreeParametersLeafNode::
GetLogCovariance(int row, int column) const
{
  return this->LogCovariance[row][column];
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
SetLogCovariance(int row, int column, double value)
{
  this->LogCovariance[row][column] = value;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
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
vtkMRMLEMSGlobalParametersNode*
vtkMRMLEMSTreeParametersLeafNode::
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
