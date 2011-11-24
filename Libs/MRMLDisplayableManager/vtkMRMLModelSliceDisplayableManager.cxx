/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLModelSliceDisplayableManager.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWeakPointer.h>


// VTK includes: customization
#include <vtkCutter.h>

// STD includes
#include <algorithm>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLModelSliceDisplayableManager );
vtkCxxRevisionMacro(vtkMRMLModelSliceDisplayableManager, "$Revision: 13525 $");

//---------------------------------------------------------------------------
class vtkMRMLModelSliceDisplayableManager::vtkInternal
{
public:
  struct Pipeline
    {
    vtkProp* actor;
    vtkCutter* cutter;
    vtkTransform* nodeToWorld;
    vtkTransformPolyDataFilter* transformer;
    vtkPlane* plane;
    };

  typedef std::map < vtkMRMLDisplayNode*, const Pipeline* > PipelinesCacheType;
  
  void UpdateDisplayNodePipelineTransforms(vtkMRMLDisplayNode* displayNode, const Pipeline* pipeline);
  vtkMatrix4x4* GetDisplayNodeMatrixToWorld(vtkMRMLDisplayNode* displayNode);
  
  // Slice Node
  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  void UpdateSliceNode();
  void SetSlicePlaneFromMatrix(vtkMatrix4x4* matrix, vtkPlane* plane);
  
  // Display Node
  void UpdateDisplayNodePipeline(vtkMRMLDisplayNode* displayNode);
  void RemoveDisplayNodePipeline(vtkMRMLDisplayNode* displayNode);
  bool IsVisible(vtkMRMLDisplayNode* displayNode);
  bool IsDisplayable(vtkMRMLDisplayNode* displayNode);
  vtkPolyData* GetDisplayNodePolyData(vtkMRMLDisplayNode* displayNode);

  // Displayable Node caching (for transform events)
  void AddUpdateDisplayableNodeRef(vtkMRMLDisplayNode* displayNode);
  void RemoveUpdateDisplayableNodeRef(vtkMRMLDisplayNode* displayNode);
  void UpdateDisplayableNodeRefs();
  void UpdateDisplayableNodePipeline(vtkMRMLDisplayableNode* node);
  
  // Actors
  void AddActor(vtkMRMLDisplayNode* displayNode);
  void UpdateActor(vtkMRMLDisplayNode* displayNode, const Pipeline* pipeline);
  void RemoveActor(vtkMRMLDisplayNode* displayNode);
  
  vtkInternal( vtkMRMLModelSliceDisplayableManager* external );
  ~vtkInternal();
  
private:
  PipelinesCacheType DisplayPipelines;
    
  std::map<vtkMRMLDisplayableNode*, vtkMRMLDisplayNode*> DisplayableNodes;
  std::vector<vtkMRMLDisplayNode*> DisplayNodesToUpdateDisplayable;
  
  vtkMRMLSliceNode* SliceNode;
  
  vtkMRMLModelSliceDisplayableManager* External;
  
};

