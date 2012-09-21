/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode);

//----------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode, FgVolumeNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode, FgVolumePropertyNodeID);

//----------------------------------------------------------------------------
vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode()
{
  this->FgVolumeNodeID = NULL;
  this->FgVolumeNode = NULL;

  this->FgVolumePropertyNodeID = NULL;
  this->FgVolumePropertyNode = NULL;

  this->DepthPeelingThreshold = 0.0f;

  this->RaycastTechnique = vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::Composite;
  this->RaycastTechniqueFg = vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::Composite;

  this->ThresholdFg[0] = 0.0;
  this->ThresholdFg[1] = 1.0;

  this->BgFgRatio = 0.0f;//default display bg volume

  this->MultiVolumeFusionMethod = vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::AlphaBlendingOR;

  this->WindowLevelFg[0] = 0.0;
  this->WindowLevelFg[1] = 0.0;
}

//----------------------------------------------------------------------------
vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode
::~vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode()
{
  if (this->FgVolumeNodeID)
    {
    this->SetAndObserveFgVolumeNodeID(NULL);
    }

  if (this->FgVolumePropertyNodeID)
    {
    this->SetAndObserveFgVolumePropertyNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
{
  this->Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "fgVolumeNodeID"))
      {
      this->SetFgVolumeNodeID(attValue);
      continue;
      }
    if (!strcmp(attName, "fgVolumePropertyNodeID"))
      {
      this->SetFgVolumePropertyNodeID(attValue);
      continue;
      }
    if (!strcmp(attName,"depthPeelingThreshold"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->DepthPeelingThreshold;
      continue;
      }
    if (!strcmp(attName,"raycastTechnique"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->RaycastTechnique;
      continue;
      }
    if (!strcmp(attName,"raycastTechniqueFg"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->RaycastTechniqueFg;
      continue;
      }
    if (!strcmp(attName,"useFgThreshold"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UseFgThreshold;
      continue;
      }
    if (!strcmp(attName,"bgFgRatio"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->BgFgRatio;
      continue;
      }
    if (!strcmp(attName,"multiVolumeFusionMethod"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MultiVolumeFusionMethod;
      continue;
      }
    if (!strcmp(attName,"thresholdFg"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ThresholdFg[0];
      ss >> this->ThresholdFg[1];
      continue;
      }
    if (!strcmp(attName,"windowLevelFg"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->WindowLevelFg[0];
      ss >> this->WindowLevelFg[1];
      continue;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " fgVolumeNodeID=\"" << (this->FgVolumeNodeID ? this->FgVolumeNodeID : "NULL") << "\"";
  of << indent << " fgVolumePropertyNodeID=\"" << (this->FgVolumePropertyNodeID ? this->FgVolumePropertyNodeID : "NULL") << "\"";
  of << indent << " depthPeelingThreshold=\"" << this->DepthPeelingThreshold << "\"";
  of << indent << " raycastTechnique=\"" << this->RaycastTechnique << "\"";
  of << indent << " raycastTechniqueFg=\"" << this->RaycastTechniqueFg << "\"";
  of << indent << " multiVolumeFusionMethod=\"" << this->MultiVolumeFusionMethod << "\"";
  of << indent << " thresholdFg=\"" << this->ThresholdFg[0] << " " << this->ThresholdFg[1] << "\"";
  of << indent << " useFgThreshold=\"" << this->UseFgThreshold << "\"";
  of << indent << " bgFgRatio=\"" << this->BgFgRatio << "\"";
  of << indent << " windowLevelFg=\"" << this->WindowLevelFg[0] << " " << this->WindowLevelFg[1] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->FgVolumeNodeID, this);
  this->Scene->AddReferencedNodeID(this->FgVolumePropertyNodeID, this);
}

//----------------------------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID,newID);
  if (this->FgVolumeNodeID && !strcmp(oldID, this->FgVolumeNodeID))
    {
    this->SetAndObserveFgVolumeNodeID(newID);
    }
  if (this->FgVolumePropertyNodeID && !strcmp(oldID, this->FgVolumePropertyNodeID))
    {
    this->SetAndObserveFgVolumePropertyNodeID(newID);
    }
}

//-----------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::UpdateReferences()
{
   this->Superclass::UpdateReferences();

  if (this->FgVolumeNodeID != NULL && this->Scene->GetNodeByID(this->FgVolumeNodeID) == NULL)
    {
    this->SetAndObserveFgVolumeNodeID(NULL);
    }
  if (this->FgVolumePropertyNodeID != NULL && this->Scene->GetNodeByID(this->FgVolumePropertyNodeID) == NULL)
    {
    this->SetAndObserveFgVolumePropertyNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::Copy(vtkMRMLNode *anode)
{
  int wasModifying = this->StartModify();
  this->Superclass::Copy(anode);
  vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode *node = vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::SafeDownCast(anode);

  this->SetFgVolumeNodeID(node->GetFgVolumeNodeID());
  this->SetFgVolumePropertyNodeID(node->GetFgVolumePropertyNodeID());
  this->SetDepthPeelingThreshold(node->GetDepthPeelingThreshold());
  this->SetRaycastTechnique(node->GetRaycastTechnique());
  this->SetRaycastTechniqueFg(node->GetRaycastTechniqueFg());
  this->SetThresholdFg(node->GetThresholdFg());
  //this->SetUseFgThreshold(node->GetUseFgThreshold());
  this->SetBgFgRatio(node->GetBgFgRatio());
  this->SetMultiVolumeFusionMethod(node->GetMultiVolumeFusionMethod());
  this->SetWindowLevelFg(node->GetWindowLevelFg());
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::SetAndObserveFgVolumeNodeID(const char *volumeNodeID)
{
  this->SetFgVolumeNodeID(volumeNodeID);
  this->FgVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    this->GetScene() ? this->GetScene()->GetNodeByID(volumeNodeID) : 0);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::GetFgVolumeNode()
{
  if (((this->FgVolumeNode != NULL && strcmp(this->FgVolumeNode->GetID(), this->FgVolumeNodeID)) ||
      (this->FgVolumeNode == NULL)) )
    {
    this->FgVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
      this->GetScene() ? this->GetScene()->GetNodeByID(this->FgVolumeNodeID) : 0);
    }
  return this->FgVolumeNode;
}

//----------------------------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode
::SetAndObserveFgVolumePropertyNodeID(const char *VolumePropertyNodeID)
{
  this->SetFgVolumePropertyNodeID(VolumePropertyNodeID);
  vtkMRMLVolumePropertyNode *node = this->GetFgVolumePropertyNode();
  vtkSetAndObserveMRMLObjectEventsMacro(
    this->FgVolumePropertyNode, node, this->ObservedEvents);
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::GetFgVolumePropertyNode()
{
  if (this->FgVolumePropertyNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectEventsMacro(this->FgVolumePropertyNode, NULL, this->ObservedEvents);
    }
  else if (this->GetScene() &&
           ((this->FgVolumePropertyNode != NULL && strcmp(this->FgVolumePropertyNode->GetID(), this->FgVolumePropertyNodeID)) ||
            (this->FgVolumePropertyNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->FgVolumePropertyNodeID);
    vtkSetAndObserveMRMLObjectEventsMacro(
      this->FgVolumePropertyNode,
      vtkMRMLVolumePropertyNode::SafeDownCast(snode), this->ObservedEvents);
    }
  return this->FgVolumePropertyNode;
}

//-----------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
  this->Superclass::UpdateScene(scene);
  this->SetAndObserveFgVolumeNodeID(this->FgVolumeNodeID);
  this->SetAndObserveFgVolumePropertyNodeID(this->FgVolumePropertyNodeID);
}

//---------------------------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  if (event == vtkCommand::ModifiedEvent &&
      this->FgVolumePropertyNode == node &&
      this->BgFgRatio > 0.)
    {
    this->Modified();
    }
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << "DepthPeelingThreshold: " << this->DepthPeelingThreshold << "\n";
  os << "RaycastTechnique: " << this->RaycastTechnique << "\n";
  os << "RaycastTechniqueFg: " << this->RaycastTechniqueFg << "\n";
  os << "FgVolumeNodeID: " << ( (this->FgVolumeNodeID) ? this->FgVolumeNodeID : "None" ) << "\n";
  os << "FgVolumePropertyNodeID: " << ( (this->FgVolumePropertyNodeID) ? this->FgVolumePropertyNodeID : "None" ) << "\n";
}
