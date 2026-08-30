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

==============================================================================*/

#include "vtkMRMLLayerDMInteractionLogic.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipeline.h"

// Slicer includes
#include "vtkMRMLAbstractWidget.h"
#include "vtkMRMLInteractionEventData.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <algorithm>
#include <limits>
#include <map>
#include <tuple>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMInteractionLogic);

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipeline* vtkMRMLLayerDMInteractionLogic::GetLastFocusedPipeline() const
{
  return this->LastFocusedPipeline;
}

vtkMRMLLayerDMInteractionLogic::vtkMRMLLayerDMInteractionLogic() = default;

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMInteractionLogic::MinWidgetState()
{
  return vtkMRMLAbstractWidget::WidgetStateOnWidget;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMInteractionLogic::LoseFocus(vtkMRMLInteractionEventData* eventData)
{
  if (this->LastFocusedPipeline)
  {
    this->LastFocusedPipeline->LoseFocus(eventData);
    this->LastFocusedPipeline = nullptr;
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMInteractionLogic::LoseFocus()
{
  vtkNew<vtkMRMLInteractionEventData> leaveEvent;
  leaveEvent->SetType(vtkCommand::LeaveEvent);
  leaveEvent->SetViewNode(this->ViewNode);
  this->LoseFocus(leaveEvent);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMInteractionLogic::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  this->ViewNode = viewNode;
}

//-----------------------------------------------------------------------------
std::vector<vtkSmartPointer<vtkMRMLLayerDMPipeline>> vtkMRMLLayerDMInteractionLogic::GetCanProcessPipelines() const
{
  return this->CanProcessPipelines;
}

//-----------------------------------------------------------------------------
std::tuple<double, int> vtkMRMLLayerDMInteractionLogic::PrioritizeCanProcessPipelines(vtkMRMLInteractionEventData* eventData)
{
  // For each pipeline, if pipeline can process, store its state value, layer and distance to interaction
  std::map<vtkMRMLLayerDMPipeline*, std::tuple<int, unsigned int, double>> priority;
  double minDistance = std::numeric_limits<double>::max();
  int maxState = this->MinWidgetState();
  for (const auto& pipeline : this->Pipelines)
  {
    if (pipeline->IsInteractionProcessingBlocked())
    {
      continue;
    }

    double pipelineDistance = std::numeric_limits<double>::max();
    if (pipeline->CanProcessInteractionEvent(eventData, pipelineDistance))
    {
      this->CanProcessPipelines.emplace_back(pipeline);
      int widgetState = std::max(this->MinWidgetState(), pipeline->GetWidgetState());
      minDistance = std::min(minDistance, pipelineDistance);
      maxState = std::max(widgetState, maxState);
      priority[pipeline] = std::make_tuple(widgetState, pipeline->GetMaxRenderOrder(), -pipelineDistance);
    }
  }
  // Sort can process by layer order and inverted square distance (larger layer number first and closest to interaction)
  std::sort(this->CanProcessPipelines.begin(),
            this->CanProcessPipelines.end(),
            [&priority](const vtkSmartPointer<vtkMRMLLayerDMPipeline>& a, const vtkSmartPointer<vtkMRMLLayerDMPipeline>& b) { return priority[a] > priority[b]; });

  return std::make_tuple(minDistance, maxState);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMInteractionLogic::LosePreviousFocusInCannotProcess(vtkMRMLInteractionEventData* eventData)
{
  // Lose focus if previous focused pipeline cannot process current interaction
  if (std::find(this->CanProcessPipelines.begin(), this->CanProcessPipelines.end(), this->LastFocusedPipeline) == this->CanProcessPipelines.end())
  {
    this->LoseFocus(eventData);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMInteractionLogic::AddPipeline(const vtkSmartPointer<vtkMRMLLayerDMPipeline>& pipeline)
{
  if (std::find(this->Pipelines.begin(), this->Pipelines.end(), pipeline) != this->Pipelines.end())
  {
    return;
  }
  this->Pipelines.emplace_back(pipeline);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMInteractionLogic::RemovePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipeline>& pipeline)
{
  if (this->LastFocusedPipeline == pipeline)
  {
    this->LoseFocus();
  }
  const auto pipelineIt = std::find(this->Pipelines.begin(), this->Pipelines.end(), pipeline);
  if (pipelineIt != this->Pipelines.end())
  {
    this->Pipelines.erase(pipelineIt);
  }
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMInteractionLogic::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  // Clear previous interaction list
  this->CanProcessPipelines.clear();

  // On leave event lose focus and early return to avoid bad pipeline state
  if (eventData->GetType() == vtkCommand::LeaveEvent)
  {
    this->LoseFocus(eventData);
    return false;
  }

  // Refresh the can process pipelines and order them by priority
  auto [minDistance, maxState] = this->PrioritizeCanProcessPipelines(eventData);

  // Lose previous focus if not in can process list
  this->LosePreviousFocusInCannotProcess(eventData);

  // Return lowest double value if any pipeline can process and is not idle
  // Otherwise, return the min distance returned by the processes.
  distance2 = maxState > this->MinWidgetState() ? std::numeric_limits<double>::lowest() : minDistance;
  return !this->CanProcessPipelines.empty();
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMInteractionLogic::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  for (const auto& pipeline : this->CanProcessPipelines)
  {
    if (pipeline->IsInteractionProcessingBlocked())
    {
      continue;
    }

    // If pipeline can process, store pipeline for further interaction events
    if (pipeline->ProcessInteractionEvent(eventData))
    {
      if (pipeline != this->LastFocusedPipeline)
      {
        this->LoseFocus(eventData);
      }
      this->LastFocusedPipeline = pipeline;
      return true;
    }
  }

  // If no pipeline was able to process interaction, lose focus
  this->LoseFocus(eventData);
  return false;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMInteractionLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Number of pipelines: " << this->Pipelines.size() << std::endl;
  os << indent << "Number of pipelines able to process the last event: " << this->CanProcessPipelines.size() << std::endl;
  os << indent << "Last focused pipeline: " << (this->LastFocusedPipeline ? "set" : "(none)") << std::endl;
  os << indent << "View node: " << (this->ViewNode ? "set" : "(none)") << std::endl;
}
