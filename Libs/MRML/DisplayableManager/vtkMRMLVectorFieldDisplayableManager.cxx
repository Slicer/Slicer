/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

  This file was further developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLVectorFieldDisplayableManager.h"
#include "vtkMRMLModelDisplayableManager.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorFieldPipeline.h>
#include <vtkVectorFieldGlyph.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkGlyph3DMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkLookupTable.h>
#include <vtkMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

// STD includes
#include <deque>
#include <map>
#include <set>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVectorFieldDisplayableManager);

namespace
{
/// The events of a displayable node that change what is drawn
const unsigned long ObservedEvents[] = {
  vtkMRMLDisplayableNode::TransformModifiedEvent,
  vtkMRMLDisplayableNode::DisplayModifiedEvent,
  vtkMRMLModelNode::PolyDataModifiedEvent,
  vtkMRMLVolumeNode::ImageDataModifiedEvent,
};

//---------------------------------------------------------------------------
/// True if the display node names an array to color the field by
bool HasRenderedActiveScalar(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  const char* activeScalarName = displayNode->GetRenderedActiveScalarName();
  return (activeScalarName && activeScalarName[0] != '\0');
}

//---------------------------------------------------------------------------
/// Copy of the color node's table (or a default table if there is none), so that setting range, alpha
/// and vector mode does not affect other mappers sharing the color node's table.
vtkSmartPointer<vtkLookupTable> CreateLookupTable(vtkMRMLColorNode* colorNode)
{
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::Take(colorNode ? colorNode->CreateLookupTableCopy() : nullptr);
  if (!lookupTable)
  {
    lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->Build();
  }
  return lookupTable;
}
} // namespace

//---------------------------------------------------------------------------
class vtkMRMLVectorFieldDisplayableManager::vtkInternal
{
public:
  /// The filters (GetGlyphPointsConnection(), GetOutputPolyDataConnection()) are owned by the display node
  /// and shared by all 3D views; a view only maps them.
  struct Pipeline
  {
    vtkSmartPointer<vtkGlyph3DMapper> Glypher;
    /// Draws the geometry of the grid, contour and streamline modes; the glyph mapper is
    /// only used in glyph mode.
    vtkSmartPointer<vtkPolyDataMapper> PolyDataMapper;
    vtkSmartPointer<vtkActor> Actor;
    /// The undeformed grid, which has a color of its own
    vtkSmartPointer<vtkPolyDataMapper> NonWarpedMapper;
    vtkSmartPointer<vtkActor> NonWarpedActor;
  };

  typedef std::map<vtkMRMLDisplayNode*, const Pipeline*> PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map<vtkMRMLDisplayableNode*, std::set<vtkMRMLDisplayNode*>> DisplayableToDisplayCacheType;
  DisplayableToDisplayCacheType DisplayableToDisplayNodes;

  // Transforms
  void UpdateDisplayableTransforms(vtkMRMLDisplayableNode* node);
  /// Linear field-to-world transform applied to the actor. Identity if there is no parent transform, it is
  /// not linear (the display node moves the points then), or the node is a transform (evaluated in world).
  void GetLinearFieldToWorldMatrix(vtkMRMLDisplayableNode* node, vtkMatrix4x4* linearTransformToWorld);

  // Display Nodes
  void AddDisplayNode(vtkMRMLDisplayableNode*, vtkMRMLDisplayNode*);
  void UpdateDisplayNode(vtkMRMLDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkMRMLDisplayNode*, const Pipeline*);
  void UpdateScalarColoring(vtkMRMLVectorFieldDisplayNode* displayNode, vtkMapper* mapper);
  void UpdateActorProperties(vtkMRMLVectorFieldDisplayNode* displayNode, vtkMRMLDisplayNode* effectiveDisplayNode, const Pipeline* pipeline, double hierarchyOpacity);
  void RemoveDisplayNode(vtkMRMLDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLDisplayableNode* node);
  void RemoveObservations(vtkMRMLDisplayableNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLDisplayableNode* displayNode);
  bool HasDisplayPipeline(vtkMRMLDisplayableNode* displayNode);
  void ClearDisplayableNodes();

