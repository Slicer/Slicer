#include "vtkMRMLEMSTreeParametersLeafNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
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
  this->PrintQuality                = 0;
  this->IntensityLabel              = 0;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode::~vtkMRMLEMSTreeParametersLeafNode()
{
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

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
    
    if (!strcmp(key, "PrintQuality"))
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
void
vtkMRMLEMSTreeParametersLeafNode::
AddTargetInputChannel()
{
  this->LogMean.push_back(0.0);
  for (unsigned int i = 0; i < this->NumberOfTargetInputChannels; ++i)
    {
    this->LogCovariance[i].push_back(0.0);
    }
  ++this->NumberOfTargetInputChannels; 
  this->LogCovariance.
    push_back(vtkstd::vector<double>(this->NumberOfTargetInputChannels, 0.0));
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
RemoveTargetInputChannel(int index)
{
  this->LogMean.erase(this->LogMean.begin() + index);
  for (unsigned int i = 0; i < this->NumberOfTargetInputChannels; ++i)
    {
    this->LogCovariance[i].erase(this->LogCovariance[i].begin() + index);
    }
  this->LogCovariance.erase(this->LogCovariance.begin() + index);
  --this->NumberOfTargetInputChannels;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
MoveTargetInputChannel(int fromIndex, int toIndex)
{
  double movingValue = this->LogMean[fromIndex];
  std::rotate(this->LogMean.begin()+fromIndex,
              this->LogMean.begin()+fromIndex+1,
              this->LogMean.begin()+toIndex+1);
  this->LogMean[toIndex] = movingValue;

  vtkstd::vector<double> movingVector = this->LogCovariance[fromIndex];
  std::rotate(this->LogCovariance.begin()+fromIndex,
              this->LogCovariance.begin()+fromIndex+1,
              this->LogCovariance.begin()+toIndex+1);
  this->LogCovariance[toIndex] = movingVector;  
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



