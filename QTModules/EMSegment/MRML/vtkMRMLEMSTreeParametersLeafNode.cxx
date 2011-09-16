#include "vtkMRMLEMSTreeParametersLeafNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <vtkObjectFactory.h>
#include <algorithm>

#include <vtksys/ios/sstream>

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
  this->NumberOfTargetInputChannels     = 0;
  this->PrintQuality                    = 0;
  this->IntensityLabel                  = 0;
  this->DistributionSpecificationMethod = 0;
  this->LogMean.clear();
  this->LogCovariance.clear();
  this->LogMeanCorrection.clear();
  this->LogCovarianceCorrection.clear();
  this->SubParcellationVolumeName     = NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode::~vtkMRMLEMSTreeParametersLeafNode()
{
this->SetSubParcellationVolumeName(NULL);
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

  of << indent << "LogMeanCorrection=\"";
  for (unsigned int i = 0; i < this->GetNumberOfTargetInputChannels(); ++i)
    {
    of << this->LogMeanCorrection[i] << " ";
    }
  of << "\" ";



  of << indent << "LogCovariance=\"";
  for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
    {
    for (unsigned int c = 0; c < this->GetNumberOfTargetInputChannels(); ++c)  
      {
      of << this->LogCovariance[r][c] << " ";
      }
    if (r < this->GetNumberOfTargetInputChannels() - 1)
      {
      of << "| ";
      }
    }
  of << "\" ";

  of << indent << "LogCovarianceCorrection=\"";
  for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
    {
    for (unsigned int c = 0; c < this->GetNumberOfTargetInputChannels(); ++c)  
      {
      of << this->LogCovarianceCorrection[r][c] << " ";
      }
    if (r < this->GetNumberOfTargetInputChannels() - 1)
      {
      of << "| ";
      }
    }
  of << "\" ";


  of << indent << "DistributionSpecificationMethod=\""
     << this->DistributionSpecificationMethod << "\" ";

  of << indent << "DistributionSamplePointsRAS=\"";
  for (SamplePointListConstIterator i = 
         this->DistributionSamplePointsRAS.begin();
       i != this->DistributionSamplePointsRAS.end(); ++i)
    {
    of << (*i)[0] << " " << (*i)[1] << " " << (*i)[2] << " ";
    }
  of << "\" ";

   of << indent << "SubParcellationVolumeName=\"" 
     << (this->SubParcellationVolumeName ? this->SubParcellationVolumeName : "")
     << "\" "; 
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  //
  // we assume an even number of attrs
  //
  unsigned int i;
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
            this->SetNumberOfTargetInputChannels((int)tmpVec.size());
        }
      
      // copy data
      vtksys_stl::copy(tmpVec.begin(), tmpVec.end(), this->LogMean.begin());
      }
    else if (!strcmp(key, "LogCovariance"))
      {
      // remove visual row separators
      std::string valStr(val);
      for (i = 0; i < valStr.size(); ++i)
        {
        if (valStr[i] == '|')
          {
          valStr[i] = ' ';
          }
        }

      // read data into a temporary vector
      vtksys_stl::stringstream ss;
      ss << valStr;
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
      i = 0;
      for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
        {
        for (unsigned int c = 0; c<this->GetNumberOfTargetInputChannels(); ++c)
          {
          this->LogCovariance[r][c] = tmpVec[i++];
          }   
        }
      }
    else if (!strcmp(key, "LogMeanCorrection"))
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
            this->SetNumberOfTargetInputChannels((int)tmpVec.size());
        }
      
      // copy data
      vtksys_stl::copy(tmpVec.begin(), tmpVec.end(), this->LogMeanCorrection.begin());
      }
    else if (!strcmp(key, "LogCovarianceCorrection"))
      {
      // remove visual row seperators
      std::string valStr(val);
      for (i = 0; i < valStr.size(); ++i)
        {
        if (valStr[i] == '|')
          {
          valStr[i] = ' ';
          }
        }

      // read data into a temporary vector
      vtksys_stl::stringstream ss;
      ss << valStr;
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
      i = 0;
      for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
        {
        for (unsigned int c = 0; c<this->GetNumberOfTargetInputChannels(); ++c)
          {
          this->LogCovarianceCorrection[r][c] = tmpVec[i++];
          }   
        }
      }
    else if (!strcmp(key, "DistributionSpecificationMethod"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->DistributionSpecificationMethod;
      }
    else if (!strcmp(key, "DistributionSamplePointsRAS"))
      {
      this->DistributionSamplePointsRAS.clear();
      vtksys_stl::stringstream ss;
      ss << val;
      vtkstd::vector<double> point(3);
      while (ss >> point[0] >> point[1] >> point[2])
        {
        this->DistributionSamplePointsRAS.push_back(point);
        }
      }
     else if (!strcmp(key, "SubParcellationVolumeName"))
      {
      this->SetSubParcellationVolumeName(val);
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
  this->LogMeanCorrection = node->LogMeanCorrection;
  this->LogCovarianceCorrection = node->LogCovarianceCorrection;
  this->SetDistributionSpecificationMethod
    (node->DistributionSpecificationMethod);
  this->DistributionSamplePointsRAS = node->DistributionSamplePointsRAS;
  this->SetSubParcellationVolumeName(node->SubParcellationVolumeName);
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

  os << indent << "LogMeanCorrection: ";
  for (unsigned int i = 0; i < this->GetNumberOfTargetInputChannels(); ++i)
    {
    os << this->LogMeanCorrection[i] << " ";
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

  os << indent << "LogCovarianceCorrection: ";
  for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
    {
    for (unsigned int c = 0; c < this->GetNumberOfTargetInputChannels(); ++c)
      {
      os << this->LogCovarianceCorrection[r][c] << " ";
      }
    }
  os << "\n";

  os << indent << "DistributionSpecificationMethod: "
     << this->DistributionSpecificationMethod
     << "\n";

  os << indent << "DistributionSamplePointsRAS: \n";
  for (SamplePointListConstIterator i = 
         this->DistributionSamplePointsRAS.begin();
       i != this->DistributionSamplePointsRAS.end(); ++i)
    {
    os << (*i)[0] << " " << (*i)[1] << " " << (*i)[2] << "\n";
    }

 os << indent << "SubParcellationVolumeName: " 
     << (this->SubParcellationVolumeName ? this->SubParcellationVolumeName :"(none)")
     << "\n"; 
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
    this->LogMeanCorrection.resize(n);
    vtksys_stl::fill(this->LogMeanCorrection.begin(), this->LogMeanCorrection.end(), 0.0);

    // resize LogCovariance, don't preserve data!
    this->LogCovariance.clear();
    this->LogCovariance.resize(n);
    this->LogCovarianceCorrection.clear();
    this->LogCovarianceCorrection.resize(n);
    for (unsigned int i = 0; i < n; ++i)
      {
      this->LogCovariance[i].resize(n, 0.0);
      this->LogCovarianceCorrection[i].resize(n, 0.0);
      }
    }

}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
AddTargetInputChannel()
{
  this->LogMean.push_back(0.0);
  this->LogMeanCorrection.push_back(0.0);
  for (unsigned int i = 0; i < this->NumberOfTargetInputChannels; ++i)
    {
    this->LogCovariance[i].push_back(0.0);
    this->LogCovarianceCorrection[i].push_back(0.0);
    }
  ++this->NumberOfTargetInputChannels; 
  this->LogCovariance.push_back(vtkstd::vector<double>(this->NumberOfTargetInputChannels, 0.0));
  this->LogCovarianceCorrection.push_back(vtkstd::vector<double>(this->NumberOfTargetInputChannels, 0.0));
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
RemoveNthTargetInputChannel(int index)
{
  this->LogMean.erase(this->LogMean.begin() + index);
  this->LogMeanCorrection.erase(this->LogMeanCorrection.begin() + index);
  for (unsigned int i = 0; i < this->NumberOfTargetInputChannels; ++i)
    {
    this->LogCovariance[i].erase(this->LogCovariance[i].begin() + index);
    this->LogCovarianceCorrection[i].erase(this->LogCovarianceCorrection[i].begin() + index);
    }
  this->LogCovariance.erase(this->LogCovariance.begin() + index);
  this->LogCovarianceCorrection.erase(this->LogCovarianceCorrection.begin() + index);
  --this->NumberOfTargetInputChannels;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
MoveNthTargetInputChannel(int fromIndex, int toIndex)
{
  double movingParam = this->LogMean[fromIndex];
  this->LogMean.erase(this->LogMean.begin() + fromIndex);
  this->LogMean.insert(this->LogMean.begin() + toIndex, movingParam);

  movingParam = this->LogMeanCorrection[fromIndex];
  this->LogMeanCorrection.erase(this->LogMeanCorrection.begin() + fromIndex);
  this->LogMeanCorrection.insert(this->LogMeanCorrection.begin() + toIndex, movingParam);

  for (unsigned int i = 0; i < this->NumberOfTargetInputChannels; ++i)
    {
    movingParam = this->LogCovariance[i][fromIndex];
    this->LogCovariance[i].erase(this->LogCovariance[i].begin() + fromIndex);
    this->LogCovariance[i].insert(this->LogCovariance[i].begin() + toIndex, movingParam);

    movingParam = this->LogCovarianceCorrection[i][fromIndex];
    this->LogCovarianceCorrection[i].erase(this->LogCovarianceCorrection[i].begin() + fromIndex);
    this->LogCovarianceCorrection[i].insert(this->LogCovarianceCorrection[i].begin() + toIndex, movingParam);
    }

  vtkstd::vector<double> movingVec = this->LogCovariance[fromIndex];
  this->LogCovariance.erase(this->LogCovariance.begin() + fromIndex);
  this->LogCovariance.insert(this->LogCovariance.begin() + toIndex, movingVec);

  movingVec = this->LogCovarianceCorrection[fromIndex];
  this->LogCovarianceCorrection.erase(this->LogCovarianceCorrection.begin() + fromIndex);
  this->LogCovarianceCorrection.insert(this->LogCovarianceCorrection.begin() + toIndex, movingVec);
}

//-----------------------------------------------------------------------------
double vtkMRMLEMSTreeParametersLeafNode::GetLogMean(int index) 
{
  int size = int(this->LogMean.size());
  if (size <= index)
    {
      vtkErrorMacro("Index (" << index << ") surpasses size of logMean (" << size << ") !");
      return -1;
    }
  return this->LogMean[index];
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
SetLogMean(int index, double value)
{
  if (int(this->LogMean.size()) <= index)
    {
      vtkErrorMacro("Index (" << index << ") surpasses size of logMean (" << int(this->LogMean.size()) << ") !");
      return;
    }

  if (value != this->LogMean[index] ) 
    {
      this->LogMean[index] = value;
      this->Modified();
    }
}


//-----------------------------------------------------------------------------
double vtkMRMLEMSTreeParametersLeafNode::GetLogMeanCorrection(int index) 
{
    if (int(this->LogMeanCorrection.size()) <= index)
    {
      vtkErrorMacro("Index (" << index << ") surpasses size of logMeanCorrection (" << int(this->LogMeanCorrection.size()) << ") !");
      return 0;
    }
  return this->LogMeanCorrection[index];
}



//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
SetLogMeanCorrection(int index, double value)
{
    if (int(this->LogMeanCorrection.size()) <= index)
    {
      vtkErrorMacro("Index (" << index << ") surpasses size of logMeanCorrection (" << int(this->LogMeanCorrection.size()) << ") !");
      return;
    }

  if (value != this->LogMeanCorrection[index] ) 
    {
      this->LogMeanCorrection[index] = value;
      this->Modified();
    }
}

//-----------------------------------------------------------------------------
double
vtkMRMLEMSTreeParametersLeafNode::
GetLogCovariance(int row, int column) 
{
  if ((int(this->LogCovariance.size()) <= row)  || (int(this->LogCovariance.size()) <= column)  )
    {   
      vtkErrorMacro("Row (" << row << ")  or column (" << column<< ") surpasses size of logCovariance (" << int(this->LogCovariance.size()) << " x " << int(this->LogCovariance.size()) << ") !");
      return -1;
    }
  return this->LogCovariance[row][column];
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
SetLogCovariance(int row, int column, double value)
{
  if ((int(this->LogCovariance.size()) <= row) || (int(this->LogCovariance.size()) <= column))
    {
      vtkErrorMacro("Row (" << row << ")  or column (" << column<< ") surpasses size of logCovariance (" << int(this->LogCovariance.size()) << " x " << int(this->LogCovariance.size()) << ") !");
      return;
    }

  if (value != this->LogCovariance[row][column])
    { 
      this->LogCovariance[row][column] = value;
      this->Modified();
    }
}

//-----------------------------------------------------------------------------
double
vtkMRMLEMSTreeParametersLeafNode::
GetLogCovarianceCorrection(int row, int column) 
{
  if ((int(this->LogCovarianceCorrection.size()) <= row) || (int(this->LogCovarianceCorrection.size()) <= column))
    {
      vtkErrorMacro("Row (" << row << ")  or column (" << column<< ") surpasses size of logCovarianceCorrection (" << int(this->LogCovarianceCorrection.size()) << " x " << int(this->LogCovarianceCorrection.size()) << ") !");
      return 0;
    }
  return this->LogCovarianceCorrection[row][column];
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
SetLogCovarianceCorrection(int row, int column, double value)
{
   if ((int(this->LogCovarianceCorrection.size()) <= row) || (int(this->LogCovarianceCorrection.size()) <= column))
    {
      vtkErrorMacro("Row (" << row << ")  or column (" << column<< ") surpasses size of logCovarianceCorrection (" << int(this->LogCovarianceCorrection.size()) << " x " << int(this->LogCovarianceCorrection.size()) << ") !");
      return;
    }
  if (value != this->LogCovarianceCorrection[row][column])
    { 
      this->LogCovarianceCorrection[row][column] = value;
      this->Modified();
    }
}

//-----------------------------------------------------------------------------
int 
vtkMRMLEMSTreeParametersLeafNode::
GetNumberOfSamplePoints() const
{
  return (int) this->DistributionSamplePointsRAS.size();
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTreeParametersLeafNode::
AddSamplePoint(double xyz[3])
{
  PointType point(3);
  point[0] = xyz[0];
  point[1] = xyz[1];
  point[2] = xyz[2];
  this->DistributionSamplePointsRAS.push_back(point);
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTreeParametersLeafNode::
RemoveNthSamplePoint(int n)
{
  this->DistributionSamplePointsRAS.erase
    (this->DistributionSamplePointsRAS.begin()+n);
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTreeParametersLeafNode::
ClearSamplePoints()
{
  this->DistributionSamplePointsRAS.clear();
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTreeParametersLeafNode::
GetNthSamplePoint(int n, double xyz[3]) const
{
  if (n >= static_cast<int>(this->DistributionSamplePointsRAS.size()))
    {
    std::cerr << "ERROR: invalid sample number" << n << std::endl;
    }
  xyz[0] = this->DistributionSamplePointsRAS[n][0];
  xyz[1] = this->DistributionSamplePointsRAS[n][1];
  xyz[2] = this->DistributionSamplePointsRAS[n][2];
}

