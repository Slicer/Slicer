#include "vtkMRMLEMSGlobalParametersNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>
#include <vtkObjectFactory.h>
#include <iterator>

#include <vtksys/stl/string>

//-----------------------------------------------------------------------------
vtkMRMLEMSGlobalParametersNode* 
vtkMRMLEMSGlobalParametersNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSGlobalParametersNode");
  if(ret)
    {
    return (vtkMRMLEMSGlobalParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSGlobalParametersNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSGlobalParametersNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSGlobalParametersNode");
  if(ret)
    {
    return (vtkMRMLEMSGlobalParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSGlobalParametersNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSGlobalParametersNode::vtkMRMLEMSGlobalParametersNode()
{
  this->NumberOfTargetInputChannels   = 0;

  this->RegistrationAffineType        = 0;
  this->RegistrationDeformableType    = 0;
  this->RegistrationInterpolationType = 0; // !!! this needs to be specified
  this->RegistrationPackageType       = 0;

  this->RegistrationAtlasVolumeKey.clear(); 
  this->RegistrationTargetVolumeKey = NULL;

  this->EnableTargetToTargetRegistration = 0;

  this->WorkingDirectory = NULL;

  this->SaveIntermediateResults       = 0;
  this->SaveSurfaceModels             = 0;
  this->MultithreadingEnabled         = 1;
  this->UpdateIntermediateData        = 1;

  this->SegmentationBoundaryMin[0] = 0;
  this->SegmentationBoundaryMin[1] = 0;
  this->SegmentationBoundaryMin[2] = 0;  

  this->SegmentationBoundaryMax[0] = 0;
  this->SegmentationBoundaryMax[1] = 0;
  this->SegmentationBoundaryMax[2] = 0;  

  this->Colormap = NULL;          

  this->EnableSubParcellation = 0;
  this->MinimumIslandSize = 1;
  this->Island2DFlag = 0;
  this->AMFSmoothing = 0;
         
  this->InputChannelNames.clear();

  this->TemplateFileName = NULL;
  this->TemplateSaveAfterSegmentation = 0;
  this->TaskTclFileName = NULL;
  this->TaskPreProcessingSetting = NULL;
  this->SetTaskTclFileName(this->GetDefaultTaskTclFileName());

}

//-----------------------------------------------------------------------------
vtkMRMLEMSGlobalParametersNode::~vtkMRMLEMSGlobalParametersNode()
{
  this->SetWorkingDirectory(NULL);
  this->SetRegistrationTargetVolumeKey(NULL);
  this->RegistrationAtlasVolumeKey.clear(); 
  this->SetColormap(NULL);
  
  this->SetTemplateFileName(NULL);
  this->SetTaskTclFileName(NULL);
  this->SetTaskPreProcessingSetting(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " NumberOfTargetInputChannels=\"" 
     << this->NumberOfTargetInputChannels << "\" ";

  of << indent << " InputChannelNames=\"";
  for (std::vector<std::string>::iterator i = this->InputChannelNames.begin(); i != this->InputChannelNames.end(); 
       ++i)
    {
      of << *i << " " ;
    }
   of << "\" ";

  of << indent << " WorkingDirectory=\"" 
     << (this->WorkingDirectory ? this->WorkingDirectory : "NULL") << "\" ";
  
    {
    vtksys_stl::stringstream ss;
    ss << this->SegmentationBoundaryMin[0] << " " 
       << this->SegmentationBoundaryMin[1] << " " 
       << this->SegmentationBoundaryMin[2];
    of << indent << " SegmentationBoundaryMin=\"" << ss.str() << "\" ";
    }
  
    {
    vtksys_stl::stringstream ss;
    ss << this->SegmentationBoundaryMax[0] << " " 
       << this->SegmentationBoundaryMax[1] << " " 
       << this->SegmentationBoundaryMax[2];
    of << indent << " SegmentationBoundaryMax=\"" << ss.str() << "\" ";
    }

    of << indent << " RegistrationAffineType=\"" 
       << this->RegistrationAffineType << "\" ";
    of << indent << " RegistrationDeformableType=\"" 
       << this->RegistrationDeformableType << "\" ";
    of << indent << " RegistrationInterpolationType=\"" 
       << this->RegistrationInterpolationType << "\" ";
    of << indent << " RegistrationPackageType=\""
       << this->RegistrationPackageType << "\" ";

    of << indent << " RegistrationAtlasVolumeKey=\""; 
    for (vtkIdType i =0 ; i < (vtkIdType) this->RegistrationAtlasVolumeKey.size(); i++)
      {
    of << (this->RegistrationAtlasVolumeKey[i].compare("") ? this->RegistrationAtlasVolumeKey[i].c_str() : "-") << " "; 
      }
    of << "\" ";

    of << indent << " RegistrationTargetVolumeKey=\"" 
       << (this->RegistrationTargetVolumeKey
           ? this->RegistrationTargetVolumeKey 
           : "") << "\" ";

    of << indent << " EnableTargetToTargetRegistration=\""
       << this->EnableTargetToTargetRegistration << "\" ";

    of << indent << " SaveIntermediateResults=\"" 
       << this->SaveIntermediateResults << "\" ";
    of << indent << " SaveSurfaceModels=\"" 
       << this->SaveSurfaceModels << "\" ";
    of << indent << " MultithreadingEnabled=\"" 
       << this->MultithreadingEnabled << "\" ";
    of << indent << " UpdateIntermediateData=\"" 
       << this->UpdateIntermediateData << "\" ";

    of << indent << " EnableSubParcellation=\"" << this->EnableSubParcellation << "\" ";
    of << indent << " MinimumIslandSize=\"" << this->MinimumIslandSize << "\" ";
    of << indent << " Island2DFlag=\"" << this->Island2DFlag << "\" ";
    of << indent << " AMFSmoothing=\"" << this->AMFSmoothing << "\" ";

    of << indent << " Colormap=\"" << (this->Colormap ? this->Colormap : "NULL") << "\" ";

    of << indent << "TemplateFileName=\"" 
     << (this->TemplateFileName ? this->TemplateFileName : "NULL") << "\" ";
    of << indent << "TemplateSaveAfterSegmentation=\"" 
     << this->TemplateSaveAfterSegmentation << "\" ";
    of << indent << "TaskTclFileName=\"" 
     << (this->TaskTclFileName ? this->TaskTclFileName  : "NULL") << "\" ";
    of << indent << "TaskPreProcessingSetting=\"" 
     << (this->TaskPreProcessingSetting ? this->TaskPreProcessingSetting : "NULL") << "\" ";
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    
    if (!strcmp(key, "NumberOfTargetInputChannels"))
      {
        vtksys_stl::stringstream ss;
        ss << val;
    int n ;
        ss >> n;
    this->SetNumberOfTargetInputChannels(n);
      }
    else if (!strcmp(key, "EnableTargetToTargetRegistration"))
      {
        vtksys_stl::stringstream ss;
        ss << val;
        ss >> this->EnableTargetToTargetRegistration;
      }
    else if (!strcmp(key, "WorkingDirectory"))
      {
      this->SetWorkingDirectory(val);
      }
    else if (!strcmp(key, "SegmentationBoundaryMin"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      for (unsigned int i = 0; i < 3; ++i)
        {
        int m;
        if (ss >> m)
          { 
          this->SegmentationBoundaryMin[i] = m;
          }
        }
      }
    else if (!strcmp(key, "SegmentationBoundaryMax"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      for (unsigned int i = 0; i < 3; ++i)
        {
        int m;
        if (ss >> m)
          { 
          this->SegmentationBoundaryMax[i] = m;
          }
        }
      }
    else if (!strcmp(key, "RegistrationAffineType"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->RegistrationAffineType;
      }
    else if (!strcmp(key, "RegistrationDeformableType"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->RegistrationDeformableType;
      }
    else if (!strcmp(key, "RegistrationInterpolationType"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->RegistrationInterpolationType;
      }
    else if (!strcmp(key, "RegistrationPackageType"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->RegistrationPackageType;
      }
    else if (!strcmp(key, "RegistrationAtlasVolumeKey"))
      {
        vtksys_stl::stringstream ss;
        ss << val;
        vtksys_stl::string s;

        while (ss >> s)
        {
          this->RegistrationAtlasVolumeKey.push_back(s);
        }
      }
    else if (!strcmp(key, "RegistrationTargetVolumeKey"))
      {
         this->SetRegistrationTargetVolumeKey(val);
      }
    else if (!strcmp(key, "SaveIntermediateResults"))
      {
        vtksys_stl::stringstream ss;
        ss << val;
        ss >> this->SaveIntermediateResults;
      }
    else if (!strcmp(key, "SaveSurfaceModels"))
      {
         vtksys_stl::stringstream ss;
         ss << val;
         ss >> this->SaveSurfaceModels;
      }
    else if (!strcmp(key, "MultithreadingEnabled"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->MultithreadingEnabled;
      }
    else if (!strcmp(key, "UpdateIntermediateData"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->UpdateIntermediateData;
      }
    else if (!strcmp(key, "EnableSubParcellation")) 
      {
        vtksys_stl::stringstream ss;
        ss << val;
        int n ;
        ss >> n;
    this->SetEnableSubParcellation(n);
      }
    else if (!strcmp(key, "MinimumIslandSize"))
      {
        vtksys_stl::stringstream ss;
        ss << val;
        int n ;
        ss >> n;
        this->SetMinimumIslandSize(n);
      } 
    else if (!strcmp(key, "Island2DFlag"))
      {
        vtksys_stl::stringstream ss;
        ss << val;
        int n ;
        ss >> n;
        this->SetIsland2DFlag(n);
      } 
    else if (!strcmp(key, "AMFSmoothing"))
      {
        vtksys_stl::stringstream ss;
        ss << val;
        int n ;
        ss >> n;
        this->SetAMFSmoothing(n);
      } 
    else if (!strcmp(key, "Colormap"))
      {
      this->SetColormap(val);
      }
    else if (!strcmp(key, "InputChannelNames"))
      {
        vtksys_stl::stringstream ss;
        ss << val;
        vtksys_stl::string name;
        int index = 0;
        while (ss >> name)
         {
           this->InputChannelNames.resize(index+1);
           this->InputChannelNames[index] = name;
           index ++;
        }
      }
    else if (!strcmp(key, "TemplateFileName"))
      {
        this->SetTemplateFileName(val);
      }
    else if (!strcmp(key, "TemplateSaveAfterSegmentation"))
      {
         vtksys_stl::stringstream ss;
         ss << val;
         ss >> this->TemplateSaveAfterSegmentation;
      }
    else if (!strcmp(key, "TaskTclFileName"))
      {
        this->SetTaskTclFileName(val);
      }
    else if (!strcmp(key, "TaskPreProcessingSetting"))
      {
        this->SetTaskPreProcessingSetting(val);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSGlobalParametersNode* node = 
    (vtkMRMLEMSGlobalParametersNode*) rhs;

  this->NumberOfTargetInputChannels = node->NumberOfTargetInputChannels;
  this->SetEnableTargetToTargetRegistration(node->EnableTargetToTargetRegistration);
  this->SetWorkingDirectory(node->WorkingDirectory);

  this->SetSegmentationBoundaryMin(node->SegmentationBoundaryMin);
  this->SetSegmentationBoundaryMax(node->SegmentationBoundaryMax);

  this->SetRegistrationAffineType(node->RegistrationAffineType);
  this->SetRegistrationDeformableType(node->RegistrationDeformableType);
  this->SetRegistrationInterpolationType(node->RegistrationInterpolationType);
  this->SetRegistrationPackageType(node->RegistrationPackageType);

  this->RegistrationAtlasVolumeKey = node->RegistrationAtlasVolumeKey;
  this->SetRegistrationTargetVolumeKey(node->RegistrationTargetVolumeKey);
  
  this->SetSaveIntermediateResults(node->SaveIntermediateResults);
  this->SetSaveSurfaceModels(node->SaveSurfaceModels);
  this->SetMultithreadingEnabled(node->MultithreadingEnabled);
  this->SetUpdateIntermediateData(node->UpdateIntermediateData);

  this->SetColormap(node->Colormap);
  this->EnableSubParcellation = node->EnableSubParcellation;
  this->MinimumIslandSize = node->MinimumIslandSize;
  this->Island2DFlag = node->Island2DFlag;
  this->AMFSmoothing = node->AMFSmoothing;

  this->InputChannelNames= node->InputChannelNames;

  this->SetTemplateFileName(node->TemplateFileName);
  this->SetTemplateSaveAfterSegmentation(node->TemplateSaveAfterSegmentation);
  this->SetTaskTclFileName(node->TaskTclFileName);
  this->SetTaskPreProcessingSetting(node->TaskPreProcessingSetting);

}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::PrintSelf(ostream& os, 
                                               vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
 
  os << indent << "NumberOfTargetInputChannels: "
     << this->NumberOfTargetInputChannels << "\n";

  os << indent << "InputChannelNames: \n";
  for (std::vector<std::string>::iterator i = this->InputChannelNames.begin(); i != this->InputChannelNames.end(); 
       ++i)
    {
      os << indent << "   " << *i << "\n" ;
    }

  os << indent << "EnableTargetToTargetRegistration: "
     << (this->EnableTargetToTargetRegistration ? "true" : "false") << "\n";

  os << indent << "WorkingDirectory: " 
     << (this->WorkingDirectory ? this->WorkingDirectory : "(none)") << "\n";

  os << indent << "SegmentationBoundaryMin: " 
     << this->SegmentationBoundaryMin[0] 
     << this->SegmentationBoundaryMin[1] 
     << this->SegmentationBoundaryMin[2] << "\n";

  os << indent << "SegmentationBoundaryMax: " 
     << this->SegmentationBoundaryMax[0] 
     << this->SegmentationBoundaryMax[1] 
     << this->SegmentationBoundaryMax[2] << "\n";

  os << indent << "RegistrationAffineType: " 
     << this->RegistrationAffineType << "\n";
  os << indent << "RegistrationDeformableType: " 
     << this->RegistrationDeformableType << "\n";
  os << indent << "RegistrationInterpolationType: " 
     << this->RegistrationInterpolationType << "\n";
  os << indent << "RegistrationPackageType: "
     << this->RegistrationPackageType << "\n";

  os << indent << "RegistrationAtlasVolumeKey: " ;
  for (vtkIdType i = 0 ; i < (vtkIdType) this->RegistrationAtlasVolumeKey.size(); i++ ) 
    {
      os << (this->RegistrationAtlasVolumeKey[i].compare("") ? this->RegistrationAtlasVolumeKey[i].c_str() : "(none)") << "\n";
    }

  os << indent << "RegistrationTargetVolumeKey: " 
     << (this->RegistrationTargetVolumeKey ? 
         this->RegistrationTargetVolumeKey : "(none)") << "\n";

  os << indent << "SaveIntermediateResults: " 
     << this->SaveIntermediateResults << "\n";
  os << indent << "SaveSurfaceModels: " 
     << this->SaveSurfaceModels << "\n";
  os << indent << "MultithreadingEnabled: " 
     << this->MultithreadingEnabled << "\n";
  os << indent << "UpdateIntermediateData: " 
     << this->UpdateIntermediateData << "\n";

  os << indent << "EnableSubParcellation: " << this->EnableSubParcellation << "\n";
  os << indent << "MinimumIslandSize:     " << this->MinimumIslandSize << "\n";
  os << indent << "Island2DFlag:     " << this->Island2DFlag << "\n";
  os << indent << "AMFSmoothing:     " << this->AMFSmoothing << "\n";

  os << indent << "Colormap: " 
     << (this->Colormap ? this->Colormap : "(none)") << "\n";

  os << indent << "TemplateFileName: " <<
    (this->TemplateFileName ? this->TemplateFileName : "(none)") << "\n";
  os << indent << "TemplateSaveAfterSegmentation: " 
     << this->TemplateSaveAfterSegmentation << "\n";
  os << indent << "TaskTclFileName: " <<
    (this->TaskTclFileName ? this->TaskTclFileName : "(none)") << "\n";
  os << indent << "TaskPreProcessingSetting: " <<
    (this->TaskPreProcessingSetting ? this->TaskPreProcessingSetting : "(none)") << "\n";  

}

void
vtkMRMLEMSGlobalParametersNode::
AddTargetInputChannel()
{
  ++this->NumberOfTargetInputChannels;

  this->InputChannelNames.resize(this->NumberOfTargetInputChannels);
  this->InputChannelNames[this->NumberOfTargetInputChannels-1] = "";

}

void
vtkMRMLEMSGlobalParametersNode::
RemoveNthTargetInputChannel(int n)
{
  if ( (n < 0) && (n >= this->NumberOfTargetInputChannels)) 
    {
      vtkErrorMacro("n is out of range");
      return;
    }  
  --this->NumberOfTargetInputChannels;

  for (int i = n ; i < int(this->InputChannelNames.size() -1); i ++)
    {
      this->InputChannelNames[i]  = this->InputChannelNames[i+1]; 
    }

  this->InputChannelNames.resize(this->NumberOfTargetInputChannels);
}

void
vtkMRMLEMSGlobalParametersNode::
MoveNthTargetInputChannel(int n, int toIndex)
{
  if (toIndex == n)
    {
    return;
    }

  cout << "MoveNthTargetInputChannel: Function still has to change name " << endl;  
}

void vtkMRMLEMSGlobalParametersNode::SetRegistrationAtlasVolumeKey(vtkIdType inputID, const char* key)
{
  if (inputID >= (vtkIdType) this->RegistrationAtlasVolumeKey.size())
    {
      RegistrationAtlasVolumeKey.resize(inputID +1);
    }
  RegistrationAtlasVolumeKey[inputID] = key;

}

const char* vtkMRMLEMSGlobalParametersNode::GetRegistrationAtlasVolumeKey(vtkIdType inputID)
{
  if (inputID >= (vtkIdType) this->RegistrationAtlasVolumeKey.size())
    {
      return NULL;
    }
  return RegistrationAtlasVolumeKey[inputID].c_str();
}

const char* vtkMRMLEMSGlobalParametersNode::GetNthTargetInputChannelName(vtkIdType index)
{
  if (index >= vtkIdType(this->InputChannelNames.size()))
    {
      return NULL;
    }
  return this->InputChannelNames[index].c_str();
}

void vtkMRMLEMSGlobalParametersNode::SetNthTargetInputChannelName(vtkIdType index,  const char* newName)
{
  if (index >= vtkIdType(this->InputChannelNames.size()))
    {
      return;
    }
  this->InputChannelNames[index] = newName;
}


void vtkMRMLEMSGlobalParametersNode::SetNumberOfTargetInputChannels(vtkIdType n)
{
  this->NumberOfTargetInputChannels = n ;
  this->InputChannelNames.resize(n);
}
