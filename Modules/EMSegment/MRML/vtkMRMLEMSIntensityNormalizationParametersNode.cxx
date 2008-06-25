#include "vtkMRMLEMSIntensityNormalizationParametersNode.h"
#include <sstream>
#include "vtkMRMLScene.h"

#include <vtksys/ios/sstream>

//-----------------------------------------------------------------------------
vtkMRMLEMSIntensityNormalizationParametersNode* 
vtkMRMLEMSIntensityNormalizationParametersNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::
    CreateInstance("vtkMRMLEMSIntensityNormalizationParametersNode");
  if(ret)
    {
    return (vtkMRMLEMSIntensityNormalizationParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSIntensityNormalizationParametersNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSIntensityNormalizationParametersNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::
    CreateInstance("vtkMRMLEMSIntensityNormalizationParametersNode");
  if(ret)
    {
    return (vtkMRMLEMSIntensityNormalizationParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSIntensityNormalizationParametersNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSIntensityNormalizationParametersNode::vtkMRMLEMSIntensityNormalizationParametersNode()
{
  this->SetToDefaultT1SPGR();
  this->PrintInfo                      = 1;
  this->Enabled                        = 1;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSIntensityNormalizationParametersNode::~vtkMRMLEMSIntensityNormalizationParametersNode()
{
  // nothing to do here
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSIntensityNormalizationParametersNode::
SetToDefaultT1SPGR()
{
  this->NormValue                      = 90;
  this->NormType                       = 1; //!!!INTENSITY_NORM_MEAN_MRI;
  this->InitialHistogramSmoothingWidth = 5;
  this->MaxHistogramSmoothingWidth     = 10;
  this->RelativeMaxVoxelNum            = 0.99;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSIntensityNormalizationParametersNode::
SetToDefaultT2()
{
  this->NormValue                      = 310;
  this->NormType                       = 1; //!!!INTENSITY_NORM_MEAN_MRI;
  this->InitialHistogramSmoothingWidth = 5;
  this->MaxHistogramSmoothingWidth     = 10;
  this->RelativeMaxVoxelNum            = 0.99;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSIntensityNormalizationParametersNode::
SetToDefaultT2_2()
{
  this->NormValue                      = 310;
  this->NormType                       = 1; //!!!INTENSITY_NORM_MEAN_MRI;
  this->InitialHistogramSmoothingWidth = 5;
  this->MaxHistogramSmoothingWidth     = 10;
  this->RelativeMaxVoxelNum            = 0.95;
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSIntensityNormalizationParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "NormValue=\"" 
     << this->NormValue << "\" ";
  of << indent << "NormType=\"" 
     << this->NormType << "\" ";
  of << indent << "InitialHistogramSmoothingWidth=\"" 
     << this->InitialHistogramSmoothingWidth << "\" ";
  of << indent << "MaxHistogramSmoothingWidth=\"" 
     << this->MaxHistogramSmoothingWidth << "\" ";
  of << indent << "RelativeMaxVoxelNum=\"" 
     << this->RelativeMaxVoxelNum << "\" ";
  of << indent << "PrintInfo=\"" 
     << this->PrintInfo << "\" ";
  of << indent << "Enabled=\"" 
     << this->Enabled << "\" ";
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSIntensityNormalizationParametersNode::ReadXMLAttributes(const char** attrs)
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
    
    if (!strcmp(key, "NormValue"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->NormValue;
      }
    else if (!strcmp(key, "NormType"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->NormType;
      }
    else if (!strcmp(key, "InitialHistogramSmoothingWidth"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->InitialHistogramSmoothingWidth;
      }
    else if (!strcmp(key, "MaxHistogramSmoothingWidth"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->MaxHistogramSmoothingWidth;
      }
    else if (!strcmp(key, "RelativeMaxVoxelNum"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->RelativeMaxVoxelNum;
      }
    else if (!strcmp(key, "PrintInfo"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->PrintInfo;
      }
    else if (!strcmp(key, "Enabled"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->Enabled;
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSIntensityNormalizationParametersNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSIntensityNormalizationParametersNode* node = 
    (vtkMRMLEMSIntensityNormalizationParametersNode*) rhs;

  this->SetNormValue(node->NormValue);
  this->SetNormType(node->NormType);
  this->
    SetInitialHistogramSmoothingWidth(node->InitialHistogramSmoothingWidth);
  this->SetMaxHistogramSmoothingWidth(node->MaxHistogramSmoothingWidth);
  this->SetRelativeMaxVoxelNum(node->RelativeMaxVoxelNum);
  this->SetPrintInfo(node->PrintInfo);
  this->SetEnabled(node->Enabled);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSIntensityNormalizationParametersNode::PrintSelf(ostream& os, 
                                                     vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "NormValue: " << this->NormValue << "\n";
  os << indent << "NormType: " << this->NormType << "\n";
  os << indent << "InitialHistogramSmoothingWidth: " 
     << this->InitialHistogramSmoothingWidth << "\n";
  os << indent << "MaxHistogramSmoothingWidth: " 
     << this->MaxHistogramSmoothingWidth << "\n";
  os << indent << "RelativeMaxVoxelNum: " 
     << this->RelativeMaxVoxelNum << "\n";
  os << indent << "PrintInfo: " 
     << this->PrintInfo << "\n";
  os << indent << "Enabled: " 
     << this->Enabled << "\n";
}


