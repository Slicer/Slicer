#include "vtkMRMLEMSGlobalParametersNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>

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
  this->AtlasNodeID                   = NULL;
  this->TargetNodeID                  = NULL;

  this->RegistrationAffineType        = 0;
  this->RegistrationDeformableType    = 0;
  this->RegistrationInterpolationType = 0;

  this->RegistrationAtlasVolumeKey    = NULL;
  this->RegistrationTargetVolumeKey   = NULL;

  this->WorkingDirectory              = NULL;

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
  this->SetAtlasNodeID(NULL);
  this->SetTargetNodeID(NULL);
  this->SetWorkingDirectory(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "AtlasNodeID=\"" 
     << (this->AtlasNodeID ? this->AtlasNodeID : "NULL") << "\" ";

  of << indent << "TargetNodeID=\"" 
     << (this->TargetNodeID ? this->TargetNodeID : "NULL") << "\" ";

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
     << (this->RegistrationTargetVolumeKey ? this->RegistrationTargetVolumeKey 
         : "") << "\" ";

  of << indent << "SaveIntermediateResults=\"" 
     << this->SaveIntermediateResults << "\" ";
  of << indent << "SaveSurfaceModels=\"" 
     << this->SaveSurfaceModels << "\" ";
  of << indent << "MultithreadingEnabled=\"" 
     << this->MultithreadingEnabled << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSGlobalParametersNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->AtlasNodeID && !strcmp(oldID, this->AtlasNodeID))
  {
    this->SetAtlasNodeID(newID);
  }
  if (this->TargetNodeID && !strcmp(oldID, this->TargetNodeID))
  {
    this->SetTargetNodeID(newID);
  }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSGlobalParametersNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->AtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->AtlasNodeID) == NULL)
  {
    this->SetAtlasNodeID(NULL);
  }
  if (this->TargetNodeID != NULL && 
      this->Scene->GetNodeByID(this->TargetNodeID) == NULL)
  {
    this->SetTargetNodeID(NULL);
  }
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
    
    if (!strcmp(key, "AtlasNodeID"))
    {
      this->SetAtlasNodeID(val);
      this->Scene->AddReferencedNodeID(this->AtlasNodeID, this); 
    }
    else if (!strcmp(key, "TargetNodeID"))
    {
      this->SetTargetNodeID(val);
      this->Scene->AddReferencedNodeID(this->TargetNodeID, this); 
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
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSGlobalParametersNode* node = 
    (vtkMRMLEMSGlobalParametersNode*) rhs;

  this->SetAtlasNodeID(node->AtlasNodeID);
  this->SetTargetNodeID(node->TargetNodeID);
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
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSGlobalParametersNode::PrintSelf(ostream& os, 
                                                 vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "AtlasNodeID: "         
     << (this->AtlasNodeID ? this->AtlasNodeID : "(none)") << "\n";

  os << indent << "TargetNodeID: "        
     << (this->TargetNodeID ? this->TargetNodeID : "(none)") << "\n";

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
}

//-----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkMRMLEMSGlobalParametersNode::
GetAtlasNode()
{
  vtkMRMLEMSAtlasNode* node = NULL;
  if (this->GetScene() && this->GetAtlasNodeID() )
  {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->AtlasNodeID);
    node = vtkMRMLEMSAtlasNode::SafeDownCast(snode);
  }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkMRMLEMSGlobalParametersNode::
GetTargetNode()
{
  vtkMRMLEMSTargetNode* node = NULL;
  if (this->GetScene() && this->GetTargetNodeID() )
  {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->TargetNodeID);
    node = vtkMRMLEMSTargetNode::SafeDownCast(snode);
  }
  return node;
}

//-----------------------------------------------------------------------------
int
vtkMRMLEMSGlobalParametersNode::
GetNumberOfTargetInputChannels()
{
  //
  // get target node
  //
  vtkMRMLEMSTargetNode* target = this->GetTargetNode();
  if (target == NULL)
  {
    return 0;
  }
  else
  {
    return target->GetNumberOfVolumes();
  }
}
