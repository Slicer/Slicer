#include "vtkMRMLModelPickingDMPipeline.h"

#include "vtkMRMLModelDMPipeline.h"
#include "vtkMRMLModelPickingNode.h"

#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLLayerDMPipelineManager.h"
#include "vtkMRMLScene.h"

#include <vtkActor.h>
#include <vtkCellPicker.h>
#include <vtkObjectFactory.h>
#include <vtkPropCollection.h>
#include <vtkPropPicker.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLModelPickingDMPipeline);

//----------------------------------------------------------------------------
vtkMRMLModelPickingDMPipeline::vtkMRMLModelPickingDMPipeline()
  : PropPicker{ vtkSmartPointer<vtkPropPicker>::New() }
  , CellPicker{ vtkSmartPointer<vtkCellPicker>::New() }
{
}

//----------------------------------------------------------------------------
vtkMRMLModelPickingDMPipeline::~vtkMRMLModelPickingDMPipeline() = default;

//----------------------------------------------------------------------------
void vtkMRMLModelPickingDMPipeline::SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager)
{
  Superclass::SetPipelineManager(pipelineManager);
  UpdateObserver(nullptr, pipelineManager);
  this->UpdateActorToDisplayNodeMap();
}

//----------------------------------------------------------------------------
void vtkMRMLModelPickingDMPipeline::OnUpdate(vtkObject* obj, unsigned long eventId, void* callData)
{
  if (obj == this->GetPipelineManager())
  {
    this->UpdateActorToDisplayNodeMap();
  }
}

//----------------------------------------------------------------------------
vtkMRMLModelPickingNode* vtkMRMLModelPickingDMPipeline::GetPickingNode() const
{
  return vtkMRMLModelPickingNode::SafeDownCast(this->GetDisplayNode());
}

//----------------------------------------------------------------------------
void vtkMRMLModelPickingDMPipeline::UpdateActorToDisplayNodeMap()
{
  // Reset actor to display pick list
  this->ActorToDisplayNodeMap.clear();
  this->CellPicker->InitializePickList();
  this->CellPicker->PickFromListOn();

  // Update given the current pipelines available in the pipeline manager
  if (!this->GetPipelineManager())
  {
    return;
  }

  for (int i = 0; i < this->GetPipelineManager()->GetNumberOfPipelines(); ++i)
  {
    const vtkMRMLModelDMPipeline* modelPipeline = vtkMRMLModelDMPipeline::SafeDownCast(this->GetPipelineManager()->GetNthPipeline(i));
    if (!modelPipeline || !modelPipeline->GetModelDisplayNode())
    {
      continue;
    }

    for (auto& actor : modelPipeline->GetProps())
    {
      if (actor)
      {
        this->ActorToDisplayNodeMap[actor] = modelPipeline->GetModelDisplayNode();
        this->CellPicker->AddPickList(actor);
      }
    }
  }
}

//----------------------------------------------------------------------------
bool vtkMRMLModelPickingDMPipeline::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  vtkMRMLModelPickingNode* pickingNode = this->GetPickingNode();
  if (!this->GetPipelineManager() || !pickingNode || !pickingNode->GetEnabled() || !this->GetViewNode() || !this->GetRenderer())
  {
    return false;
  }

  // Initialize picking node view ID and reset the picking node content in case of early return
  MRMLNodeModifyBlocker blocker{ pickingNode };
  pickingNode->SetViewID(this->GetViewNode()->GetID());
  pickingNode->SetPickedNodeID(nullptr);
  pickingNode->SetPickedCellID(-1);
  pickingNode->SetPickedPointID(-1);

  // Use hardware picking to check if any actors have been hit
  int displayPos[2];
  eventData->GetDisplayPosition(displayPos);
  if (!this->PropPicker->Pick(displayPos[0], displayPos[1], 0.0, this->GetRenderer()))
  {
    return false;
  }

  // Check if picked actor matches a model node
  vtkActor* pickedActor = vtkActor::SafeDownCast(this->PropPicker->GetViewProp());
  if (!pickedActor || !this->ActorToDisplayNodeMap.count(pickedActor))
  {
    return false;
  }

  // Update the picked model node
  vtkMRMLDisplayNode* displayNode = this->ActorToDisplayNodeMap[pickedActor];
  vtkMRMLDisplayableNode* displayableNode = displayNode ? displayNode->GetDisplayableNode() : nullptr;
  pickingNode->SetPickedNodeID(displayableNode ? displayableNode->GetID() : nullptr);

  // Use Cell picking for finer granularity
  double ras[3];
  this->PropPicker->GetPickPosition(ras);
  pickingNode->SetPickedRAS(ras);
  this->CellPicker->SetTolerance(pickingNode->GetPickTolerance());
  if (this->CellPicker->Pick(displayPos[0], displayPos[1], 0.0, this->GetRenderer()))
  {
    pickingNode->SetPickedCellID(this->CellPicker->GetCellId());
    pickingNode->SetPickedPointID(this->CellPicker->GetPointId());
  }

  // Always return false as we only want to update the picking node
  return false;
}
