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
#include "vtkMRMLGPURayCastVolumeRenderingDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STL includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLGPURayCastVolumeRenderingDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLGPURayCastVolumeRenderingDisplayNode::vtkMRMLGPURayCastVolumeRenderingDisplayNode()
{
  this->RaycastTechnique = vtkMRMLGPURayCastVolumeRenderingDisplayNode::Composite;
}

//----------------------------------------------------------------------------
vtkMRMLGPURayCastVolumeRenderingDisplayNode::~vtkMRMLGPURayCastVolumeRenderingDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLGPURayCastVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
{
  this->Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
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
void vtkMRMLGPURayCastVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " raycastTechnique=\"" << this->RaycastTechnique << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLGPURayCastVolumeRenderingDisplayNode::Copy(vtkMRMLNode *anode)
{
  int wasModifying = this->StartModify();
  this->Superclass::Copy(anode);
  vtkMRMLGPURayCastVolumeRenderingDisplayNode *node = vtkMRMLGPURayCastVolumeRenderingDisplayNode::SafeDownCast(anode);

  this->SetRaycastTechnique(node->GetRaycastTechnique());

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLGPURayCastVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << "RaycastTechnique: " << this->RaycastTechnique << "\n";
}
