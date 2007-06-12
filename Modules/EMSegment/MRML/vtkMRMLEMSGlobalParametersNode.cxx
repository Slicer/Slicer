#include "vtkMRMLEMSGlobalParametersNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>
#include <iterator>

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

  this->RegistrationAtlasVolumeKey = NULL;
  this->RegistrationTargetVolumeKey = NULL;

  this->WorkingDirectory = NULL;

  this->SaveIntermediateResults       = 0;
  this->SaveSurfaceModels             = 0;
  this->MultithreadingEnabled         = 0;

  this->SegmentationBoundaryMin[0] = 0;
  this->SegmentationBoundaryMin[1] = 0;
  this->SegmentationBoundaryMin[2] = 0;  

  this->SegmentationBoundaryMax[0] = 0;
  this->SegmentationBoundaryMax[1] = 0;
  this->SegmentationBoundaryMax[2] = 0;  
}

//-----------------------------------------------------------------------------
vtkMRMLEMSGlobalParametersNode::~vtkMRMLEMSGlobalParametersNode()
{
  this->SetWorkingDirectory(NULL);
  this->SetRegistrationTargetVolumeKey(NULL);
  this->SetRegistrationAtlasVolumeKey(NULL);
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSGlobalParametersNode::
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
vtkMRMLEMSGlobalParametersNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  for (IntensityNormalizationParameterListIterator i = 
         this->IntensityNormalizationParameterList.begin(); 
       i != this->IntensityNormalizationParameterList.end(); ++i)
    {
    if (this->Scene->GetNodeByID((*i).c_str()) == NULL)
      {
      *i = "NULL";
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "NumberOfTargetInputChannels=\"" 
     << this->NumberOfTargetInputChannels << "\" ";

  of << indent << "WorkingDirectory=\"" 
     << (this->WorkingDirectory ? this->WorkingDirectory : "NULL") << "\" ";
  
    {
    vtksys_stl::stringstream ss;
    ss << this->SegmentationBoundaryMin[0] << " " 
       << this->SegmentationBoundaryMin[1] << " " 
       << this->SegmentationBoundaryMin[2];
    of << indent << "SegmentationBoundaryMin=\"" << ss.str() << "\" ";
    }
  
    {
    vtksys_stl::stringstream ss;
    ss << this->SegmentationBoundaryMax[0] << " " 
       << this->SegmentationBoundaryMax[1] << " " 
       << this->SegmentationBoundaryMax[2];
    of << indent << "SegmentationBoundaryMax=\"" << ss.str() << "\" ";
    }

    of << indent << "RegistrationAffineType=\"" 
       << this->RegistrationAffineType << "\" ";
    of << indent << "RegistrationDeformableType=\"" 
       << this->RegistrationDeformableType << "\" ";
    of << indent << "RegistrationInterpolationType=\"" 
       << this->RegistrationInterpolationType << "\" ";

    of << indent << "RegistrationAtlasVolumeKey=\"" 
       << (this->RegistrationAtlasVolumeKey ? this->RegistrationAtlasVolumeKey 
           : "") << "\" ";

    of << indent << "RegistrationTargetVolumeKey=\"" 
       << (this->RegistrationTargetVolumeKey
           ? this->RegistrationTargetVolumeKey 
           : "") << "\" ";

    of << indent << "SaveIntermediateResults=\"" 
       << this->SaveIntermediateResults << "\" ";
    of << indent << "SaveSurfaceModels=\"" 
       << this->SaveSurfaceModels << "\" ";
    of << indent << "MultithreadingEnabled=\"" 
       << this->MultithreadingEnabled << "\" ";

    of << indent << "IntensityNormalizationParameterNodeIDs=\"";
    vtksys_stl::copy(this->IntensityNormalizationParameterList.begin(),
                     this->IntensityNormalizationParameterList.end(),
                     vtksys_stl::
                     ostream_iterator<vtksys_stl::string>(of, " "));
    of << "\" ";
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
        ss >> this->NumberOfTargetInputChannels;
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
    else if (!strcmp(key, "RegistrationAtlasVolumeKey"))
      {
      this->SetRegistrationAtlasVolumeKey(val);
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
    if (!strcmp(key, "IntensityNormalizationParameterNodeIDs"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      vtksys_stl::string s;

      int index = 0;
      while (ss >> s)
        {
        this->IntensityNormalizationParameterList.push_back(s);
        if (s != "NULL")
          {
          this->Scene->AddReferencedNodeID(s.c_str(), this);
          }
        ++index;
        }
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
  this->SetWorkingDirectory(node->WorkingDirectory);

  this->SetSegmentationBoundaryMin(node->SegmentationBoundaryMin);
  this->SetSegmentationBoundaryMax(node->SegmentationBoundaryMax);

  this->SetRegistrationAffineType(node->RegistrationAffineType);
  this->SetRegistrationDeformableType(node->RegistrationDeformableType);
  this->SetRegistrationInterpolationType(node->RegistrationInterpolationType);
  this->SetRegistrationAtlasVolumeKey(node->RegistrationAtlasVolumeKey);
  this->SetRegistrationTargetVolumeKey(node->RegistrationTargetVolumeKey);
  
  this->SetSaveIntermediateResults(node->SaveIntermediateResults);
  this->SetSaveSurfaceModels(node->SaveSurfaceModels);
  this->SetMultithreadingEnabled(node->MultithreadingEnabled);

  this->IntensityNormalizationParameterList = 
    node->IntensityNormalizationParameterList;
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::PrintSelf(ostream& os, 
                                               vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "NumberOfTargetInputChannels: "
     << this->NumberOfTargetInputChannels << "\n";

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

  os << indent << "RegistrationAtlasVolumeKey: " 
     << (this->RegistrationAtlasVolumeKey ? 
         this->RegistrationAtlasVolumeKey : "(none)") << "\n";

  os << indent << "RegistrationTargetVolumeKey: " 
     << (this->RegistrationTargetVolumeKey ? 
         this->RegistrationTargetVolumeKey : "(none)") << "\n";

  os << indent << "SaveIntermediateResults: " 
     << this->SaveIntermediateResults << "\n";
  os << indent << "SaveSurfaceModels: " 
     << this->SaveSurfaceModels << "\n";
  os << indent << "MultithreadingEnabled: " 
     << this->MultithreadingEnabled << "\n";

  os << indent << "IntensityNormalizationParameterNodeIDs: ";
  vtksys_stl::copy(this->IntensityNormalizationParameterList.begin(),
                   this->IntensityNormalizationParameterList.end(),
                   vtksys_stl::ostream_iterator<vtksys_stl::string>(os, " "));
  os << "\n";
}

const char*
vtkMRMLEMSGlobalParametersNode::
GetNthIntensityNormalizationParametersNodeID(int n)
{
  return this->IntensityNormalizationParameterList[n].c_str();
}


vtkMRMLEMSIntensityNormalizationParametersNode*
vtkMRMLEMSGlobalParametersNode::
GetNthIntensityNormalizationParametersNode(int n)
{
  vtkMRMLEMSIntensityNormalizationParametersNode* node = NULL;
  if (this->GetScene() && 
      this->GetNthIntensityNormalizationParametersNodeID(n))
    {
      vtkMRMLNode* snode = this->GetScene()->
        GetNodeByID(this->GetNthIntensityNormalizationParametersNodeID(n));
      node = 
        vtkMRMLEMSIntensityNormalizationParametersNode::SafeDownCast(snode);
    }
  return node;  
}

void
vtkMRMLEMSGlobalParametersNode::
SetNthIntensityNormalizationParametersNodeID(int n, const char* id)
{
  this->IntensityNormalizationParameterList[n] = id;
  this->Scene->AddReferencedNodeID(id, this);  
}

void
vtkMRMLEMSGlobalParametersNode::
AddTargetInputChannel()
{
  ++this->NumberOfTargetInputChannels;

  // create intensity normalization parameter node
  vtkMRMLEMSIntensityNormalizationParametersNode* intensityNormalizationNode
    = vtkMRMLEMSIntensityNormalizationParametersNode::New();
  intensityNormalizationNode->SetScene(this->GetScene());
  this->GetScene()->AddNode(intensityNormalizationNode);

  // add it to the scene
  this->IntensityNormalizationParameterList.
    push_back(intensityNormalizationNode->GetID());
  this->GetScene()->
    AddReferencedNodeID(intensityNormalizationNode->GetID(), this);

  // clean up
  intensityNormalizationNode->Delete();
}

void
vtkMRMLEMSGlobalParametersNode::
RemoveNthTargetInputChannel(int n)
{
  --this->NumberOfTargetInputChannels;
  this->IntensityNormalizationParameterList.
    erase(this->IntensityNormalizationParameterList.begin() + n);
}

void
vtkMRMLEMSGlobalParametersNode::
MoveNthTargetInputChannel(int n, int toIndex)
{
  if (toIndex == n)
    {
    return;
    }
  IntensityNormalizationParameterListIterator b = 
    this->IntensityNormalizationParameterList.begin();
  std::string movingParam = this->IntensityNormalizationParameterList[n];
  this->IntensityNormalizationParameterList.erase(b + n);
  this->IntensityNormalizationParameterList.insert(b + toIndex, movingParam);
}