  vtkInternal(vtkMRMLVectorFieldDisplayableManager* external);
  ~vtkInternal();

private:
  vtkMRMLVectorFieldDisplayableManager* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods
vtkMRMLVectorFieldDisplayableManager::vtkInternal::vtkInternal(vtkMRMLVectorFieldDisplayableManager* external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayableManager::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldDisplayableManager::vtkInternal::UseDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  return displayNode && displayNode->IsA("vtkMRMLVectorFieldDisplayNode");
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldDisplayableManager::vtkInternal::UseDisplayableNode(vtkMRMLDisplayableNode* node)
{
  // Any displayable node may store a vector field and get a vector field display node later, so observe all;
  // nodes without one have no pipeline (see HasDisplayPipeline()).
  return node != nullptr;
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldDisplayableManager::vtkInternal::HasDisplayPipeline(vtkMRMLDisplayableNode* node)
{
  auto displayableIt = this->DisplayableToDisplayNodes.find(node);
  return displayableIt != this->DisplayableToDisplayNodes.end() && !displayableIt->second.empty();
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldDisplayableManager::vtkInternal::IsVisible(vtkMRMLDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return false;
  }
  vtkMRMLViewNode* viewNode = this->External->GetMRMLViewNode();
  bool visibleInView = (viewNode == nullptr) || displayNode->IsDisplayableInView(viewNode->GetID());
  return displayNode->GetVisibility() && displayNode->GetVisibility3D() && visibleInView;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::GetLinearFieldToWorldMatrix(vtkMRMLDisplayableNode* node, vtkMatrix4x4* linearTransformToWorld)
{
  linearTransformToWorld->Identity();
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(node);
  // A transform is evaluated in the world, with its parent transforms, so its parent
  // transform is not applied a second time
  if (!transformableNode || vtkMRMLTransformNode::SafeDownCast(node))
  {
    return;
  }
  vtkMRMLTransformNode* transformNode = transformableNode->GetParentTransformNode();
  if (transformNode && transformNode->IsTransformToWorldLinear())
  {
    transformNode->GetMatrixTransformToWorld(linearTransformToWorld);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::UpdateDisplayableTransforms(vtkMRMLDisplayableNode* node)
{
  std::set<vtkMRMLDisplayNode*> displayNodes = this->DisplayableToDisplayNodes[node];
  for (auto displayNodeIt = displayNodes.begin(); displayNodeIt != displayNodes.end(); ++displayNodeIt)
  {
    PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(*displayNodeIt);
    if (pipelineIt != this->DisplayPipelines.end())
    {
      this->UpdateDisplayNodePipeline(pipelineIt->first, pipelineIt->second);
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::RemoveDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(displayNode);
  if (pipelineIt == this->DisplayPipelines.end())
  {
    return;
  }
  const Pipeline* pipeline = pipelineIt->second;
  this->External->GetRenderer()->RemoveViewProp(pipeline->Actor);
  this->External->GetRenderer()->RemoveViewProp(pipeline->NonWarpedActor);
  delete pipeline;
  this->DisplayPipelines.erase(pipelineIt);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::AddDisplayNode(vtkMRMLDisplayableNode* displayableNode, vtkMRMLDisplayNode* displayNode)
{
  if (!displayableNode || !displayNode)
  {
    return;
  }

  // Do not add the display node if it is already associated with a pipeline object
  if (this->DisplayPipelines.find(displayNode) != this->DisplayPipelines.end())
  {
    return;
  }

  Pipeline* pipeline = new Pipeline();
  pipeline->Glypher = vtkSmartPointer<vtkGlyph3DMapper>::New();
  pipeline->PolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  pipeline->Actor = vtkSmartPointer<vtkActor>::New();
  pipeline->Actor->SetMapper(pipeline->Glypher);
  pipeline->Actor->SetVisibility(0);
  // Not pickable: markups would snap onto arrowheads, and every pick would have to test all glyphs.
  pipeline->Actor->SetPickable(0);

  pipeline->NonWarpedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  pipeline->NonWarpedMapper->ScalarVisibilityOff();
  pipeline->NonWarpedActor = vtkSmartPointer<vtkActor>::New();
  pipeline->NonWarpedActor->SetMapper(pipeline->NonWarpedMapper);
  pipeline->NonWarpedActor->SetVisibility(0);
  pipeline->NonWarpedActor->SetPickable(0);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddViewProp(pipeline->Actor);
  this->External->GetRenderer()->AddViewProp(pipeline->NonWarpedActor);
  this->DisplayPipelines.insert(std::make_pair(displayNode, pipeline));

  this->UpdateDisplayNodePipeline(displayNode, pipeline);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::UpdateDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return;
  }
  PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(displayNode);
  if (pipelineIt != this->DisplayPipelines.end())
  {
    this->UpdateDisplayNodePipeline(displayNode, pipelineIt->second);
  }
  else
  {
    this->External->AddDisplayableNode(displayNode->GetDisplayableNode());
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::UpdateDisplayNodePipeline(vtkMRMLDisplayNode* displayNode, const Pipeline* pipeline)
{
  if (!pipeline)
  {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLVectorFieldDisplayableManager::UpdateDisplayNodePipeline failed: pipeline is invalid");
    return;
  }

  pipeline->NonWarpedActor->SetVisibility(false);

  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(displayNode);
  if (!fieldDisplayNode)
  {
    pipeline->Actor->SetVisibility(false);
    return;
  }

  // Get display node from hierarchy that applies display properties on branch
  vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  vtkMRMLDisplayNode* overrideHierarchyDisplayNode = vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);

  // Use hierarchy display node if any, and if overriding is allowed for the current display node.
  bool hierarchyVisibility = true;
  double hierarchyOpacity = 1.0;
  vtkMRMLDisplayNode* effectiveDisplayNode = fieldDisplayNode;
  if (fieldDisplayNode->GetFolderDisplayOverrideAllowed())
  {
    if (overrideHierarchyDisplayNode)
    {
      effectiveDisplayNode = overrideHierarchyDisplayNode;
    }
    hierarchyVisibility = vtkMRMLFolderDisplayNode::GetHierarchyVisibility(displayableNode);
    hierarchyOpacity = vtkMRMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
  }

  if (!hierarchyVisibility || !this->IsVisible(fieldDisplayNode))
  {
    pipeline->Actor->SetVisibility(false);
    return;
  }

  // Parent transform: a linear transform is applied to the actor, which is much cheaper than
  // moving every point. The display node moves the points under a non-linear transform.
  vtkNew<vtkMatrix4x4> linearTransformToWorld;
  this->GetLinearFieldToWorldMatrix(displayableNode, linearTransformToWorld);
  pipeline->Actor->SetUserMatrix(linearTransformToWorld);

  vtkMapper* mapper = nullptr;
  if (fieldDisplayNode->IsPolyDataVisualizationMode())
  {
    // The modes that draw poly data (grid, contour, streamline): the display node builds their
    // geometry and the actor uses a plain mapper
    vtkAlgorithmOutput* polyDataConnection = fieldDisplayNode->GetPipeline()->GetOutputPolyDataConnection();
    if (!polyDataConnection)
    {
      pipeline->Actor->SetVisibility(false);
      return;
    }
    pipeline->Glypher->SetInputConnection(nullptr);
    pipeline->PolyDataMapper->SetInputConnection(polyDataConnection);
    mapper = pipeline->PolyDataMapper;
  }
  else
  {
    // Glyph mode: the display node selects the points, the view places the glyphs at them
    vtkAlgorithmOutput* glyphPointsConnection = fieldDisplayNode->GetGlyphPointsConnection();
    if (!glyphPointsConnection)
    {
      pipeline->Actor->SetVisibility(false);
      return;
    }
    pipeline->PolyDataMapper->SetInputConnection(nullptr);
    pipeline->Glypher->SetInputConnection(glyphPointsConnection);
    // Glyph shape, orientation and scale, as the glyph filter describes them: the geometry that
    // the display node exports is built with the same settings
    fieldDisplayNode->GetGlyphFilter()->ConfigureMapper(pipeline->Glypher);
    mapper = pipeline->Glypher;
  }
  pipeline->Actor->SetMapper(mapper);
  this->UpdateScalarColoring(fieldDisplayNode, mapper);
  this->UpdateActorProperties(fieldDisplayNode, effectiveDisplayNode, pipeline, hierarchyOpacity);

  // Looks like the deformed grid, except for its color
  vtkAlgorithmOutput* nonWarpedConnection = fieldDisplayNode->GetPipeline()->GetNonWarpedGridConnection();
  pipeline->NonWarpedMapper->SetInputConnection(nonWarpedConnection);
  if (nonWarpedConnection)
  {
    pipeline->NonWarpedActor->SetUserMatrix(linearTransformToWorld);
    pipeline->NonWarpedActor->GetProperty()->DeepCopy(pipeline->Actor->GetProperty());
    pipeline->NonWarpedActor->GetProperty()->SetColor(fieldDisplayNode->GetGridNonWarpedColor());
    pipeline->NonWarpedActor->SetVisibility(true);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::UpdateScalarColoring(vtkMRMLVectorFieldDisplayNode* displayNode, vtkMapper* mapper)
{
  // Coloring: either by the active point data scalar array through the color node's
  // lookup table, or by the display node's solid color.
  if (!displayNode->GetScalarVisibility() || !HasRenderedActiveScalar(displayNode))
  {
    mapper->ScalarVisibilityOff();
    return;
  }
  // Multi-component arrays are colored by the chosen component or by their magnitude
  int component = displayNode->GetRenderedActiveScalarComponent();
  bool multiComponent = displayNode->IsRenderedActiveScalarMultiComponent();

  mapper->ScalarVisibilityOn();
  mapper->SetScalarModeToUsePointFieldData();
  mapper->SelectColorArray(displayNode->GetRenderedActiveScalarName());
  mapper->SetColorModeToMapScalars();
  // Component -1 lets the lookup table turn the vector into a scalar (magnitude);
  // single-component arrays are mapped directly.
  mapper->SetArrayComponent(multiComponent ? component : 0);
  vtkMRMLColorNode* colorNode = displayNode->GetColorNode();
  if (multiComponent)
  {
    // The vector-to-scalar mode is a property of the lookup table, so a table of this view's
    // own is used
    vtkSmartPointer<vtkLookupTable> lookupTable = CreateLookupTable(colorNode);
    vtkMRMLModelDisplayableManager::SetLookupTableComponent(lookupTable, component);
    mapper->SetLookupTable(lookupTable);
  }
  else if (colorNode && colorNode->GetLookupTable())
  {
    mapper->SetLookupTable(colorNode->GetLookupTable());
  }
  mapper->UseLookupTableScalarRangeOff();
  mapper->SetScalarRange(displayNode->GetScalarRange());
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::UpdateActorProperties(vtkMRMLVectorFieldDisplayNode* displayNode,
                                                                              vtkMRMLDisplayNode* effectiveDisplayNode,
                                                                              const Pipeline* pipeline,
                                                                              double hierarchyOpacity)
{
  vtkProperty* actorProperties = pipeline->Actor->GetProperty();
  actorProperties->SetColor(effectiveDisplayNode->GetColor());
  double opacity = hierarchyOpacity * effectiveDisplayNode->GetOpacity();
  if (displayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeContour)
  {
    // Nested isosurfaces are only readable if they can be seen through
    opacity *= displayNode->GetContourOpacity3D();
  }
  actorProperties->SetOpacity(opacity);
  actorProperties->SetAmbient(displayNode->GetAmbient());
  actorProperties->SetDiffuse(displayNode->GetDiffuse());
  actorProperties->SetSpecular(displayNode->GetSpecular());
  actorProperties->SetSpecularPower(displayNode->GetPower());
  actorProperties->SetRepresentation(displayNode->GetRepresentation());
  actorProperties->SetBackfaceCulling(displayNode->GetBackfaceCulling());
  actorProperties->SetFrontfaceCulling(displayNode->GetFrontfaceCulling());

  pipeline->Actor->SetVisibility(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::AddObservations(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  for (unsigned long event : ObservedEvents)
  {
    if (!broker->GetObservationExist(node, event, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
      broker->AddObservation(node, event, this->External, this->External->GetMRMLNodesCallbackCommand());
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::RemoveObservations(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  for (unsigned long event : ObservedEvents)
  {
    vtkEventBroker::ObservationVector observations = broker->GetObservations(node, event, this->External, this->External->GetMRMLNodesCallbackCommand());
    broker->RemoveObservations(observations);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::vtkInternal::ClearDisplayableNodes()
{
  while (this->DisplayableToDisplayNodes.size() > 0)
  {
    this->External->RemoveDisplayableNode(this->DisplayableToDisplayNodes.begin()->first);
  }
}

//---------------------------------------------------------------------------
// vtkMRMLVectorFieldDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayableManager::vtkMRMLVectorFieldDisplayableManager()
{
  this->Internal = new vtkInternal(this);
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayableManager::~vtkMRMLVectorFieldDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::AddDisplayableNode(vtkMRMLDisplayableNode* node)
{
  if (this->AddingDisplayableNode)
  {
    return;
  }
  if (!this->Internal->UseDisplayableNode(node))
  {
    return;
  }

  this->AddingDisplayableNode = 1;
  this->Internal->AddObservations(node);
  // Registers the node even if it has no vector field display node yet, so that one can be
  // added to it later.
  this->Internal->DisplayableToDisplayNodes[node];

  int numberOfDisplayNodes = node->GetNumberOfDisplayNodes();
  for (int i = 0; i < numberOfDisplayNodes; i++)
  {
    vtkMRMLDisplayNode* displayNode = node->GetNthDisplayNode(i);
    if (this->Internal->UseDisplayNode(displayNode))
    {
      this->Internal->DisplayableToDisplayNodes[node].insert(displayNode);
      this->Internal->AddDisplayNode(node, displayNode);
    }
  }
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::RemoveDisplayableNode(vtkMRMLDisplayableNode* node)
{
  if (!node)
  {
    return;
  }
  vtkInternal::DisplayableToDisplayCacheType::iterator displayableIt = this->Internal->DisplayableToDisplayNodes.find(node);
  if (displayableIt == this->Internal->DisplayableToDisplayNodes.end())
  {
    return;
  }

  std::set<vtkMRMLDisplayNode*> displayNodes = displayableIt->second;
  for (auto displayNodeIt = displayNodes.begin(); displayNodeIt != displayNodes.end(); ++displayNodeIt)
  {
    this->Internal->RemoveDisplayNode(*displayNodeIt);
  }
  this->Internal->RemoveObservations(node);
  this->Internal->DisplayableToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!displayableNode)
  {
    return;
  }

  // Escape if the scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
  {
    this->SetUpdateFromMRMLRequested(true);
    return;
  }

  this->AddDisplayableNode(displayableNode);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  bool modified = false;
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (displayableNode)
  {
    this->RemoveDisplayableNode(displayableNode);
    modified = true;
  }
  else
  {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(node);
    if (displayNode)
    {
      this->Internal->RemoveDisplayNode(displayNode);
      modified = true;
    }
  }
  if (modified)
  {
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene == nullptr || scene->IsBatchProcessing())
  {
    return;
  }

  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(caller);
  if (!displayableNode)
  {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    return;
  }

  if (event == vtkMRMLDisplayableNode::DisplayModifiedEvent)
  {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(reinterpret_cast<vtkMRMLNode*>(callData));
    if (displayNode && this->Internal->UseDisplayNode(displayNode))
    {
      // A vector field display node was added to the node or one of its properties changed
      this->Internal->DisplayableToDisplayNodes[displayableNode].insert(displayNode);
      this->Internal->UpdateDisplayNode(displayNode);
      this->RequestRender();
    }
    else if (this->Internal->HasDisplayPipeline(displayableNode))
    {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
    }
  }
  else if (event == vtkMRMLDisplayableNode::TransformModifiedEvent //
           || event == vtkMRMLModelNode::PolyDataModifiedEvent     //
           || event == vtkMRMLVolumeNode::ImageDataModifiedEvent)
  {
    if (!this->Internal->HasDisplayPipeline(displayableNode))
    {
      // Nothing is drawn for this node, there is nothing to update or to re-render
      return;
    }
    // The pipeline decides for itself whether it has to re-execute; only the parent
    // transform has to be pushed into it, and a new render has to be requested.
    this->Internal->UpdateDisplayableTransforms(displayableNode);
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkDebugMacro("vtkMRMLVectorFieldDisplayableManager->UpdateFromMRML: Scene is not set.");
    return;
  }

  // Find the display nodes that need to be removed
  // (not deleted immediately because it would invalidate the pipeline iteration)
  std::set<vtkMRMLDisplayableNode*> displayableNodesToRemove;
  std::deque<vtkMRMLDisplayNode*> displayNodesToRemove;
  for (auto displayNodeToPipeline : this->Internal->DisplayPipelines)
  {
    vtkMRMLDisplayNode* displayNode = displayNodeToPipeline.first;
    if (!displayNode)
    {
      continue;
    }
    if (!scene->IsNodePresent(displayNode))
    {
      displayNodesToRemove.push_back(displayNode);
      continue;
    }
    vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
    if (displayableNodesToRemove.find(displayableNode) != displayableNodesToRemove.end())
    {
      continue;
    }
    if (!this->Internal->UseDisplayableNode(displayableNode))
    {
      displayableNodesToRemove.insert(displayableNode);
      continue;
    }
    if (!this->Internal->UseDisplayNode(displayNode))
    {
      displayNodesToRemove.push_back(displayNode);
      continue;
    }
  }
  for (vtkMRMLDisplayableNode* displayableNodeToRemove : displayableNodesToRemove)
  {
    this->RemoveDisplayableNode(displayableNodeToRemove);
  }
  for (vtkMRMLDisplayNode* displayNodeToRemove : displayNodesToRemove)
  {
    this->Internal->RemoveDisplayNode(displayNodeToRemove);
  }

  std::vector<vtkMRMLNode*> displayableNodes;
  int numberOfDisplayableNodes = scene->GetNodesByClass("vtkMRMLDisplayableNode", displayableNodes);
  for (int i = 0; i < numberOfDisplayableNodes; i++)
  {
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(displayableNodes[i]);
    if (this->Internal->UseDisplayableNode(displayableNode))
    {
      this->AddDisplayableNode(displayableNode);
    }
  }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldDisplayableManager::Create()
{
  this->SetUpdateFromMRMLRequested(true);
}