//---------------------------------------------------------------------------
// vtkInternal methods
vtkMRMLModelSliceDisplayableManager::vtkInternal
::vtkInternal(vtkMRMLModelSliceDisplayableManager* external)
{
  this->External = external;
  this->SliceNode = 0;
}

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::vtkInternal
::~vtkInternal()
{
  PipelinesCacheType::iterator pipelinesIt;
  for (pipelinesIt = this->DisplayPipelines.begin();
       pipelinesIt != this->DisplayPipelines.end();
       pipelinesIt = this->DisplayPipelines.begin())
    {
    pipelinesIt->first->RemoveObserver(this->External->GetMRMLNodesCallbackCommand());
    this->RemoveDisplayNodePipeline(pipelinesIt->first);
    }
  assert(this->DisplayPipelines.size() == 0);
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal
::IsDisplayable(vtkMRMLDisplayNode* displayNode)
{
  return displayNode && displayNode->IsA("vtkMRMLModelDisplayNode");
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal
::IsVisible(vtkMRMLDisplayNode* displayNode)
{
  // TBD: hide when !visible or !scalarsvisible?
  return (displayNode->GetSliceIntersectionVisibility() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::AddUpdateDisplayableNodeRef(vtkMRMLDisplayNode* displayNode)
{
  this->DisplayNodesToUpdateDisplayable.push_back(displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::RemoveUpdateDisplayableNodeRef(vtkMRMLDisplayNode* displayNode)
{
  std::vector<vtkMRMLDisplayNode*>::iterator it =
    std::find( this->DisplayNodesToUpdateDisplayable.begin(),
               this->DisplayNodesToUpdateDisplayable.end(),
               displayNode);
  if (it == this->DisplayNodesToUpdateDisplayable .end())
    {
    return;
    }
  this->DisplayNodesToUpdateDisplayable.erase(it);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayableNodeRefs()
{
  // The DisplayableNode reference may not be available when NodeAdded fires
  //   so AddActor pushes to the UpdateNodeRef queue to try again.
  //   This gives up after one call - maybe incorrect, but we don't want to
  //   call GetDisplayableNode() every single update. 
  //   At worst, transform updates are not observed.
  
  std::vector<vtkMRMLDisplayNode*>::iterator it;
  for (it = DisplayNodesToUpdateDisplayable.begin();
       it != DisplayNodesToUpdateDisplayable.end(); it++)
    {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(*it);
    vtkMRMLDisplayableNode* node = displayNode->GetDisplayableNode();
    if (node)
      {
      node->AddObserver(vtkMRMLDisplayableNode::TransformModifiedEvent, 
                        this->External->GetMRMLNodesCallbackCommand() );
      this->DisplayableNodes[node] = displayNode;
      }
    }
  // This is safer cross-platform than erasing in the loop
  this->DisplayNodesToUpdateDisplayable.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayableNodePipeline(vtkMRMLDisplayableNode* displayableNode)
{
  vtkMRMLDisplayNode* displayNode = this->DisplayableNodes[displayableNode];
  if (displayNode)
    {
    this->UpdateDisplayNodePipeline(displayNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
    return;
  this->SliceNode = sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateSliceNode()
{
  PipelinesCacheType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end(); ++it)
    {
    this->UpdateDisplayNodePipelineTransforms(it->first, it->second);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::SetSlicePlaneFromMatrix(vtkMatrix4x4 *sliceMatrix, vtkPlane* plane)
{
  double normal[3];
  double origin[3];
  
  // +/-1: relative orientation of the normal
  const int planeOrientation = 1;
  for (int i = 0; i < 3; i++)
    {
    normal[i] = planeOrientation * sliceMatrix->GetElement(i,2);
    origin[i] = sliceMatrix->GetElement(i,3);
    }
    
  plane->SetNormal(normal);
  plane->SetOrigin(origin);
}

//---------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLModelSliceDisplayableManager::vtkInternal
::GetDisplayNodeMatrixToWorld(vtkMRMLDisplayNode* displayNode)
{
  vtkMRMLDisplayableNode* node = displayNode->GetDisplayableNode();
  vtkSmartPointer<vtkMatrix4x4> nodeMatrixToWorld = vtkSmartPointer<vtkMatrix4x4>::New(); 
  //TBD: do anything special if Displayable is not available?
  if (!node)
    return nodeMatrixToWorld;

  vtkMRMLTransformNode* tnode = 
    node->GetParentTransformNode();
  if (tnode != 0 && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(nodeMatrixToWorld);
    }
  return nodeMatrixToWorld;
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayNodePipelineTransforms(vtkMRMLDisplayNode* displayNode, const Pipeline* pipeline)
{
  /*  TODO: I think it should be possible to split the transforms in two:
   *    modelPolyData -> TransformPDFilter{modelToWorldMatrix} -> 
   *    cutter{slicePlane in world space}
   *  this might avoid updating the TransformPDFilter part of the pipeline
   *  when the modelToWorldMatrix has not changed (which is most of the time)
   */
  vtkSmartPointer<vtkMatrix4x4> modelMatrixToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  modelMatrixToWorld->DeepCopy(this->GetDisplayNodeMatrixToWorld(displayNode));
  
  vtkSmartPointer<vtkMatrix4x4> rasToXY = vtkSmartPointer<vtkMatrix4x4>::New();
  rasToXY->DeepCopy(this->SliceNode->GetXYToRAS());
  rasToXY->Invert();
  
  vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkMatrix4x4::Multiply4x4(rasToXY, modelMatrixToWorld, mat);
  rasToXY->DeepCopy(mat);
  
  pipeline->nodeToWorld->SetMatrix(rasToXY);

  modelMatrixToWorld->Invert();
  
  rasToXY->DeepCopy(this->SliceNode->GetXYToRAS());
  vtkMatrix4x4::Multiply4x4(modelMatrixToWorld, rasToXY, mat);
  rasToXY->DeepCopy(mat);
  
  this->SetSlicePlaneFromMatrix(rasToXY, pipeline->plane);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::RemoveDisplayNodePipeline(vtkMRMLDisplayNode* displayNode)
{
  PipelinesCacheType::iterator actorsIt = this->DisplayPipelines.find(displayNode);
  if(actorsIt == this->DisplayPipelines.end())
    {
    return;
    }
  const Pipeline* pipeline = actorsIt->second;
  this->External->GetRenderer()->RemoveActor(pipeline->actor);
  pipeline->actor->Delete();
  pipeline->cutter->Delete();
  pipeline->nodeToWorld->Delete();
  pipeline->transformer->Delete();
  pipeline->plane->Delete();
  delete pipeline;
  this->DisplayPipelines.erase(actorsIt);
  this->External->RequestRender();
}

void vtkMRMLModelSliceDisplayableManager::vtkInternal
::AddActor(vtkMRMLDisplayNode* displayNode)
{  
  // TBD: probably could avoid searching every time
  if ( !this->IsDisplayable(displayNode) && 
        this->DisplayPipelines.find(displayNode) != this->DisplayPipelines.end() )
    {
    return;
    }
    
  vtkActor2D* actor = vtkActor2D::New();
  if (displayNode->IsA("vtkMRMLModelDisplayNode"))
    {
    vtkSmartPointer<vtkPolyDataMapper2D> mapper =
      vtkSmartPointer<vtkPolyDataMapper2D>::New();
    actor->SetMapper( mapper );
    }

  Pipeline* pNew = new Pipeline();
  pNew->actor = actor;
  pNew->cutter = vtkCutter::New();
  pNew->nodeToWorld = vtkTransform::New();
  pNew->transformer = vtkTransformPolyDataFilter::New();
  pNew->plane = vtkPlane::New();
  
  pNew->cutter->SetCutFunction(pNew->plane);
  pNew->cutter->SetGenerateCutScalars(0);
  pNew->transformer->SetTransform(pNew->nodeToWorld);

  this->External->GetRenderer()->AddActor( actor );
  this->DisplayPipelines.insert( std::make_pair(displayNode, pNew) );
  this->UpdateActor(displayNode, pNew);
}

void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateActor(vtkMRMLDisplayNode* displayNode, const Pipeline* pipeline)
{
  bool visible = this->IsVisible(displayNode);
  pipeline->actor->SetVisibility(visible);
  
  if (visible && displayNode->IsA("vtkMRMLModelDisplayNode"))
    {
    vtkMRMLModelDisplayNode* modelDisplayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
    vtkPolyData* polyData = this->GetDisplayNodePolyData(modelDisplayNode);
    if (!polyData)
      {
      return;
      }

    this->UpdateDisplayNodePipelineTransforms(displayNode, pipeline);

    pipeline->cutter->SetInput( polyData );

    pipeline->transformer->SetInputConnection( pipeline->cutter->GetOutputPort() );
    vtkActor2D* actor2D = vtkActor2D::SafeDownCast(pipeline->actor);
    vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(
      actor2D->GetMapper());
      
    actor2D->SetPosition(0,0);  

    vtkProperty2D* actorProperties = actor2D->GetProperty();
    actorProperties->SetColor(displayNode->GetColor() );

    mapper->SetInputConnection( pipeline->transformer->GetOutputPort() );
    mapper->SetLookupTable( modelDisplayNode->GetColorNode() ?
                            modelDisplayNode->GetColorNode()->GetScalarsToColors() : 0);
    mapper->SetScalarRange(modelDisplayNode->GetScalarRange());
    }

  // TBD: Not sure a render request has to systematically be called
  pipeline->cutter->Modified();
  this->External->RequestRender();
}

void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayNodePipeline(vtkMRMLDisplayNode* displayNode)
{
  PipelinesCacheType::iterator it;
  if (!displayNode 
      || !this->IsDisplayable(displayNode)
      || (it = this->DisplayPipelines.find(displayNode)) == this->DisplayPipelines.end()
      )
    {
    return;
    }
  if (this->DisplayNodesToUpdateDisplayable.size() > 0)
    {
    this->UpdateDisplayableNodeRefs();
    }
  this->UpdateActor(displayNode, it->second);
}

vtkPolyData* vtkMRMLModelSliceDisplayableManager::vtkInternal
::GetDisplayNodePolyData(vtkMRMLDisplayNode* displayNode)
{
  // Allow customization of PD handling if needed.
  vtkPolyData* polyData = displayNode->GetPolyData();
  return polyData;
}

//---------------------------------------------------------------------------
// vtkMRMLModelSliceDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::vtkMRMLModelSliceDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::~vtkMRMLModelSliceDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::AddDisplayNode(
  vtkMRMLDisplayNode* displayNode)
{
  this->Internal->AddActor(displayNode);
  displayNode->AddObserver(vtkCommand::ModifiedEvent, this->GetMRMLNodesCallbackCommand() );
  
  // Observe the displayable node for TransformModified events
  vtkMRMLDisplayableNode* modelNode = displayNode->GetDisplayableNode();
  if (modelNode)
    {
    modelNode->AddObserver( vtkCommand::ModifiedEvent, this->GetMRMLNodesCallbackCommand() );
    }
  else
    {  
    this->Internal->AddUpdateDisplayableNodeRef(displayNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::RemoveDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  displayNode->RemoveObserver(this->GetMRMLNodesCallbackCommand());
  this->Internal->RemoveDisplayNodePipeline(displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  if ( !node->IsA("vtkMRMLModelDisplayNode") 
      || node->IsA("vtkMRMLFiberBundleLineDisplayNode")
      || node->IsA("vtkMRMLFiberBundleGlyphDisplayNode") )
    {
    return;
    }
  this->AddDisplayNode(vtkMRMLModelDisplayNode::SafeDownCast(node));
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node)
{
  vtkMRMLModelDisplayNode* displayNode =
    vtkMRMLModelDisplayNode::SafeDownCast(node);
  if ( !displayNode )
    {
    return;
    }
  this->Internal->RemoveUpdateDisplayableNodeRef(displayNode);
  this->RemoveDisplayNode(displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLDisplayNode* displayNode;
  vtkMRMLDisplayableNode* displayableNode;
  if (event == vtkCommand::ModifiedEvent)
    {
    if ( vtkMRMLSliceNode::SafeDownCast(caller) )
      {
      this->Internal->UpdateSliceNode();
      }
    else if ( (displayNode = vtkMRMLDisplayNode::SafeDownCast(caller)) )
      {
      this->Internal->UpdateDisplayNodePipeline(displayNode);
      }
    }
  else if (vtkMRMLDisplayableNode::TransformModifiedEvent &&
           (displayableNode = vtkMRMLDisplayableNode::SafeDownCast(caller)) )
    {
    this->Internal->UpdateDisplayableNodePipeline(displayableNode);
    }
      
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }    
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::Create()
{
  this->Internal->SetSliceNode(this->GetMRMLSliceNode());
}
