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
#include "vtkMRMLNCIRayCastVolumeRenderingDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STL includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLNCIRayCastVolumeRenderingDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLNCIRayCastVolumeRenderingDisplayNode::vtkMRMLNCIRayCastVolumeRenderingDisplayNode()
{
  this->DepthPeelingThreshold = 0.0f;
  this->DistanceColorBlending = 0.0f;

  this->ICPEScale = 1.0f;
  this->ICPESmoothness = 0.5f;

  this->RaycastTechnique = vtkMRMLNCIRayCastVolumeRenderingDisplayNode::Composite;
}

//----------------------------------------------------------------------------
vtkMRMLNCIRayCastVolumeRenderingDisplayNode::~vtkMRMLNCIRayCastVolumeRenderingDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLNCIRayCastVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
{
  this->Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName,"depthPeelingThreshold"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->DepthPeelingThreshold;
      continue;
      }
    if (!strcmp(attName,"distanceColorBlending"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->DistanceColorBlending;
      continue;
      }
    if (!strcmp(attName,"icpeScale"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ICPEScale;
      continue;
      }
    if (!strcmp(attName,"icpeSmoothness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ICPESmoothness;
      continue;
      }
    if (!strcmp(attName,"raycastTechnique"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->RaycastTechnique;
      continue;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNCIRayCastVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " depthPeelingThreshold=\"" << this->DepthPeelingThreshold << "\"";
  of << indent << " distanceColorBlending=\"" << this->DistanceColorBlending << "\"";
  of << indent << " icpeScale=\"" << this->ICPEScale << "\"";
  of << indent << " icpeSmoothness=\"" << this->ICPESmoothness << "\"";
  of << indent << " raycastTechnique=\"" << this->RaycastTechnique << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLNCIRayCastVolumeRenderingDisplayNode::Copy(vtkMRMLNode *anode)
{
  int wasModifying = this->StartModify();
  this->Superclass::Copy(anode);
  vtkMRMLNCIRayCastVolumeRenderingDisplayNode *node = vtkMRMLNCIRayCastVolumeRenderingDisplayNode::SafeDownCast(anode);

  this->SetDepthPeelingThreshold(node->GetDepthPeelingThreshold());
  this->SetDistanceColorBlending(node->GetDistanceColorBlending());
  this->SetICPEScale(node->GetICPEScale());
  this->SetICPESmoothness(node->GetICPESmoothness());
  this->SetRaycastTechnique(node->GetRaycastTechnique());

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLNCIRayCastVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << "DepthPeelingThreshold: " << this->DepthPeelingThreshold << "\n";
  os << "DistanceColorBlending: " << this->DistanceColorBlending << "\n";
  os << "ICPEScale: " << this->ICPEScale << "\n";
  os << "ICPESmoothness: " << this->ICPESmoothness << "\n";
  os << "RaycastTechnique: " << this->RaycastTechnique << "\n";
}
