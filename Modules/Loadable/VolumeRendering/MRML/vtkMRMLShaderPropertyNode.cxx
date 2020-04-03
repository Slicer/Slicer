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

  This file was originally developed by Simon Drouin, Brigham and Women's
  Hospital, Boston, MA.

==============================================================================*/

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLShaderPropertyNode.h"
#include "vtkMRMLVolumePropertyStorageNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLShaderPropertyNode);

//----------------------------------------------------------------------------
vtkMRMLShaderPropertyNode::vtkMRMLShaderPropertyNode()
{
  this->ObservedEvents = vtkIntArray::New();
  this->ObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);

  vtkShaderProperty* property = vtkShaderProperty::New();
  vtkSetAndObserveMRMLObjectEventsMacro(this->ShaderProperty, property, this->ObservedEvents);
  property->Delete();

  // Observe uniform variables
  vtkObserveMRMLObjectEventsMacro(this->ShaderProperty->GetVertexCustomUniforms(), this->ObservedEvents);
  vtkObserveMRMLObjectEventsMacro(this->ShaderProperty->GetFragmentCustomUniforms(), this->ObservedEvents);

  this->SetHideFromEditors(0);
}

//----------------------------------------------------------------------------
vtkMRMLShaderPropertyNode::~vtkMRMLShaderPropertyNode()
{
  if(this->ShaderProperty)
    {
    vtkUnObserveMRMLObjectMacro(this->ShaderProperty->GetVertexCustomUniforms());
    vtkUnObserveMRMLObjectMacro(this->ShaderProperty->GetFragmentCustomUniforms());
    vtkSetAndObserveMRMLObjectMacro(this->ShaderProperty, nullptr);
    }
  this->ObservedEvents->Delete();
}

//----------------------------------------------------------------------------
vtkUniforms * vtkMRMLShaderPropertyNode::GetVertexUniforms()
{
  return this->ShaderProperty->GetVertexCustomUniforms();
}

//----------------------------------------------------------------------------
vtkUniforms * vtkMRMLShaderPropertyNode::GetFragmentUniforms()
{
  return this->ShaderProperty->GetFragmentCustomUniforms();
}

//----------------------------------------------------------------------------
vtkUniforms * vtkMRMLShaderPropertyNode::GetGeometryUniforms()
{
  return this->ShaderProperty->GetGeometryCustomUniforms();
}

//----------------------------------------------------------------------------
void vtkMRMLShaderPropertyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

//  vtkMRMLWriteXMLBeginMacro(of);
//  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLShaderPropertyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

//  vtkMRMLReadXMLBeginMacro(atts);
//  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLShaderPropertyNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLShaderPropertyNode* node = vtkMRMLShaderPropertyNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  this->ShaderProperty->DeepCopy( node->ShaderProperty);
}

//----------------------------------------------------------------------------
void vtkMRMLShaderPropertyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ShaderProperty: ";
  this->ShaderProperty->PrintSelf(os,indent.GetNextIndent());
}

//---------------------------------------------------------------------------
void vtkMRMLShaderPropertyNode::ProcessMRMLEvents( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  switch (event)
    {
    case vtkCommand::ModifiedEvent:
      this->Modified();
      break;
    }
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLShaderPropertyNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLShaderPropertyStorageNode"));
}

//---------------------------------------------------------------------------
bool vtkMRMLShaderPropertyNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->ShaderProperty &&
     this->ShaderProperty->GetMTime() > this->GetStoredTime());
}
