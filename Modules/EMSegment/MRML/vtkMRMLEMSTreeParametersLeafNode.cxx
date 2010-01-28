/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLEMSTreeParametersLeafNode.cxx,v$
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

=======================================================================auto=*/

#include "vtkMRMLEMSTreeParametersLeafNode.h"
#include <sstream>

#include "vtkMRMLScene.h"
#include <algorithm>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode* vtkMRMLEMSTreeParametersLeafNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLEMSTreeParametersLeafNode");

  if (ret)
  {
    return (vtkMRMLEMSTreeParametersLeafNode*)ret;
  }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeParametersLeafNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLEMSTreeParametersLeafNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLEMSTreeParametersLeafNode");

  if (ret)
  {
    return (vtkMRMLEMSTreeParametersLeafNode*)ret;
  }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeParametersLeafNode;
}

//----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode::vtkMRMLEMSTreeParametersLeafNode()
{
  this->NumberOfTargetInputChannels     = 0;
  this->PrintQuality                    = 0;
  this->IntensityLabel                  = 0;
  this->DistributionSpecificationMethod = 0;
}

//----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode::~vtkMRMLEMSTreeParametersLeafNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent
    << " PrintQuality=\""
    << this->PrintQuality
    << "\"";

  of << indent
    << " IntensityLabel=\""
    << this->IntensityLabel
    << "\"";

  of << indent
    << " LogMean=\"";

  for (unsigned int i = 0; i < this->GetNumberOfTargetInputChannels(); ++i)
  {
    of << this->LogMean[i] << " ";
  }
  of << "\"";

  of << indent
    << " Mean=\"";

  for (unsigned int i = 0; i < this->GetNumberOfTargetInputChannels(); ++i)
  {
    of << this->Mean[i] << " ";
  }
  of << "\"";

  of << indent
    << " LogCovariance=\"";

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
  of << "\"";

  of << indent
    << " Covariance=\"";

  for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels(); ++r)
  {
    for (unsigned int c = 0; c < this->GetNumberOfTargetInputChannels(); ++c)
    {
      of << this->Covariance[r][c] << " ";
    }

    if (r < this->GetNumberOfTargetInputChannels() - 1)
    {
      of << "| ";
    }
  }
  of << "\"";

  of << indent
    << " DistributionSpecificationMethod=\""
    << this->DistributionSpecificationMethod
    << "\"";

  of << indent
    << " DistributionSamplePointsRAS=\"";

  for (SamplePointListConstIterator i = this->
      DistributionSamplePointsRAS.begin();
       i != this->DistributionSamplePointsRAS.end(); i++)
  {
    of << (*i)[0] << " " << (*i)[1] << " " << (*i)[2] << " ";
  }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  // we assume an even number of attrs

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
    else if (!strcmp(key,"Mean"))
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
      vtksys_stl::copy(tmpVec.begin(), tmpVec.end(), this->Mean.begin());
    }
    else if (!strcmp(key, "LogCovariance"))
    {
      // remove visual row seperators
      std::string valStr(val);

      for (i=0; i < valStr.size(); ++i)
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
      for (unsigned int r = 0; r < this->GetNumberOfTargetInputChannels();r++)
        {
        for (unsigned int c=0; c < this->GetNumberOfTargetInputChannels();c++)
          {
          this->LogCovariance[r][c] = tmpVec[i++];
          }
        }
    }
    else if (!strcmp(key, "Covariance"))
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
      int numTargets = this->GetNumberOfTargetInputChannels();

      for (int r=0; r < numTargets; r++)
      {
        for (int c=0; c < numTargets; c++)
        {
          this->Covariance[r][c] = tmpVec[i++];
        }
      }
    }
    else if (strcmp(key,"DistributionSpecificationMethod") == 0)
    {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->DistributionSpecificationMethod;
    }
    else if (strcmp(key, "DistributionSamplePointsRAS") == 0)
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
  }
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);

  vtkMRMLEMSTreeParametersLeafNode* node = (vtkMRMLEMSTreeParametersLeafNode*)
    rhs;

  int numTargets = node->GetNumberOfTargetInputChannels();

  this->SetNumberOfTargetInputChannels(numTargets);
  this->SetPrintQuality(node->PrintQuality);
  this->SetIntensityLabel(node->IntensityLabel);

  this->LogMean       = node->LogMean;
  this->Mean          = node->Mean;
  this->LogCovariance = node->LogCovariance;
  this->Covariance    = node->Covariance;

  this->SetDistributionSpecificationMethod
    (node->DistributionSpecificationMethod);
  this->DistributionSamplePointsRAS = node->DistributionSamplePointsRAS;
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::PrintSelf(ostream& os,vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  int numTargets = this->GetNumberOfTargetInputChannels();

  os << indent << "PrintQuality: "
     << this->PrintQuality
     << "\n";

  os << indent << "IntensityLabel: "
     << this->IntensityLabel
     << "\n";

  os << indent << "LogMean: ";

  for (int i=0; i < numTargets; i++)
    {
    os << this->LogMean[i] << " ";
    }

  os << "\n";

  os << indent << "Mean: ";

  for (int i=0; i < numTargets; i++)
  {
    os << this->Mean[i] << " ";
  }

  os << "\n";

  os << indent << "LogCovariance: ";

  for (int r=0; r < numTargets; r++)
    {
    for (int c=0; c < numTargets; c++)
      {
      os << this->LogCovariance[r][c] << " ";
      }
    }

  os << "\n";

  os << indent << "Covariance: ";

  for (int r=0; r < numTargets; r++)
    {
    for (int c=0; c < numTargets; c++)
      {
      os << this->Covariance[r][c] << " ";
      }
    }

  os << "\n";

  os << indent
    << "DistributionSpecificationMethod: "
    << this->DistributionSpecificationMethod
    << "\n";

  os << indent << "DistributionSamplePointsRAS: \n";

  for (SamplePointListConstIterator i = this->
      DistributionSamplePointsRAS.begin();
      i != this->DistributionSamplePointsRAS.end(); i++)
    {
    os << (*i)[0] << " " << (*i)[1] << " " << (*i)[2] << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::AddChildNode(const char* vtkNotUsed(childNodeID))
{
  // nothing to do here
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::RemoveNthChildNode(int vtkNotUsed(n))
{
  // nothing to do here
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::MoveNthChildNode(int vtkNotUsed(fromIndex), int
    vtkNotUsed(toIndex))
{
  // nothing to do here
}

//----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
SetNumberOfTargetInputChannels(
    unsigned int n)
{
  if (n != this->GetNumberOfTargetInputChannels())
  {
    this->NumberOfTargetInputChannels = n;

    // resize LogMean, do not preserve data!

    this->LogMean.resize(n);

    vtksys_stl::fill(this->LogMean.begin(), this->LogMean.end(), 0.0);

    // resize Mean, do not preserve data!
    this->Mean.resize(n);

    vtksys_stl::fill(this->Mean.begin(), this->Mean.end(), 0.0);

    // resize LogCovariance, do not preserve data!
    this->LogCovariance.clear();
    this->LogCovariance.resize(n);

    for (unsigned int i=0; i < n; i++)
    {
      this->LogCovariance[i].resize(n, 0.0);
    }

    // resize Covariance, do not preserve data!
    this->Covariance.clear();
    this->Covariance.resize(n);

    for (unsigned int i=0; i < n; i++)
    {
      this->Covariance[i].resize(n, 0.0);
    }
  }
}

//----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
AddTargetInputChannel()
{
  this->LogMean.push_back(0.0);
  this->Mean.   push_back(0.0);

  for (unsigned int i=0; i < this->NumberOfTargetInputChannels; i++)
  {
    this->LogCovariance[i].push_back(0.0);
    this->Covariance[i].   push_back(0.0);
  }

  unsigned int numTargets = ++this->NumberOfTargetInputChannels;

  this->LogCovariance.push_back(vtkstd::vector<double>(numTargets,0.0));
  this->Covariance.   push_back(vtkstd::vector<double>(numTargets,0.0));
}

//----------------------------------------------------------------------------
void
vtkMRMLEMSTreeParametersLeafNode::
RemoveNthTargetInputChannel(
    int index)
{
  this->LogMean.erase(this->LogMean.begin() + index);
  this->Mean.   erase(this->Mean.begin()    + index);

  for (unsigned int i=0; i < this->NumberOfTargetInputChannels; ++i)
  {
    this->LogCovariance[i].erase(this->LogCovariance[i].begin() + index);
    this->Covariance[i].   erase(this->Covariance[i].begin()    + index);
  }

  this->LogCovariance.erase(this->LogCovariance.begin() + index);
  this->Covariance.   erase(this->Covariance.begin()    + index);

  this->NumberOfTargetInputChannels--;
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::
MoveNthTargetInputChannel(
    int fromIndex,
    int toIndex)
{
  double movingParam = this->LogMean[fromIndex];

  this->LogMean.erase( this->LogMean.begin() + fromIndex);
  this->LogMean.insert(this->LogMean.begin() + toIndex, movingParam);

  double movingParam2 = this->LogMean[fromIndex];

  this->Mean.erase( this->Mean.begin() + fromIndex);
  this->Mean.insert(this->Mean.begin() + toIndex, movingParam);

  for (unsigned int i=0; i < this->NumberOfTargetInputChannels; i++)
  {
    movingParam = this->LogCovariance[i][fromIndex];

    this->LogCovariance[i].erase(this->LogCovariance[i].begin() + fromIndex);
    this->LogCovariance[i].insert(this->LogCovariance[i].begin() + toIndex,
                                  movingParam);
  }

  vtkstd::vector<double> movingVec = this->LogCovariance[fromIndex];

  this->LogCovariance.erase(this->LogCovariance.begin() + fromIndex);
  this->LogCovariance.insert(this->LogCovariance.begin() + toIndex,movingVec);

  for (unsigned int i=0; i < this->NumberOfTargetInputChannels; i++)
  {
    movingParam2 = this->Covariance[i][fromIndex];
    this->Covariance[i].erase(this->Covariance[i].begin() + fromIndex);
    this->Covariance[i].insert(this->Covariance[i].begin() + toIndex,
        movingParam2);
  }

  vtkstd::vector<double> movingVec2 = this->Covariance[fromIndex];
  this->Covariance.erase(this->Covariance.begin() + fromIndex);
  this->Covariance.insert(this->Covariance.begin() + toIndex,
      movingVec2);
}

//----------------------------------------------------------------------------
double vtkMRMLEMSTreeParametersLeafNode::GetLogMean(int index) const
{
  return this->LogMean[index];
}

//----------------------------------------------------------------------------
double vtkMRMLEMSTreeParametersLeafNode::GetMean(int index) const
{
  return this->Mean[index];
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::SetLogMean(int index, double value)
{
  this->LogMean[index] = value;
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::SetMean(int index, double value)
{
  this->Mean[index] = value;
}

//----------------------------------------------------------------------------
double vtkMRMLEMSTreeParametersLeafNode::GetLogCovariance(int row, int column)
  const
{
  return this->LogCovariance[row][column];
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::SetCovariance(int row, int column,
    double value)
{
  this->Covariance[row][column] = value;
}

//----------------------------------------------------------------------------
double vtkMRMLEMSTreeParametersLeafNode::GetCovariance(int row, int column)
  const
{
  return this->Covariance[row][column];
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::SetLogCovariance(int row, int column,
    double value)
{
  this->LogCovariance[row][column] = value;
}

//----------------------------------------------------------------------------
int vtkMRMLEMSTreeParametersLeafNode::GetNumberOfSamplePoints() const
{
  return (int)this->DistributionSamplePointsRAS.size();
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::AddSamplePoint(double xyz[3])
{
  PointType point(3);

  point[0] = xyz[0];
  point[1] = xyz[1];
  point[2] = xyz[2];

  this->DistributionSamplePointsRAS.push_back(point);
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::RemoveNthSamplePoint(int n)
{
  this->DistributionSamplePointsRAS.erase(this->DistributionSamplePointsRAS.
      begin()+n);
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::ClearSamplePoints()
{
  this->DistributionSamplePointsRAS.clear();
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTreeParametersLeafNode::GetNthSamplePoint(int n, double xyz[3])
  const
{
  if (n >= static_cast<int>(this->DistributionSamplePointsRAS.size()))
  {
    std::cerr << __FILE__ << "\n"
      << "\t" << "Line" << __LINE__
      << ": " << "\nERROR: invalid sample number" << n << std::endl;
  }

  xyz[0] = this->DistributionSamplePointsRAS[n][0];
  xyz[1] = this->DistributionSamplePointsRAS[n][1];
  xyz[2] = this->DistributionSamplePointsRAS[n][2];
}

