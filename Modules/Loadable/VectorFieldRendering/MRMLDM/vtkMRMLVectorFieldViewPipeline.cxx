/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#include "vtkMRMLVectorFieldViewPipeline.h"
#include "vtkMRMLVectorField2DViewPipeline.h"
#include "vtkMRMLVectorField3DViewPipeline.h"

// LayerDM includes
#include <vtkMRMLLayerDMPipelineCreateHelper.h>
#include <vtkMRMLLayerDMPipelineCreator.h>
#include <vtkMRMLLayerDMPipelineFactory.h>

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorFieldPipeline.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkLookupTable.h>

namespace
{
/// The events of the displayable node that change what is drawn
const std::vector<unsigned long> DisplayableNodeEvents = {
  vtkMRMLDisplayableNode::TransformModifiedEvent,
  vtkMRMLDisplayableNode::DisplayModifiedEvent,
  vtkMRMLModelNode::PolyDataModifiedEvent,
  vtkMRMLVolumeNode::ImageDataModifiedEvent,
};

/// Creates the pipelines in the views, set by RegisterPipelineCreator()
vtkSmartPointer<vtkMRMLLayerDMPipelineCreator> PipelineCreator;
} // namespace

//---------------------------------------------------------------------------
vtkMRMLVectorFieldViewPipeline::vtkMRMLVectorFieldViewPipeline() = default;

//---------------------------------------------------------------------------
vtkMRMLVectorFieldViewPipeline::~vtkMRMLVectorFieldViewPipeline() = default;

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldViewPipeline::RegisterPipelineCreator()
{
  if (PipelineCreator)
  {
    return;
  }
  PipelineCreator = vtkMRMLLayerDMPipelineFactory::GetInstance()->AddPipelineCreator(
    [](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node)
    {
      return layer_dm::TryCreate<vtkMRMLViewNode,
                                 vtkMRMLVectorFieldDisplayNode,
                                 vtkMRMLVectorField3DViewPipeline,
                                 vtkMRMLSliceNode,
                                 vtkMRMLVectorFieldDisplayNode,
                                 vtkMRMLVectorField2DViewPipeline>(viewNode, node);
    });
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldViewPipeline::UnregisterPipelineCreator()
{
  if (!PipelineCreator)
  {
    return;
  }
  vtkMRMLLayerDMPipelineFactory::GetInstance()->RemovePipelineCreator(PipelineCreator);
  PipelineCreator = nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldViewPipeline::SetDisplayNode(vtkMRMLNode* displayNode)
{
  this->Superclass::SetDisplayNode(displayNode);
  this->FieldPipeline = vtkMRMLVectorFieldPipeline::GetSharedPipeline(vtkMRMLVectorFieldDisplayNode::SafeDownCast(displayNode));
  this->UpdateDisplayableNodeObservation();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldViewPipeline::SetScene(vtkMRMLScene* scene)
{
  this->Superclass::SetScene(scene);
  // Nodes are not updated while the scene is batch processing (loading, closing), so all of them are updated after it
  this->UpdateObservation(this->ObservedScene, scene, vtkMRMLScene::EndBatchProcessEvent);
  this->ObservedScene = scene;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldViewPipeline::UpdateDisplayableNodeObservation()
{
  vtkMRMLDisplayableNode* displayableNode = this->GetDisplayableNode();
  this->UpdateObservation(this->ObservedDisplayableNode, displayableNode, DisplayableNodeEvents);
  this->ObservedDisplayableNode = displayableNode;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldViewPipeline::OnUpdate(vtkObject* caller, unsigned long event, void* callData)
{
  if (caller == this->ObservedDisplayableNode && event == vtkMRMLDisplayableNode::DisplayModifiedEvent && callData == this->GetDisplayNode())
  {
    // Already handled, the display node itself is observed
    return;
  }
  this->UpdateDisplay();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldViewPipeline::UpdateFromMRML()
{
  // The display node is added to the scene before it is added to its displayable node
  this->UpdateDisplayableNodeObservation();
  vtkMRMLScene* scene = this->GetScene();
  if (scene && scene->IsBatchProcessing())
  {
    return;
  }
  this->UpdatePipeline();
}

//---------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* vtkMRMLVectorFieldViewPipeline::GetFieldDisplayNode() const
{
  return vtkMRMLVectorFieldDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//---------------------------------------------------------------------------
vtkMRMLVectorFieldPipeline* vtkMRMLVectorFieldViewPipeline::GetFieldPipeline() const
{
  return this->FieldPipeline;
}

//---------------------------------------------------------------------------
vtkMRMLDisplayableNode* vtkMRMLVectorFieldViewPipeline::GetDisplayableNode() const
{
  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = this->GetFieldDisplayNode();
  return fieldDisplayNode ? fieldDisplayNode->GetDisplayableNode() : nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLVectorFieldViewPipeline::GetEffectiveDisplayNode(double& hierarchyOpacity) const
{
  hierarchyOpacity = 1.0;
  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = this->GetFieldDisplayNode();
  vtkMRMLDisplayableNode* displayableNode = this->GetDisplayableNode();
  vtkMRMLAbstractViewNode* viewNode = this->GetViewNode();
  if (!fieldDisplayNode || !displayableNode || !viewNode || !this->FieldPipeline)
  {
    return nullptr;
  }
  bool visibleInViewType = vtkMRMLSliceNode::SafeDownCast(viewNode) ? fieldDisplayNode->GetVisibility2D() : fieldDisplayNode->GetVisibility3D();
  if (!fieldDisplayNode->GetVisibility() || !visibleInViewType || !fieldDisplayNode->IsDisplayableInView(viewNode->GetID()))
  {
    return nullptr;
  }

  // Use hierarchy display node if any, and if overriding is allowed for the current display node.
  vtkMRMLDisplayNode* effectiveDisplayNode = fieldDisplayNode;
  if (fieldDisplayNode->GetFolderDisplayOverrideAllowed())
  {
    if (!vtkMRMLFolderDisplayNode::GetHierarchyVisibility(displayableNode))
    {
      return nullptr;
    }
    vtkMRMLDisplayNode* overrideHierarchyDisplayNode = vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);
    if (overrideHierarchyDisplayNode)
    {
      effectiveDisplayNode = overrideHierarchyDisplayNode;
    }
    hierarchyOpacity = vtkMRMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
  }
  return effectiveDisplayNode;
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldViewPipeline::HasRenderedActiveScalar(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  const char* activeScalarName = displayNode->GetRenderedActiveScalarName();
  return (activeScalarName && activeScalarName[0] != '\0');
}

//---------------------------------------------------------------------------
vtkSmartPointer<vtkLookupTable> vtkMRMLVectorFieldViewPipeline::CreateLookupTable(vtkMRMLColorNode* colorNode)
{
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::Take(colorNode ? colorNode->CreateLookupTableCopy() : nullptr);
  if (!lookupTable)
  {
    lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->Build();
  }
  return lookupTable;
}
