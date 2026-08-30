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

#include "vtkMRMLLayerDMPipelineFactory.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineCreator.h"
#include "vtkMRMLLayerDMPipelineCallbackCreator.h"
#include "vtkMRMLLayerDMPipeline.h"
#include "vtkMRMLLayerDMObjectEventObserver.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMPipelineFactory);

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMRMLLayerDMPipelineFactory> vtkMRMLLayerDMPipelineFactory::GetInstance()
{
  static vtkSmartPointer<vtkMRMLLayerDMPipelineFactory> instance = vtkSmartPointer<vtkMRMLLayerDMPipelineFactory>::New();
  return instance;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineFactory::AddPipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreator>& creator)
{
  if (this->ContainsPipelineCreator(creator))
  {
    return;
  }

  this->Observer->UpdateObserver(nullptr, creator);
  this->PipelineCreators.emplace_back(creator);
  this->SortPipelineCreators();
  this->InvokeEvent(vtkCommand::ModifiedEvent);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMRMLLayerDMPipelineCreator> vtkMRMLLayerDMPipelineFactory::AddPipelineCreator(
  const std::function<vtkSmartPointer<vtkMRMLLayerDMPipeline>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& creatorCallBack,
  int priority)
{
  auto creator = vtkSmartPointer<vtkMRMLLayerDMPipelineCallbackCreator>::New();
  creator->SetCallback(creatorCallBack);
  creator->SetPriority(priority);
  this->AddPipelineCreator(creator);
  return creator;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineFactory::RemovePipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreator>& creator)
{
  this->Observer->RemoveObserver(creator);
  size_t prevSize = this->PipelineCreators.size();
  this->PipelineCreators.erase(std::remove_if(this->PipelineCreators.begin(),
                                              this->PipelineCreators.end(),
                                              [creator](const vtkSmartPointer<vtkMRMLLayerDMPipelineCreator>& value) { return value == creator; }),
                               this->PipelineCreators.end());
  if (this->PipelineCreators.size() != prevSize)
  {
    this->InvokeEvent(vtkCommand::ModifiedEvent);
  }
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMPipelineFactory::ContainsPipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreator>& creator) const
{
  return std::find(this->PipelineCreators.begin(), this->PipelineCreators.end(), creator) != this->PipelineCreators.end();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMRMLLayerDMPipeline> vtkMRMLLayerDMPipelineFactory::CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node)
{
  for (const auto& ctor : this->PipelineCreators)
  {
    if (auto created = ctor->CreatePipeline(viewNode, node))
    {
      this->LastView = viewNode;
      this->LastNode = node;
      this->LastPipeline = created;
      this->LastCreator = ctor;
      this->InvokeEvent(PipelineAboutToBeCreatedEvent);
      return created;
    }
  }

  return {};
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractViewNode* vtkMRMLLayerDMPipelineFactory::GetLastViewNode() const
{
  return this->LastView;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLLayerDMPipelineFactory::GetLastNode() const
{
  {
    return this->LastNode;
  }
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipeline* vtkMRMLLayerDMPipelineFactory::GetLastPipeline() const
{
  {
    return this->LastPipeline;
  }
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipelineCreator* vtkMRMLLayerDMPipelineFactory::GetLastCreator() const
{
  return this->LastCreator;
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipelineFactory::vtkMRMLLayerDMPipelineFactory()
  : Observer(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
{
  this->Observer->SetUpdateCallback([this](vtkObject* node) { this->SortPipelineCreators(); });
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineFactory::SortPipelineCreators()
{
  std::sort(std::begin(this->PipelineCreators),
            std::end(this->PipelineCreators),
            [](const vtkSmartPointer<vtkMRMLLayerDMPipelineCreator>& a, const vtkSmartPointer<vtkMRMLLayerDMPipelineCreator>& b)
            {
              if (!a || !b)
              {
                return true;
              }

              return a->GetPriority() > b->GetPriority();
            });
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Number of pipeline creators: " << this->PipelineCreators.size() << std::endl;
  os << indent << "Last view node: " << (this->LastView ? "set" : "(none)") << std::endl;
  os << indent << "Last node: " << (this->LastNode ? "set" : "(none)") << std::endl;
  os << indent << "Last pipeline: " << (this->LastPipeline ? "set" : "(none)") << std::endl;
  os << indent << "Last creator: " << (this->LastCreator ? "set" : "(none)") << std::endl;
}
